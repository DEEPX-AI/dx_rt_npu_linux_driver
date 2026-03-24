// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/pm_runtime.h>
#include <linux/dmaengine.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#include <linux/cpumask.h>

#include "dx_mmio_compat.h"
#include "dw-edma-core.h"
#include "dw-edma-v0-core.h"
#include "dw-edma-v0-regs.h"
#include "virt-dma.h"
#include "dx_sgdma_cdev.h"
#include "dx_lib.h"
#include "dx_util.h"
#include "dw-edma-thread.h"
#include "dx_message.h"
#include "dw-edma-mem.h"

#ifdef DX_DEBUG_ENABLE /*DEEPX MODIFIED*/
	#ifdef dev_vdbg
		#undef dev_vdbg
		#define dev_vdbg		dev_err
	#endif
#endif

/* ---------------- User IRQ vector table (placed early for dx_sw_intr_init) --- */
typedef struct user_irq_v_table_t {
	irq_handler_t handler;
	char          name[40];
	int           irq_pos;   /* Position in host IRQ table */
	int           event_id;  /* Max ID defined by EDMA_EVENT_NUM_MAX */
	int           dma_ch_n;  /* DMA channel number */
	uint32_t      bit;       /* Bit position mask */
} user_irq_v_table_t;

static user_irq_v_table_t *user_irq_vec_table; /* selected at runtime */
/* Cached active events derived from vector table (host-only state) */
static u32 dx_sw_active_mask;       /* bit i set when event i is valid */
static int dx_sw_active_count;      /* number of active events */


static irqreturn_t dw_edma_user_irq_npu(int irq, void *data);
static irqreturn_t dw_edma_user_events(int irq, void *data);
static irqreturn_t user_irq_service(int irq, struct dx_dma_user_irq *user_irq);
static irqreturn_t user_irq_events(struct dx_edma_irq *dw_irq, struct dx_dma_user_irq *user_irq);

static user_irq_v_table_t user_irq_vec_table_v3[USER_IRQ_NUMS] = {
	/* handler / name / irq_pos / event_id / dma_ch_n / bit */
	{dw_edma_user_irq_npu, "npu0_d", 0, 0, 0, BIT(0)},
	{dw_edma_user_irq_npu, "npu1_d", 1, 1, 1, BIT(1)},
	{dw_edma_user_irq_npu, "npu2_d", 2, 2, 2, BIT(2)},
	{dw_edma_user_events , "events", 3, 3, 0, BIT(3)},
};

// #define DX_SW_IRQ_DEBUG

/* ---------------- Software-only interrupt block (no HW register logic) -------- */
/* Endpoint writes set_off (bit mask). Host latches into status_raw_off and clears
 * bits after servicing via simple RMW. enable mask gates service. */
struct dx_sw_irq_block {
	uint32_t version;                          /* optional version id */
	uint32_t enable;                           /* mask bits (Host writes) */
	uint32_t set_off[USER_IRQ_NUMS];           /* per-event set bits (EP writes 1, host clears to 0) */
	uint32_t status_raw_off;                   /* latched pending bitfield (mirror aggregate) */
	uint32_t event_irq_cnt[USER_IRQ_NUMS];     /* EP-side event counters */
	uint32_t event_handled_cnt[USER_IRQ_NUMS]; /* Host-side handled counters */
};

static inline struct dx_sw_irq_block __iomem *dx_sw_irq(struct dw_edma *dw)
{
	return (struct dx_sw_irq_block __iomem *)dw->dx_msg->irq_status; /* shared SRAM */
}

/* Debug helper to dump interrupt block state */
static void dx_sw_irq_dump_state(struct dw_edma *dw, const char *context)
{
	struct dx_sw_irq_block __iomem *blk = dx_sw_irq(dw);
	int i;
	ktime_t start, end;
	u32 enable, raw;
	s64 read_time_ns;

	if (!blk)
		return;

	start = ktime_get();
	enable = ioread32(&blk->enable);
	raw = ioread32(&blk->status_raw_off);
	end = ktime_get();
	read_time_ns = ktime_to_ns(ktime_sub(end, start));

	dbg_irq("DUMP[%s]: enable=0x%08x raw=0x%08x (read_time=%lldns)\n",
		context, enable, raw, read_time_ns);
	/* Print only active & asserted or delta-interesting events */
	{
		u32 setv, epc, hpc;
		for (i = 0; i < 16; i++) {
			if (!(dx_sw_active_mask & BIT(i)))
				continue; /* not registered */
			setv = ioread32(&blk->set_off[i]);
			if (!setv)
				continue; /* quiet */
			epc = ioread32(&blk->event_irq_cnt[i]);
			hpc = ioread32(&blk->event_handled_cnt[i]);
			dbg_irq("  EV%02d: set=%u EP_CNT=%u HOST_HANDLED=%u (delta=%d)\n", i, setv, epc, hpc, (int)(epc - hpc));
		}
	}
}

/* Latch EP set bits into raw status (do NOT clear set_off here; per-bit clear after handling) */
static u32 dx_sw_irq_latch_and_clear(struct dx_sw_irq_block __iomem *blk)
{
	/* Aggregate per-event set_off[i] into bitfield */
	int i;
	u32 set_bits = 0;
	for (i = 0; i < 16; i++) {
		if (!(dx_sw_active_mask & BIT(i)))
			continue;
		if (ioread32(&blk->set_off[i]))
			set_bits |= BIT(i);
	}
	if (set_bits) {
		u32 raw_before = ioread32(&blk->status_raw_off);
		iowrite32(raw_before | set_bits, &blk->status_raw_off);
	}
	return set_bits;
}

/* Get pending interrupt mask */
static u32 dx_sw_irq_get_pending(struct dx_sw_irq_block __iomem *blk, u32 enable_mask)
{
	return ioread32(&blk->status_raw_off) & enable_mask;
}

/* Increment host-side handled counter for specific interrupt */
static void dx_sw_irq_increment_handled_counter(struct dx_sw_irq_block __iomem *blk, int ev_id)
{
	u32 cnt = ioread32(&blk->event_handled_cnt[ev_id]);
	iowrite32(cnt + 1, &blk->event_handled_cnt[ev_id]);
}

/* Process pending interrupts and call handlers */
static u32 dx_sw_irq_process_pending(struct dw_edma *dw, struct dx_edma_irq *dw_irq,
                                     int irq, u32 pending_mask, struct dx_sw_irq_block __iomem *blk)
{
	u32 handled_mask = 0;
	u8 event_id;
	int i;

	dbg_irq("Processing pending=0x%x\n", pending_mask);
	for (i = 0; i < USER_IRQ_NUMS; i++) {
		if (!(dx_sw_active_mask & BIT(i)))
			continue;
		if (!(pending_mask & BIT(i)))
			continue;
			
		if (!user_irq_vec_table || !user_irq_vec_table[i].handler) {
			break;
		}

		event_id = user_irq_vec_table[i].event_id;
		if (event_id < dw->event_irq_idx) {
			user_irq_service(irq, &dw_irq->user_irqs[event_id]);
		} else {
			user_irq_events(dw_irq, &dw_irq->user_irqs[event_id]);
		}
		dx_sw_irq_increment_handled_counter(blk, i);

		handled_mask |= BIT(i);
	}

	return handled_mask;
}

/* Clear handled interrupt bits with batch operation */
static void dx_sw_irq_clear_handled_bits(struct dw_edma *dw, struct dx_sw_irq_block __iomem *blk, u32 handled_mask)
{
	int i;
	u32 raw_before, raw_after;
	if (!handled_mask)
		return;
	/* Clear status_raw_off bits */
	raw_before = ioread32(&blk->status_raw_off);
	raw_after = raw_before & ~handled_mask;
	iowrite32(raw_after, &blk->status_raw_off);
	/* Clear individual per-event set_off[i] */
	for (i = 0; i < 16; i++) {
		if (!(handled_mask & BIT(i)))
			continue;
		if (!(dx_sw_active_mask & BIT(i)))
			continue;
		if (handled_mask & BIT(i))
			iowrite32(0, &blk->set_off[i]);
	}
	dbg_irq("Cleared handled: raw(0x%x->0x%x) mask=0x%x\n", raw_before, raw_after, handled_mask);
}

/* Main software interrupt handler */
static irqreturn_t dx_sw_irq_handler(struct dw_edma *dw, struct dx_edma_irq *dw_irq, int irq)
{
	struct dx_sw_irq_block __iomem *blk = dx_sw_irq(dw);
	u32 pending_mask, handled_mask = 0;
	u32 enable_mask, set_reg;
	u32 set_count, pending_count;

	enable_mask = ioread32(&blk->enable);
	if (!enable_mask) {
		pr_warn("SWIRQ[%s] enable=0, skipping\n", dw->name);
		return IRQ_NONE;
	}

	/* Debug: Initial state before processing */
	dx_sw_irq_dump_state(dw, "IRQ_START");

	/* Latch set bits into raw and clear set bits */
	set_reg = dx_sw_irq_latch_and_clear(blk);
	set_count = hweight32(set_reg);  /* Count number of set bits from EP */

	if (set_reg)
		dbg_irq("Latched set bits=0x%x (EP_generated=%u)\n", set_reg, set_count);

	/* Get pending interrupts */
	pending_mask = dx_sw_irq_get_pending(blk, enable_mask);
	pending_count = hweight32(pending_mask);  /* Count number of pending bits to process */

	if (!pending_mask) {
		dx_sw_irq_dump_state(dw, "IRQ_NO_PENDING");
		return IRQ_NONE;
	}

	/* Log multiple interrupt coalescing case */
	if (pending_count > 1)
		dbg_irq("COALESCED: Processing %u events (pending=0x%x)\n", pending_count, pending_mask);

	/* Process pending interrupts */
	handled_mask = dx_sw_irq_process_pending(dw, dw_irq, irq, pending_mask, blk);

	/* Clear handled bits with batch operation */
	dx_sw_irq_clear_handled_bits(dw, blk, handled_mask);

#ifdef DX_SW_IRQ_DEBUG
	/* Log processing summary & per-event delta only when debug enabled */
	if (handled_mask) {
		u32 handled_count = hweight32(handled_mask);
		dbg_irq("SUMMARY: EP_gen=%u HOST_pending=%u HOST_handled=%u (set=0x%x pend=0x%x h=0x%x)\n",
			   set_count, pending_count, handled_count, set_reg, pending_mask, handled_mask);
		for (int i = 0; i < USER_IRQ_NUMS; i++) {
			if (!(handled_mask & BIT(i)))
				continue;
			if (!(dx_sw_active_mask & BIT(i)))
				continue;
			dbg_irq("  EV%02d CNT: EP=%u HOST=%u DIFF=%d\n", i,
				ioread32(&blk->event_irq_cnt[i]),
				ioread32(&blk->event_handled_cnt[i]),
				(int)(ioread32(&blk->event_irq_cnt[i]) - ioread32(&blk->event_handled_cnt[i])));
		}
	}
#endif
	
	/* Debug: Final state after processing */
	dx_sw_irq_dump_state(dw, "IRQ_END");
	
	return handled_mask ? IRQ_HANDLED : IRQ_NONE;
}

static void dx_sw_intr_init(struct dw_edma *dw)
{
	int i; 
	u32 enable = 0;
	int active = 0;

	if (dw->nr_irqs != 1) {
		return;
	}

	/* Ensure vector table is available */
	if (!user_irq_vec_table) {
		pr_info("SWIRQ[%s] vector table not set, calling set_user_irq_vec_table\n", dw->name);
		set_user_irq_vec_table(dw);
		if (!user_irq_vec_table) {
			pr_err("SWIRQ[%s] user_irq_vec_table setup failed\n", dw->name);
			return;
		}
	}

	pr_info("SWIRQ[%s] checking handlers in vector table...\n", dw->name);
	
	/* Build enable mask from valid handlers */
	for (i = 0; i < USER_IRQ_NUMS; i++) {
		if (!user_irq_vec_table[i].handler) {
			pr_info("SWIRQ[%s] handler[%d] = NULL, stopping enumeration\n", dw->name, i);
			break;
		}
		enable |= user_irq_vec_table[i].bit;
		active++;
		pr_info("SWIRQ[%s] handler[%d] = %p (%s), bit=0x%x, enable=0x%x\n", 
			dw->name, i, user_irq_vec_table[i].handler, user_irq_vec_table[i].name, 
			user_irq_vec_table[i].bit, enable);
	}

	/* Must have valid handlers for proper operation */
	if (enable == 0) {
		pr_err("SWIRQ[%s] no valid handlers found - INITIALIZATION FAILED\n", dw->name);
		return;
	}

	/* Initialize the shared interrupt block */
	memset_io(dx_sw_irq(dw), 0, sizeof(struct dx_sw_irq_block));
	iowrite32(enable, &dx_sw_irq(dw)->enable);
	dx_sw_active_mask = enable;
	dx_sw_active_count = active;
	pr_info("SWIRQ[%s] active events=%d mask=0x%x\n", dw->name, dx_sw_active_count, dx_sw_active_mask);
	pr_info("SWIRQ[%s] initialized successfully, enable=0x%x\n", dw->name, enable);
}


static inline int get_irq_to_dma_num(int irq_n)
{
	int i, dma_n = -1;
	for (i = 0; i < USER_IRQ_NUMS; i++) {
		if (!user_irq_vec_table[i].handler)
			break;
		if (user_irq_vec_table[i].irq_pos == irq_n) {
			dma_n = user_irq_vec_table[i].dma_ch_n;
			break;
		}
	}
	return dma_n;
}

void set_user_irq_vec_table(struct dw_edma *dw)
{
	if (dw->dx_ver == 3)
		user_irq_vec_table = user_irq_vec_table_v3;
	else
		pr_err("Undefined version error(%d)\n", dw->dx_ver);
}

int get_nr_user_irqs(void)
{
	int i, nr_user_irqs = 0;
	for (i = 0; i < USER_IRQ_NUMS; i++) {
		if (user_irq_vec_table) {
			if (user_irq_vec_table[i].handler)
				nr_user_irqs++;
		}
	}
	return nr_user_irqs;
}

int get_pos_user_irqs(int event_id)
{
	int pos = -1, i;
	for(i = 0; i < USER_IRQ_NUMS; i++) {
		if (user_irq_vec_table[i].handler) {
			if (user_irq_vec_table[i].event_id == event_id) {
				pos = user_irq_vec_table[i].irq_pos;
				break;
			}
		}
	}
	return pos;
}

bool check_event_id(int event_id)
{
	bool match = false;
	int i;
	for(i = 0; i < USER_IRQ_NUMS; i++) {
		if (user_irq_vec_table[i].handler) {
			if (user_irq_vec_table[i].event_id == event_id) {
				match = true;
				break;
			}
		}
	}
	return match;
}

static void dx_user_irq_init(struct dw_edma *dw)
{
	int event_id;
	int i;

	if (dw->nr_irqs == 1) {
		dw->irq[0].dw = dw;
		for (i = 0; i < USER_IRQ_NUMS; i++) {
			spin_lock_init(&dw->irq[0].user_irqs[i].events_lock);
			init_waitqueue_head(&dw->irq[0].user_irqs[i].events_wq);
			dw->irq[0].user_irqs[i].handler = NULL;
			dw->irq[0].user_irqs[i].user_idx = i; /* 0 based */
			if (user_irq_vec_table[i].handler) {
				event_id = user_irq_vec_table[i].event_id;
				strncpy(dw->irq[0].user_irqs[event_id].name,
					user_irq_vec_table[i].name,
					sizeof(user_irq_vec_table[i].name));
			}
			dw->irq[0].user_irqs[i].dw = dw;
		}
	} else {
		for (i = 0; i < dw->nr_irqs; i++) {
			dw->irq[i].dw = dw;
			spin_lock_init(&dw->irq[i].user_irq.events_lock);
			init_waitqueue_head(&dw->irq[i].user_irq.events_wq);
			dw->irq[i].user_irq.handler = NULL;
			dw->irq[i].user_irq.user_idx = i - dw->dma_irqs;;
			dw->irq[i].user_irq.dw = dw;
		}
	}
}
static inline
struct device *dchan2dev(struct dma_chan *dchan)
{
	return &dchan->dev->device;
}

static inline
struct device *chan2dev(struct dw_edma_chan *chan)
{
	return &chan->vc.chan.dev->device;
}

static inline
struct dw_edma_desc *vd2dw_edma_desc(struct virt_dma_desc *vd)
{
	return container_of(vd, struct dw_edma_desc, vd);
}





/*
 * Deferred desc free - runs in process context via system workqueue.
 * This is scheduled when vchan_free_desc() is called from tasklet/IRQ context
 * (e.g., vchan_complete tasklet after DMA completion interrupt).
 *
 * In process context, it is safe to:
 * - call cancel_work_sync() (may sleep)
 * - call dma_free_coherent() (with IOMMU, calls vunmap() which requires
 *   process context — BUG_ON(in_interrupt()) at mm/vmalloc.c)
 */
static void dw_edma_deferred_free_desc_work(struct work_struct *work)
{
	struct dw_edma_desc *desc = container_of(work, struct dw_edma_desc,
						 deferred_free_work);
	dw_edma_free_desc(desc);
}

static void vchan_free_desc(struct virt_dma_desc *vdesc)
{
	struct dw_edma_desc *desc = vd2dw_edma_desc(vdesc);

	if (in_interrupt() || in_atomic()) {
		/*
		 * Called from tasklet (vchan_complete) or interrupt context.
		 * dw_edma_free_desc() is NOT safe here because:
		 *
		 * 1) cancel_work_sync(&desc->cleanup_work) may sleep
		 * 2) dw_edma_desc_cleanup_work() calls dma_free_coherent()
		 *    which with IOMMU enabled calls vunmap() →
		 *    BUG_ON(in_interrupt()) at mm/vmalloc.c:3416
		 *
		 * Defer the entire desc cleanup to process context.
		 */
		INIT_WORK(&desc->deferred_free_work,
			  dw_edma_deferred_free_desc_work);
		schedule_work(&desc->deferred_free_work);
		return;
	}

	dw_edma_free_desc(desc);
}

/*
 * Check and fix MSI mismatch before DMA transfer (Single MSI mode only).
 * Called only when nr_irqs == 1. Detects if irqbalance changed MSI address
 * and updates EP DMA registers accordingly.
 */
static void dx_dma_check_and_fix_msi(struct dw_edma *dw)
{
	struct msi_msg pci_msi;
	struct dx_edma_irq *dw_irq = &dw->irq[0];
	struct dw_edma_chan *chan;
	int i;

	/* Read current MSI from PCI config space */
	if (dx_pci_read_msi_msg(dw->pdev, &pci_msi) != 0)
		return;

	/* Check if MSI changed since last update */
	if (pci_msi.address_lo == dw_irq->msi.address_lo &&
	    pci_msi.address_hi == dw_irq->msi.address_hi &&
	    pci_msi.data == dw_irq->msi.data) {
		return;  /* No change */
	}

	/* MSI mismatch detected - update cached value and EP registers */
	pr_debug("dx_dma: MSI config updated by irqbalance - [addr=0x%x_%x, data=0x%x] -> [addr=0x%x_%x, data=0x%x]\n",
		dw_irq->msi.address_hi, dw_irq->msi.address_lo, dw_irq->msi.data,
		pci_msi.address_hi, pci_msi.address_lo, pci_msi.data);

	/* Update cached MSI */
	memcpy(&dw_irq->msi, &pci_msi, sizeof(pci_msi));

	/* Update all DMA channels */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		chan = &dw->chan[i];
		memcpy(&chan->msi, &pci_msi, sizeof(pci_msi));
		dw_edma_v0_core_device_config(chan);
	}
}

static void dw_edma_start_transfer(struct dw_edma_chan *chan)
{
	struct dw_edma_chunk *child;
	struct dw_edma_desc *desc;
	struct virt_dma_desc *vd;
	struct dw_edma *dw = chan->chip->dw;
	dbg_core("dw_edma_start_transfer!!\n");

	/* Check and fix MSI mismatch before starting DMA (Single MSI mode only) */
	if (dw->nr_irqs == 1)
		dx_dma_check_and_fix_msi(dw);

	vd = vchan_next_desc(&chan->vc);
	if (!vd) {
		dev_err(chan->chip->dev, "vd is null\n");
		return;
	}

	desc = vd2dw_edma_desc(vd);
	if (!desc) {
		dev_err(chan->chip->dev, "desc is null\n");
		return;
	}

	child = list_first_entry_or_null(&desc->chunk->list,
					 struct dw_edma_chunk, list);
	if (!child) {
		/* 
		 * If the list is empty, it means the descriptor is already consumed 
		 * or in an invalid state. Instead of erroring out and hanging the channel,
		 * complete the descriptor and reset status to IDLE.
		 */
		dev_warn(chan->chip->dev, "Child is null (chunks_alloc:%d). Forcing completion.\n", desc->chunks_alloc);
		list_del(&vd->node);
		vchan_cookie_complete(vd);
		chan->status = EDMA_ST_IDLE;
		return;
	}

	dw_edma_v0_core_start(child, !desc->xfer_sz, chan->set_desc, chan->is_llm);
	
	/* 
	 * Do NOT free the chunk here. 
	 * The chunk must remain valid while the DMA is running.
	 * It will be freed in dw_edma_done_interrupt() after completion.
	 */
}

static int dw_edma_device_config(struct dma_chan *dchan,
				 struct dma_slave_config *config)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	dbg_core("[%s] start!!\n", __func__);

	memcpy(&chan->config, config, sizeof(*config));
	chan->configured = true;
	chan->aborted = false;  /* Clear stale abort flag from previous transfer */

	return 0;
}

static int dw_edma_device_pause(struct dma_chan *dchan)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	int err = 0;
	dbg_core("[%s] start!!\n", __func__);

	if (!chan->configured)
		err = -EPERM;
	else if (chan->status != EDMA_ST_BUSY)
		err = -EPERM;
	else if (chan->request != EDMA_REQ_NONE)
		err = -EPERM;
	else
		chan->request = EDMA_REQ_PAUSE;

	return err;
}

static int dw_edma_device_resume(struct dma_chan *dchan)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	int err = 0;
	dbg_core("[%s] start!!\n", __func__);

	if (!chan->configured) {
		err = -EPERM;
	} else if (chan->status != EDMA_ST_PAUSE) {
		err = -EPERM;
	} else if (chan->request != EDMA_REQ_NONE) {
		err = -EPERM;
	} else {
		chan->status = EDMA_ST_BUSY;
		dw_edma_start_transfer(chan);
	}

	return err;
}

static int dw_edma_device_terminate_all(struct dma_chan *dchan)
{
	struct virt_dma_chan *vc = to_virt_chan(dchan);
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	unsigned long flags;
	LIST_HEAD(head);
	int err = 0;

	dbg_core("[%s] start for channel %s!!\n", __func__, dma_chan_name(dchan));

	/*
	 * Hold vc.lock across the entire state machine to prevent races
	 * with issue_pending(), which checks configured/request/status
	 * under the same lock.  Without this, a concurrent issue_pending
	 * can see a partially-updated state and start a transfer on a
	 * channel that is being torn down.
	 */
	spin_lock_irqsave(&vc->lock, flags);

	if (!chan->configured) {
		/* Do nothing */
	} else if (chan->status == EDMA_ST_PAUSE) {
		chan->status = EDMA_ST_IDLE;
		chan->configured = false;
	} else if (chan->status == EDMA_ST_IDLE) {
		/*
		 * SW is IDLE but HW may still be in CS=2 error state
		 * (e.g., abort ISR set status=IDLE before thread called
		 * terminate_all).  Check HW and clear if needed, otherwise
		 * the next transfer hits "channel in error state" → "skipping
		 * doorbell" and fails permanently.
		 */
		if (dw_edma_v0_core_ch_status_raw(chan) == 2)
			dw_edma_v0_core_ch_soft_reset(chan);
		chan->configured = false;
	} else if (dw_edma_v0_core_ch_status(chan) == DMA_COMPLETE) {
		/*
		 * The channel is in a false BUSY state, probably didn't
		 * receive or lost an interrupt
		 */
		chan->status = EDMA_ST_IDLE;
		chan->configured = false;
	} else if (chan->request > EDMA_REQ_PAUSE) {
		err = -EPERM;
	} else if (chan->request == EDMA_REQ_STOP) {
		/*
		 * EDMA_REQ_STOP was already set (first terminate_all call)
		 * but Done ISR never fired - channel timed out.
		 * Try per-channel recovery first; fall back to engine reset.
		 */
		u32 cs = dw_edma_v0_core_ch_status_raw(chan);

		if (cs == 2) {
			/*
			 * CS=2: channel halted on error (e.g. PCIe completion
			 * error).  ch_soft_reset performs:
			 *   1. Read err_status (RC) to acknowledge the error
			 *   2. Clear abort/done interrupt status bits
			 *   3. engine_en=0->1 cycle to clear CS=2
			 *   4. Verify CS is no longer 2
			 *
			 * NOTE: engine_en cycle resets ALL channels on this
			 * direction (read or write).  CS bits are read-only HW
			 * status — there is no per-channel-only HW recovery.
			 */
			if (dw_edma_v0_core_ch_soft_reset(chan) == 0) {
				pr_info("Channel %d: CS=2, soft reset OK\n",
					chan->id);
			} else {
				pr_err("Channel %d: CS=2, soft reset FAILED\n",
					chan->id);
				chan->hw_err = true;
			}
		} else {
			/*
			 * CS!=2: channel may still be running or in an
			 * unknown state.  Engine reset is the only way to
			 * force-stop it.
			 * WARNING: kills ALL channels on this direction.
			 */
			pr_warn("Channel %d: CS=%u, forcing engine reset\n",
				chan->id, cs);
			dw_edma_v0_core_engine_cycle(chan);
			dw_edma_v0_core_clear_done_int(chan);
			dw_edma_v0_core_clear_abort_int(chan);
		}

		/* Common SW cleanup: free descriptors + return to IDLE.
		 * Drain ALL descriptor lists — including desc_completed
		 * and desc_submitted — to prevent stale tasklet callbacks
		 * from setting done->done on the NEXT transfer.  Without
		 * this, the vchan_complete tasklet can fire after we start
		 * a new transfer and wake it prematurely (done->done=true
		 * but cookie not yet complete → "completion busy" error). */
		/* vc.lock already held by outer lock */
		list_splice_tail_init(&vc->desc_allocated, &head);
		list_splice_tail_init(&vc->desc_submitted, &head);
		list_splice_tail_init(&vc->desc_issued, &head);
		list_splice_tail_init(&vc->desc_completed, &head);

		chan->request = EDMA_REQ_NONE;
		chan->status = EDMA_ST_IDLE;
		chan->configured = false;

		/* Drop lock before freeing descriptors -- free callbacks
		 * may do memory operations incompatible with spinlock. */
		spin_unlock_irqrestore(&vc->lock, flags);
		if (!list_empty(&head))
			vchan_dma_desc_free_list(vc, &head);
		spin_lock_irqsave(&vc->lock, flags);
	} else {
		chan->request = EDMA_REQ_STOP;
		dbg_core("Channel %d: EDMA_REQ_STOP set, Done ISR will complete.\n", chan->id);
	}

	spin_unlock_irqrestore(&vc->lock, flags);
	return err;
}

static void dw_edma_device_issue_pending(struct dma_chan *dchan)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	unsigned long flags;

	spin_lock_irqsave(&chan->vc.lock, flags);
	dbg_core("[%s] start!!\n", __func__);

	if (chan->configured && chan->request == EDMA_REQ_NONE &&
	    chan->status == EDMA_ST_IDLE && vchan_issue_pending(&chan->vc)) {
		chan->status = EDMA_ST_BUSY;
		dw_edma_start_transfer(chan);
	}
	spin_unlock_irqrestore(&chan->vc.lock, flags);
}

static enum dma_status dw_edma_device_tx_status(struct dma_chan *dchan, dma_cookie_t cookie,
			 struct dma_tx_state *txstate)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	struct dw_edma_desc *desc;
	struct virt_dma_desc *vd;
	unsigned long flags;
	enum dma_status ret;
	u32 residue = 0;

	dbg_core("[%s] start!!\n", __func__);
	ret = dma_cookie_status(dchan, cookie, txstate);
	if (ret == DMA_COMPLETE)
		return ret;

	if (ret == DMA_IN_PROGRESS && chan->status == EDMA_ST_PAUSE)
		ret = DMA_PAUSED;

	if (!txstate)
		goto ret_residue;

	spin_lock_irqsave(&chan->vc.lock, flags);
	vd = vchan_find_desc(&chan->vc, cookie);
	if (vd) {
		desc = vd2dw_edma_desc(vd);
		if (desc)
			residue = desc->alloc_sz - desc->xfer_sz;
	}
	spin_unlock_irqrestore(&chan->vc.lock, flags);

ret_residue:
	dma_set_residue(txstate, residue);

	return ret;
}

static struct dma_async_tx_descriptor *dw_edma_device_transfer(struct dw_edma_transfer *xfer)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(xfer->dchan);
	enum dma_transfer_direction dir = xfer->direction;
	phys_addr_t src_addr, dst_addr;
	struct scatterlist *sg = NULL;
	struct dw_edma_chunk *chunk;
	struct dw_edma_burst *burst;
	struct dw_edma_desc *desc;
	u32 cnt = 0;
	int i;

	if (!chan->configured) {
		pr_err("DMA channel configure is missed\n");
		return NULL;
	}

	switch (chan->config.direction) {
		case DMA_DEV_TO_MEM: /* local DMA */
			if (dir == DMA_DEV_TO_MEM && chan->dir == EDMA_DIR_READ)
				break;
			pr_err("[DMA_DEV_TO_MEM] DMA direction is wrong(dir:%d, chan->dir:%d, ch_id:%d)\n", dir, chan->dir, chan->id);
			return NULL;
		case DMA_MEM_TO_DEV: /* local DMA */
			if (dir == DMA_MEM_TO_DEV && chan->dir == EDMA_DIR_WRITE)
				break;
			pr_err("[DMA_MEM_TO_DEV] DMA direction is wrong(dir:%d, chan->dir:%d, ch_id:%d)\n", dir, chan->dir, chan->id);
			return NULL;
		default: /* remote DMA */
			if (dir == DMA_MEM_TO_DEV && chan->dir == EDMA_DIR_READ)
				break;
			if (dir == DMA_DEV_TO_MEM && chan->dir == EDMA_DIR_WRITE)
				break;
			pr_err("[Remote] DMA direction is wrong(dir:%d, chan->dir:%d, ch_id:%d)\n", dir, chan->dir, chan->id);
			return NULL;
	}

	if (xfer->type == EDMA_XFER_CYCLIC) {
		if (!xfer->xfer.cyclic.len || !xfer->xfer.cyclic.cnt)
			return NULL;
	} else if (xfer->type == EDMA_XFER_SCATTER_GATHER) {
		if (xfer->xfer.sg.len < 1) {
			pr_err("Scatter Gatter list length is 0\n");
			return NULL;
		}
	} else if (xfer->type == EDMA_XFER_INTERLEAVED) {
		if (!xfer->xfer.il->numf)
			return NULL;
		if (xfer->xfer.il->numf > 0 && xfer->xfer.il->frame_size > 0)
			return NULL;
	} else {
		return NULL;
	}

	desc = dw_edma_alloc_desc(chan);
	if (unlikely(!desc))
		goto err_alloc;

	chunk = dw_edma_alloc_chunk(desc);
	if (unlikely(!chunk))
		goto err_alloc;

	if (xfer->type == EDMA_XFER_INTERLEAVED) {
		src_addr = xfer->xfer.il->src_start;
		dst_addr = xfer->xfer.il->dst_start;
	} else {
		src_addr = chan->config.src_addr;
		dst_addr = chan->config.dst_addr;
	}

	if (xfer->type == EDMA_XFER_CYCLIC) {
		cnt = xfer->xfer.cyclic.cnt;
	} else if (xfer->type == EDMA_XFER_SCATTER_GATHER) {
		cnt = xfer->xfer.sg.len;
		sg = xfer->xfer.sg.sgl;
	} else if (xfer->type == EDMA_XFER_INTERLEAVED) {
		if (xfer->xfer.il->numf > 0)
			cnt = xfer->xfer.il->numf;
		else
			cnt = xfer->xfer.il->frame_size;
	}

	for (i = 0; i < cnt; i++) {
		if (xfer->type == EDMA_XFER_SCATTER_GATHER && !sg)
			break;

		if (chunk->bursts_alloc == chan->ll_max) {
			chunk = dw_edma_alloc_chunk(desc);
			if (unlikely(!chunk))
				goto err_alloc;
		}

		burst = dw_edma_alloc_burst(chunk);
		if (unlikely(!burst))
			goto err_alloc;

		if (xfer->type == EDMA_XFER_CYCLIC)
			burst->sz = xfer->xfer.cyclic.len;
		else if (xfer->type == EDMA_XFER_SCATTER_GATHER)
			burst->sz = sg_dma_len(sg);
		else if (xfer->type == EDMA_XFER_INTERLEAVED)
			burst->sz = xfer->xfer.il->sgl[i].size;

		chunk->ll_region.sz += burst->sz;
		desc->alloc_sz += burst->sz;

		if (chan->dir == EDMA_DIR_WRITE) {
			burst->sar = src_addr;
			if (xfer->type == EDMA_XFER_CYCLIC) {
				burst->dar = xfer->xfer.cyclic.paddr;
			} else if (xfer->type == EDMA_XFER_SCATTER_GATHER) {
				src_addr += sg_dma_len(sg);
				burst->dar = sg_dma_address(sg);
				/* Unlike the typical assumption by other
				 * drivers/IPs the peripheral memory isn't
				 * a FIFO memory, in this case, it's a
				 * linear memory and that why the source
				 * and destination addresses are increased
				 * by the same portion (data length)
				 */
			}
		} else {
			burst->dar = dst_addr;
			if (xfer->type == EDMA_XFER_CYCLIC) {
				burst->sar = xfer->xfer.cyclic.paddr;
			} else if (xfer->type == EDMA_XFER_SCATTER_GATHER) {
				dst_addr += sg_dma_len(sg);
				burst->sar = sg_dma_address(sg);
				/* Unlike the typical assumption by other
				 * drivers/IPs the peripheral memory isn't
				 * a FIFO memory, in this case, it's a
				 * linear memory and that why the source
				 * and destination addresses are increased
				 * by the same portion (data length)
				 */
			}
		}

		if ((i == 0) || (i == cnt-1)) {
			dbg_tfr("[%s][%d/%d]: Type:%d, DIR:%d, SAR:%llx DAR:%llx \n", \
				__func__, i+1, cnt,
				xfer->type, chan->dir, burst->sar, burst->dar);
		}

		if (xfer->type == EDMA_XFER_SCATTER_GATHER) {
			sg = sg_next(sg);
		} else if (xfer->type == EDMA_XFER_INTERLEAVED &&
			   xfer->xfer.il->frame_size > 0) {
			struct dma_interleaved_template *il = xfer->xfer.il;
			struct data_chunk *dc = &il->sgl[i];

			if (il->src_sgl) {
				src_addr += burst->sz;
				src_addr += dmaengine_get_src_icg(il, dc);
			}

			if (il->dst_sgl) {
				dst_addr += burst->sz;
				dst_addr += dmaengine_get_dst_icg(il, dc);
			}
		}
	}

	return vchan_tx_prep(&chan->vc, &desc->vd, xfer->flags);

err_alloc:
	if (desc)
		dw_edma_free_desc(desc);

	return NULL;
}

static struct dma_async_tx_descriptor *dw_edma_device_prep_slave_sg(struct dma_chan *dchan, struct scatterlist *sgl,
			     unsigned int len,
			     enum dma_transfer_direction direction,
			     unsigned long flags, void *context)
{
	struct dw_edma_transfer xfer;
	dbg_tfr("[%s][%s] dir:%d, len:0x%x\n", __func__, dma_chan_name(dchan), direction, len);

	xfer.dchan = dchan;
	xfer.direction = direction;
	xfer.xfer.sg.sgl = sgl;
	xfer.xfer.sg.len = len;
	xfer.flags = flags;
	xfer.type = EDMA_XFER_SCATTER_GATHER;

	return dw_edma_device_transfer(&xfer);
}

static struct dma_async_tx_descriptor *dw_edma_device_prep_dma_cyclic(struct dma_chan *dchan, dma_addr_t paddr,
			       size_t len, size_t count,
			       enum dma_transfer_direction direction,
			       unsigned long flags)
{
	struct dw_edma_transfer xfer;

	xfer.dchan = dchan;
	xfer.direction = direction;
	xfer.xfer.cyclic.paddr = paddr;
	xfer.xfer.cyclic.len = len;
	xfer.xfer.cyclic.cnt = count;
	xfer.flags = flags;
	xfer.type = EDMA_XFER_CYCLIC;

	return dw_edma_device_transfer(&xfer);
}

static struct dma_async_tx_descriptor *dw_edma_device_prep_interleaved_dma(struct dma_chan *dchan,
				    struct dma_interleaved_template *ilt,
				    unsigned long flags)
{
	struct dw_edma_transfer xfer;

	xfer.dchan = dchan;
	xfer.direction = ilt->dir;
	xfer.xfer.il = ilt;
	xfer.flags = flags;
	xfer.type = EDMA_XFER_INTERLEAVED;

	return dw_edma_device_transfer(&xfer);
}

static irqreturn_t user_irq_service(int irq, struct dx_dma_user_irq *user_irq)
{
	unsigned long flags;
	dbg_irq("event irq data : %d[%p,%p]\n", user_irq->events_irq, &user_irq->events_irq, &user_irq->events_wq);

	if (!user_irq) {
		pr_err("Invalid user_irq\n");
		return IRQ_NONE;
	}

#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
	{
		int dma_n = get_irq_to_dma_num(user_irq->user_idx);
		dbg_irq("user_idx:%d, dma_n:%d\n", user_irq->user_idx, dma_n);
		if (dma_n >= 0)
			dx_pcie_enqueue_response(user_irq->dw->idx, dma_n);
		else
			pr_err("Irq number is not mapped in vector table(%d)\n", dma_n);
	}
#endif

	spin_lock_irqsave(&(user_irq->events_lock), flags);
	if (!user_irq->events_irq) {
		user_irq->events_irq = 1;
		wake_up_interruptible(&(user_irq->events_wq));
	}
	spin_unlock_irqrestore(&(user_irq->events_lock), flags);

	return IRQ_HANDLED;
}

static irqreturn_t user_irq_events(struct dx_edma_irq *dw_irq, struct dx_dma_user_irq *user_irq)
{
	unsigned long flags;

#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
	dx_pcie_enqueue_event_response(dw_irq->dw->idx, 0);
#endif

	spin_lock_irqsave(&(user_irq->events_lock), flags);
	if (!user_irq->events_irq) {
		user_irq->events_irq = 1;
		wake_up_interruptible(&(user_irq->events_wq));
	}
	spin_unlock_irqrestore(&(user_irq->events_lock), flags);
	return IRQ_HANDLED;
}

static void dw_edma_done_interrupt(struct dw_edma_chan *chan)
{
	struct dw_edma_desc *desc;
	struct virt_dma_desc *vd;
	struct dw_edma_chunk *child;
	unsigned long flags;
	dbg_core("[%s] start!!\n", __func__);

	dw_edma_v0_core_clear_done_int(chan);

	spin_lock_irqsave(&chan->vc.lock, flags);
	vd = vchan_next_desc(&chan->vc);
	if (vd) {
		desc = vd2dw_edma_desc(vd);
		
		/* 
		 * The first chunk in the list is the one that just completed.
		 * We can now safely free it.
		 */
		child = list_first_entry_or_null(&desc->chunk->list,
						 struct dw_edma_chunk, list);
		if (child) {
			desc->xfer_sz += child->ll_region.sz;
			dw_edma_free_burst(child);
			
			if (child->from_pool) {
				struct dw_edma *dw = chan->chip->dw;
				unsigned long pool_flags;
				
				list_del(&child->list);
				spin_lock_irqsave(&dw->pool_lock, pool_flags);
				dw->chunk_free_list[dw->chunk_free_cnt++] = (child - dw->chunk_pool);
				spin_unlock_irqrestore(&dw->pool_lock, pool_flags);
				dev_vdbg(chan->chip->dev, "[MEM][CHUNK][POOL] Free: idx=%ld addr=%p buf_paddr=%pad buf_vaddr=%p\n", (child - dw->chunk_pool), child, &child->host_region.paddr, child->host_region.vaddr);
			} else {
				/* Non-pool chunk: Move to descriptor's pending free list
				 * Will be freed by work after all chunks complete */
				list_del(&child->list);
				list_add_tail(&child->list, &desc->pending_free_chunks);
			}
			desc->chunks_alloc--;
		}

		switch (chan->request) {
		case EDMA_REQ_NONE:
			if (desc->chunks_alloc) {
				chan->status = EDMA_ST_BUSY;
				dw_edma_start_transfer(chan);
			} else {
				/* All chunks transferred - schedule cleanup work NOW */
				if (!list_empty(&desc->pending_free_chunks))
					schedule_work(&desc->cleanup_work);
				
				list_del(&vd->node);
				vchan_cookie_complete(vd);
				chan->status = EDMA_ST_IDLE;
			}
			break;

		case EDMA_REQ_STOP:
			/* Also schedule cleanup on stop */
			if (!list_empty(&desc->pending_free_chunks))
				schedule_work(&desc->cleanup_work);
			
			list_del(&vd->node);
			vchan_cookie_complete(vd);
			chan->request = EDMA_REQ_NONE;
			chan->status = EDMA_ST_IDLE;
			break;

		case EDMA_REQ_PAUSE:
			chan->request = EDMA_REQ_NONE;
			chan->status = EDMA_ST_PAUSE;
			break;

		default:
			break;
		}
	}
	spin_unlock_irqrestore(&chan->vc.lock, flags);
}

static void dw_edma_abort_interrupt(struct dw_edma_chan *chan)
{
	struct virt_dma_desc *vd;
	unsigned long flags;
	struct dw_edma *dw = chan->chip->dw;
	u32 err_status;

	dev_err(&dw->pdev->dev,
		"DMA abort interrupt on dev %u, ch %d, dir %s\n",
		dw->idx, chan->id,
		chan->dir == EDMA_DIR_WRITE ? "WRITE" : "READ");

	/*
	 * Acknowledge the error: read err_status (Read-Clear register),
	 * clear abort and done interrupt status bits.
	 * No engine_en cycle here â that would kill all channels on
	 * this direction.  Full HW reset is deferred to user-space
	 * recovery (DXRT_CMD_RECOVERY ioctl).
	 */
	err_status = dw_edma_v0_core_ch_recover_abort(chan);

	/*
	 * Mark channel as aborted BEFORE completing the descriptor.
	 * dw_edma_sg_process checks this flag after wakeup so it can
	 * report -EIO instead of success.
	 */
	WRITE_ONCE(chan->aborted, true);

	spin_lock_irqsave(&chan->vc.lock, flags);
	vd = vchan_next_desc(&chan->vc);
	if (vd) {
		list_del(&vd->node);
		vchan_cookie_complete(vd);
	}
	spin_unlock_irqrestore(&chan->vc.lock, flags);
	chan->request = EDMA_REQ_NONE;
	chan->status = EDMA_ST_IDLE;

	/*
	 * Notify user-space with enriched DMA abort event.
	 * Includes err_status register and all channel CS values
	 * so the monitoring daemon can log diagnostics and decide
	 * whether to trigger full recovery.
	 */
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
	{
		u32 wr_sts[4] = {0}, rd_sts[4] = {0};
		int i;

		for (i = 0; i < dw->wr_ch_cnt && i < 4; i++)
			wr_sts[i] = dw_edma_v0_core_ch_status_raw(
						&dw->chan[i]);
		for (i = 0; i < dw->rd_ch_cnt && i < 4; i++)
			rd_sts[i] = dw_edma_v0_core_ch_status_raw(
						&dw->chan[dw->wr_ch_cnt + i]);

		dx_pcie_enqueue_abort_event(dw->idx,
			ERR_PCIE_DMA_CH_ABORT(chan->id),
			err_status, wr_sts, rd_sts);
	}
#endif
}

static irqreturn_t dw_edma_interrupt(int irq, void *data, bool write)
{
	struct dx_edma_irq *dw_irq = data;
	struct dw_edma *dw = dw_irq->dw;
	irqreturn_t ret = IRQ_NONE;
	unsigned long total, pos, val;
	unsigned long off;
	u32 mask;
#ifdef DMA_PERF_MEASURE
	struct dw_edma_info *info;
#endif

	if (write) {
		total = dw->wr_ch_cnt;
		off = 0;
		mask = dw_irq->wr_mask;
	} else {
		total = dw->rd_ch_cnt;
		off = dw->wr_ch_cnt;
		mask = dw_irq->rd_mask;
	}
	dbg_irq("[%d] MSI [addr:0x%x_%x, data:0x%x]\n",
		irq,
		dw_irq->msi.address_hi,
		dw_irq->msi.address_lo,
		dw_irq->msi.data);
	dbg_irq("[%d] total:0x%lx, off:0x%lx, mask:0x%x\n", irq, total, off, mask);

	val = dw_edma_v0_core_status_done_int(dw, write ?
							  EDMA_DIR_WRITE :
							  EDMA_DIR_READ);
	if (val) {
		dbg_irq("[%d][%s] Done interrupt status:0x%lx\n",
			irq, write ? "WRITE" : "READ", val);
	}

	val &= mask;
#ifdef DMA_PERF_MEASURE
	if (val) {
		for_each_set_bit(pos, &val, total) {
			info = dw_irq->data[pos][!write];
			if (info) {
				dbg_irq("(pointer : %p, pos:%ld, write:%d, size :%lu)\n", info, pos, info->cb->write, info->cb->len);
				dx_pcie_end_profile(PCIE_DMA_XFER_T, info->cb->len, info->dev_n, pos, info->cb->write);
				dx_pcie_start_profile(PCIE_ISR_EXEC_T, info->cb->len, info->dev_n, pos, info->cb->write);
			} else {
				dbg_irq("Null pointer error!(pointer : %p, pos:%ld, write:%d)\n", info, pos, write);
			}
		}
	}
#endif
	for_each_set_bit(pos, &val, total) {
		struct dw_edma_chan *chan = &dw->chan[pos + off];

		dw_edma_done_interrupt(chan);
		ret = IRQ_HANDLED;
	}

	val = dw_edma_v0_core_status_abort_int(dw, write ?
							   EDMA_DIR_WRITE :
							   EDMA_DIR_READ);
	if (val) {
		dbg_irq("[%d][%s] Abort interrupt status:0x%lx\n",
			irq, write ? "WRITE" : "READ", val);
	}

	val &= mask;
	for_each_set_bit(pos, &val, total) {
		struct dw_edma_chan *chan = &dw->chan[pos + off];

		dw_edma_abort_interrupt(chan);
		ret = IRQ_HANDLED;
	}

	return ret;
}

static inline irqreturn_t dw_edma_interrupt_write(int irq, void *data)
{
	return dw_edma_interrupt(irq, data, true);
}

static inline irqreturn_t dw_edma_interrupt_read(int irq, void *data)
{
	return dw_edma_interrupt(irq, data, false);
}

/* ---------------------------------------------- */
/* Single MSI (nr_irqs == 1) unified ISR handling */
/* ---------------------------------------------- */
static irqreturn_t dw_edma_single_msi_interrupt(int irq, void *data)
{
    struct dx_edma_irq *dw_irq = data;
    struct dw_edma *dw = dw_irq->dw;
    irqreturn_t ret = IRQ_NONE;
    irqreturn_t r;

    r = dw_edma_interrupt_write(irq, data);
    if (r == IRQ_HANDLED)
        ret = IRQ_HANDLED;

    r = dw_edma_interrupt_read(irq, data);
    if (r == IRQ_HANDLED)
        ret = IRQ_HANDLED;

    if (dw->nr_irqs == 1) {
        r = dx_sw_irq_handler(dw, dw_irq, irq);
        if (r == IRQ_HANDLED)
            ret = IRQ_HANDLED;
    }

    return ret;
}

static irqreturn_t dw_edma_user_irq_npu(int irq, void *data)
{
	struct dx_edma_irq *dw_irq = data;
	struct dx_dma_user_irq *user_irq = &dw_irq->user_irq;

	dbg_irq("[%d][%s] called!! data:%p, name:%s, msi:0x%x, data:0x%x\n",
		irq, __func__, data, user_irq->name, dw_irq->msi.address_lo, dw_irq->msi.data);

	user_irq_service(irq, user_irq); /* To do check this point */

	return IRQ_HANDLED;
}

static irqreturn_t dw_edma_user_events(int irq, void *data)
{
	struct dx_edma_irq *dw_irq = data;
	struct dx_dma_user_irq *user_irq = &dw_irq->user_irq;

	dbg_irq("[%d][%s] called!! data:%p, name:%s, msi:0x%x\n",
		irq, __func__, data, user_irq->name, dw_irq->msi.address_lo);

	user_irq_events(dw_irq, user_irq);
	return IRQ_HANDLED;
}

static irqreturn_t dw_edma_interrupt_common(int irq, void *data)
{
	dw_edma_interrupt(irq, data, true);
	dw_edma_interrupt(irq, data, false);
	return IRQ_HANDLED;
}

static int dw_edma_alloc_chan_resources(struct dma_chan *dchan)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	dbg_core("[%s] start!!\n", __func__);

	if (chan->status != EDMA_ST_IDLE)
		return -EBUSY;

	pm_runtime_get(chan->chip->dev);

	return 0;
}

static void dw_edma_free_chan_resources(struct dma_chan *dchan)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(5000);
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	int ret;
	dbg_core("[%s] start!!\n", __func__);

	while (time_before(jiffies, timeout)) {
		ret = dw_edma_device_terminate_all(dchan);
		if (!ret)
			break;

		if (time_after_eq(jiffies, timeout))
			return;

		cpu_relax();
	}

	pm_runtime_put(chan->chip->dev);
}

static int dw_edma_channel_setup(struct dw_edma_chip *chip, bool write,
				 u32 wr_alloc, u32 rd_alloc)
{
	struct dx_edma_region *dt_region;
	struct device *dev = chip->dev;
	struct dw_edma *dw = chip->dw;
	struct dw_edma_chan *chan;
	struct dx_edma_irq *irq;
	struct dma_device *dma;
	u32 alloc, off_alloc;
	u32 i, j, cnt;
	int err = 0;
	u32 pos;

	if (write) {
		i = 0;
		cnt = dw->wr_ch_cnt;
		dma = &dw->wr_edma;
		alloc = wr_alloc;
		off_alloc = 0;
	} else {
		i = dw->wr_ch_cnt;
		cnt = dw->rd_ch_cnt;
		dma = &dw->rd_edma;
		alloc = rd_alloc;
		off_alloc = wr_alloc;
	}

	INIT_LIST_HEAD(&dma->channels);
	for (j = 0; (alloc || dw->nr_irqs == 1) && j < cnt; j++, i++) {
		chan = &dw->chan[i];

		dt_region = devm_kzalloc(dev, sizeof(*dt_region), GFP_KERNEL);
		if (!dt_region)
			return -ENOMEM;

		chan->vc.chan.private = dt_region;

		chan->chip = chip;
		chan->id = j;
		chan->dir = write ? EDMA_DIR_WRITE : EDMA_DIR_READ;
		chan->configured = false;
		chan->request = EDMA_REQ_NONE;
		chan->status = EDMA_ST_IDLE;
		chan->aborted = false;

		if (write)
			chan->ll_max = (dw->ll_region_wr[j].sz / EDMA_LL_SZ);
		else
			chan->ll_max = (dw->ll_region_rd[j].sz / EDMA_LL_SZ);
		chan->ll_max -= 1;

		dev_vdbg(dev, "L. List:\tChannel %s[%u] max_cnt=%u, max_size=%uKB\n",
			 write ? "write" : "read", j, chan->ll_max, chan->ll_max*4);

		// if (dw->nr_irqs == 1)
			pos = 0;
		// else
		// 	pos = off_alloc + (j % alloc);
		// pos = j;

		irq = &dw->irq[pos];

		if (write)
			irq->wr_mask |= BIT(j);
		else
			irq->rd_mask |= BIT(j);

		irq->dw = dw;
		memcpy(&chan->msi, &irq->msi, sizeof(chan->msi));

		dev_vdbg(dev, "MSI:\t\tChannel %s[%u] addr=0x%.8x%.8x, data=0x%.8x\n",
			 write ? "write" : "read", j,
			 chan->msi.address_hi, chan->msi.address_lo,
			 chan->msi.data);

		chan->vc.desc_free = vchan_free_desc;
		vchan_init(&chan->vc, dma);

		dw_edma_v0_core_device_config(chan);
	}

	/* Set DMA channel capabilities */
	dma_cap_zero(dma->cap_mask);
	dma_cap_set(DMA_SLAVE, dma->cap_mask);
	dma_cap_set(DMA_CYCLIC, dma->cap_mask);
	dma_cap_set(DMA_PRIVATE, dma->cap_mask);
	dma_cap_set(DMA_INTERLEAVE, dma->cap_mask);
	dma->directions = BIT(write ? DMA_DEV_TO_MEM : DMA_MEM_TO_DEV);
	dma->src_addr_widths = BIT(DMA_SLAVE_BUSWIDTH_4_BYTES);
	dma->dst_addr_widths = BIT(DMA_SLAVE_BUSWIDTH_4_BYTES);
	dma->residue_granularity = DMA_RESIDUE_GRANULARITY_DESCRIPTOR;
	dma->chancnt = cnt;

	/* Set DMA channel callbacks */
	dma->dev = chip->dev;
	dma->device_alloc_chan_resources = dw_edma_alloc_chan_resources;
	dma->device_free_chan_resources = dw_edma_free_chan_resources;
	dma->device_config = dw_edma_device_config;
	dma->device_pause = dw_edma_device_pause;
	dma->device_resume = dw_edma_device_resume;
	dma->device_terminate_all = dw_edma_device_terminate_all;
	dma->device_issue_pending = dw_edma_device_issue_pending;
	dma->device_tx_status = dw_edma_device_tx_status;
	dma->device_prep_slave_sg = dw_edma_device_prep_slave_sg;
	dma->device_prep_dma_cyclic = dw_edma_device_prep_dma_cyclic;
	dma->device_prep_interleaved_dma = dw_edma_device_prep_interleaved_dma;

	dma_set_max_seg_size(dma->dev, U32_MAX);

	/* Register DMA device */
	err = dma_async_device_register(dma);

	dbg_init("%s Channel ID ::: %d\n",
		write ? "DMA_READ" : "DMA_WRITE",
		chan->vc.chan.device->dev_id);
	if (write) {
		dw->rd_dma_id = chan->vc.chan.device->dev_id;
		memset(dw->rd_dma_chan, 0x00, sizeof(dw->rd_dma_chan));
	} else {
		dw->wr_dma_id = chan->vc.chan.device->dev_id;
		memset(dw->wr_dma_chan, 0x00, sizeof(dw->wr_dma_chan));
	}

	return err;
}

static inline void dw_edma_dec_irq_alloc(int *nr_irqs, u32 *alloc, u16 cnt)
{
	if (*nr_irqs && *alloc < cnt) {
		(*alloc)++;
		(*nr_irqs)--;
	}
}

static inline void dw_edma_add_irq_mask(u32 *mask, u32 alloc, u16 cnt)
{
	while (*mask * alloc < cnt)
		(*mask)++;
}

/*IRQ : DMA IRQ(dma_irq_cnt) + USER IRQ*/
static inline void dw_set_user_irq(struct dw_edma *dw, u8 dma_irq_cnt)
{
	struct dx_dma_user_irq *user_irq;
	int i;

	for(i = 0; i < USER_IRQ_NUMS; i++) {
		if (user_irq_vec_table[i].handler) {
			user_irq = &(dw->irq[dma_irq_cnt + i].user_irq);
			user_irq->handler = user_irq_vec_table[i].handler;
			snprintf(user_irq->name, sizeof(user_irq->name), 
				"%s_%s", dw->name, user_irq_vec_table[i].name);
		} else {
			break;
		}
	}
}

static int dw_edma_irq_request(struct dw_edma_chip *chip,
			       u32 *wr_alloc, u32 *rd_alloc)
{
	struct device *dev = chip->dev;
	struct dw_edma *dw = chip->dw;
	u32 wr_mask = 1;
	u32 rd_mask = 1;
	int i, err = 0;
	u32 ch_cnt;
	int irq;

	ch_cnt = dw->wr_ch_cnt + dw->rd_ch_cnt;

	if (dw->nr_irqs < 1)
		return -EINVAL;

	if (dw->nr_irqs == 1) {
		/* Common IRQ shared among all channels */
		irq = dw->ops->irq_vector(dev, 0);
		err = request_irq(irq, dw_edma_single_msi_interrupt,
			  IRQF_SHARED, dw->name, &dw->irq[0]);
		if (err) {
			dw->nr_irqs = 0;
			return err;
		}

		/* Read current MSI from PCI config space - use OS-assigned value as-is */
		if (dx_pci_read_msi_msg(dw->pdev, &dw->irq[0].msi) != 0) {
			pr_warn("dx_dma: Failed to read MSI from PCI config\n");
			if (irq_get_msi_desc(irq))
				get_cached_msi_msg(irq, &dw->irq[0].msi);
		}
		pr_info("dx_dma: Single MSI mode - addr=0x%x_%x, data=0x%x (IRQ %d)\n",
			dw->irq[0].msi.address_hi, dw->irq[0].msi.address_lo,
			dw->irq[0].msi.data, irq);
	} else {
		/* Distribute IRQs equally among all channels */
		int tmp = dw->nr_irqs;

		dbg_irq("dw->nr_irqs:%d, wr_alloc:%d, rd_alloc:%d, wr_mask:%d, rd_mask:%d\n",
			dw->nr_irqs, *wr_alloc, *rd_alloc, wr_mask, rd_mask);
		while (tmp && (*wr_alloc + *rd_alloc) < ch_cnt) {
			dw_edma_dec_irq_alloc(&tmp, wr_alloc, dw->wr_ch_cnt);
			dw_edma_dec_irq_alloc(&tmp, rd_alloc, dw->rd_ch_cnt);
		}

		dw_edma_add_irq_mask(&wr_mask, *wr_alloc, dw->wr_ch_cnt);
		dw_edma_add_irq_mask(&rd_mask, *rd_alloc, dw->rd_ch_cnt);
		dbg_irq("dw->nr_irqs:%d, wr_alloc:%d, rd_alloc:%d, wr_mask:%d, rd_mask:%d\n",
			dw->nr_irqs, *wr_alloc, *rd_alloc, wr_mask, rd_mask);

		dw_set_user_irq(dw, dw->dma_irqs);

		dbg_irq("Register for split interrupt (Write/Read:%d, NPU:%d)\n",
			*wr_alloc, USER_NUM_MAX);

		/* Register - dma irq */
		/* Common IRQ shared among all channels */
		/* TODO - need to be verfied for multi DMA interrupt */
		for (i = 0; i < dw->dma_irqs; i ++) {
			snprintf(dw->irq_name[i], sizeof(dw->name), "%s", dw->name);
			irq = dw->ops->irq_vector(dev, i);
			err = request_irq(irq, dw_edma_interrupt_common,
					IRQF_SHARED, dw->irq_name[i], &dw->irq[i]);
			if (err) {
				dw->nr_irqs = i;
				return err;
			}
			if (irq_get_msi_desc(irq)) {
				u16 msi_data;
				get_cached_msi_msg(irq, &dw->irq[i].msi);
				msi_data = dx_pci_read_msi_data(dw->pdev);
				if (dw->irq[i].msi.data != msi_data) {
					pr_info("Msi data is replaced(%d, cached:%d)\n", msi_data, dw->irq[i].msi.data);
					dw->irq[i].msi.data = msi_data;
				}
			}
		}
		/* Register - user irq */
		for (i = dw->dma_irqs; i < USER_IRQ_NUMS; i++) {
			if (dw->irq[i].user_irq.handler != NULL) {
				irq = dw->ops->irq_vector(dev, i);
				err = request_irq(irq,
						dw->irq[i].user_irq.handler,
						IRQF_SHARED,
						dw->irq[i].user_irq.name,
						&dw->irq[i]);
				if (err) {
					dw->nr_irqs = i;
					return err;
				}
				if (irq_get_msi_desc(irq)) {
					u16 msi_data;
					get_cached_msi_msg(irq, &dw->irq[i].msi);
					msi_data = dx_pci_read_msi_data(dw->pdev);
					if (dw->irq[i].msi.data != msi_data) {
						pr_info("Msi data is replaced(%d, cached:%d)\n", msi_data, dw->irq[i].msi.data);
						dw->irq[i].msi.data = msi_data;
																				}
				}
			} else {
				break;
			}
		}

		dw->nr_irqs = i;
	}
	dbg_irq("Registered Interrupt Handelr : %d\n", dw->nr_irqs);

	return err;
}

int dx_dma_probe(struct dw_edma_chip *chip)
{
	struct device *dev;
	struct dw_edma *dw;
	u32 wr_alloc = 0;
	u32 rd_alloc = 0;
	int i, err;

	if (!chip) {
		dev_err(dev, "Chip pointer error!(%p)\n", chip);
		return -EINVAL;
	}

	dev = chip->dev;
	if (!dev) {
		dev_err(dev, "Device pointer error!(%p)\n", dev);
		return -EINVAL;
	}

	dw = chip->dw;
	if (!dw || !dw->irq || !dw->ops || !dw->ops->irq_vector) {
		dev_err(dev, "dw setting errors!(dw:%p,irq:%p,ops:%p,vec:%p)\n",
			dw, dw->irq, dw->ops, dw->ops->irq_vector);
		return -EINVAL;
	}

	raw_spin_lock_init(&dw->lock);
	spin_lock_init(&dw->engine_reset_lock);
	for(i = 0; i < EDMA_MAX_WR_CH; i++) {
		spin_lock_init(&dw->wr_dma_chan_locks[i].ch_lock);
		dw->wr_dma_chan_locks[i].ch_in_use = false;
	}
	for(i = 0; i < EDMA_MAX_RD_CH; i++) {
		spin_lock_init(&dw->rd_dma_chan_locks[i].ch_lock);
		dw->rd_dma_chan_locks[i].ch_in_use = false;
	}

	/* Set iATU */
	dw->pdev = to_pci_dev(dev);
	dw_iatu_default_config_set(dw);

	dw->wr_ch_cnt = min_t(u16, dw->wr_ch_cnt,
			      dw_edma_v0_core_ch_count(dw, EDMA_DIR_WRITE));
	dw->wr_ch_cnt = min_t(u16, dw->wr_ch_cnt, EDMA_MAX_WR_CH);

	dw->rd_ch_cnt = min_t(u16, dw->rd_ch_cnt,
			      dw_edma_v0_core_ch_count(dw, EDMA_DIR_READ));
	dw->rd_ch_cnt = min_t(u16, dw->rd_ch_cnt, EDMA_MAX_RD_CH);

	if (!dw->wr_ch_cnt && !dw->rd_ch_cnt) {
		dev_err(dev, "Get error for dma channel(wr:%d,rd:%d)\n",
			dw->wr_ch_cnt, dw->rd_ch_cnt);
		return -EINVAL;
	}

	dev_vdbg(dev, "Channels:\twrite=%d, read=%d\n",
		 dw->wr_ch_cnt, dw->rd_ch_cnt);

	/* Allocate channels */
	dw->chan = devm_kcalloc(dev, dw->wr_ch_cnt + dw->rd_ch_cnt,
				sizeof(*dw->chan), GFP_KERNEL);
	if (!dw->chan)
		return -ENOMEM;

	/* Initialize Global Memory Pools */
	if (dw_edma_mem_init(dw))
		return -ENOMEM;

	// snprintf(dw->name, sizeof(dw->name), "dx-dma_%d", chip->id);
	snprintf(dw->name, sizeof(dw->name), "dx-dma_%d", dw->idx);

	/* Disable eDMA, only to establish the ideal initial conditions */
	dw_edma_v0_core_off(dw);

	/* Request IRQs */
	err = dw_edma_irq_request(chip, &wr_alloc, &rd_alloc);
	if (err)
		return err;

	/* Setup write channels */
	err = dw_edma_channel_setup(chip, true, wr_alloc, rd_alloc);
	if (err)
		goto err_irq_free;

	/* Setup read channels */
	err = dw_edma_channel_setup(chip, false, wr_alloc, rd_alloc);
	if (err)
		goto err_irq_free;

	/* Power management */
	if (!pm_runtime_enabled(dev)) {
		pm_runtime_enable(dev);
		dw->pm_runtime_managed = true;
	} else {
		dw->pm_runtime_managed = false;
	}
	dw_edma_v0_core_engine_enable(dw->chan);

	/* Set up data user IRQ data strutures */
	dx_user_irq_init(dw);

#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
	if (dx_pcie_message_init(dw->idx)) {
		dev_err(dev, "message init error\n");
	}
#endif
	/* Initialize software-only interrupt block */
	dx_sw_intr_init(dw);

	/* Turn debugfs on */
	dw_edma_v0_core_debugfs_on(chip);

	return 0;

err_irq_free:
	for (i = (dw->nr_irqs - 1); i >= 0; i--) {
		int irq = dw->ops->irq_vector(dev, i);
		free_irq(irq, &dw->irq[i]);
	}

	dw->nr_irqs = 0;

	return err;
}


int dx_dma_remove(struct dw_edma_chip *chip)
{
	struct dw_edma_chan *chan, *_chan;
	struct device *dev = chip->dev;
	struct dw_edma *dw = chip->dw;
	int i;

	/* Disable eDMA */
	dw_edma_v0_core_off(dw);
	
	/* Free Global Memory Pools */
	dw_edma_mem_deinit(dw);

	/* Free IRQs */
	for (i = (dw->nr_irqs - 1); i >= 0; i--) {
		int irq = dw->ops->irq_vector(dev, i);
		free_irq(irq, &dw->irq[i]);
	}

	/* Power management */
	if (dw->pm_runtime_managed)
		pm_runtime_disable(dev);
	dw_edma_v0_core_engine_disable(dw->chan);

	/* Deregister eDMA device */
	list_for_each_entry_safe(chan, _chan, &dw->wr_edma.channels,
				 vc.chan.device_node) {
		dw_edma_device_terminate_all(&chan->vc.chan);
		tasklet_kill(&chan->vc.task);
		device_unregister(dchan2dev(&chan->vc.chan));
		list_del(&chan->vc.chan.device_node);
	}
	dma_async_device_unregister(&dw->wr_edma);

	list_for_each_entry_safe(chan, _chan, &dw->rd_edma.channels,
				 vc.chan.device_node) {
		dw_edma_device_terminate_all(&chan->vc.chan);
		tasklet_kill(&chan->vc.task);
		device_unregister(dchan2dev(&chan->vc.chan));
		list_del(&chan->vc.chan.device_node);
	}
	dma_async_device_unregister(&dw->rd_edma);

	/* Turn debugfs off */
	dw_edma_v0_core_debugfs_off(chip);

	return 0;
}
