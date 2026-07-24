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
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#include <linux/cpumask.h>

#include "dx_mmio_compat.h"
#include "dw-edma-core.h"
#include "dw-edma-v0-core.h"
#include "dx_link_health.h"
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

#define DW_EDMA_PREP_BURST_BATCH_SIZE	64
#define DW_EDMA_LAZY_REFILL_WAIT_MS	3000

/*
 * Diagnostic toggle for shadow-WQ pre-build path (commit ab06eef).
 * shadow_wq=0 bypasses the shadow pre-build and forces the classic
 * multi-chunk SG path.  Used to bisect multi-channel regressions.
 * Default: 1 (matches current behaviour).
 */
static int shadow_wq = 1;
module_param(shadow_wq, int, 0644);
MODULE_PARM_DESC(shadow_wq,
	"Shadow WQ pre-build for multi-chunk SG (1=on, 0=off; diagnostic)");

/*
 * Shadow WQ adds workqueue/launch handoff overhead on moderate SG lists.
 * Keep the classic path for normal multi-channel baseline sizes and reserve
 * shadow pre-build for very large transfers where chunk overlap can pay off.
 * 16K 4KB-SG entries is 64MiB; 0 disables this size gate.
 */
static uint shadow_wq_min_sg = 16384;
module_param(shadow_wq_min_sg, uint, 0644);
MODULE_PARM_DESC(shadow_wq_min_sg,
	"Minimum SG entries for shadow WQ pre-build (0=always when shadow_wq=1)");

/* ---------------- User IRQ vector table (placed early for dx_sw_intr_init) --- */

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
#ifdef DX_SW_IRQ_DEBUG
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
			if (!(dw->sw_active_mask & BIT(i)))
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
#else
static inline void dx_sw_irq_dump_state(struct dw_edma *dw, const char *context)
{
	(void)dw;
	(void)context;
}
#endif

/* Latch EP set bits into raw status (do NOT clear set_off here; per-bit clear after handling) */
static u32 dx_sw_irq_latch_and_clear(struct dw_edma *dw, struct dx_sw_irq_block __iomem *blk)
{
	/* Aggregate per-event set_off[i] into bitfield */
	int i;
	u32 set_bits = 0;
	for (i = 0; i < 16; i++) {
		if (!(dw->sw_active_mask & BIT(i)))
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
		if (!(dw->sw_active_mask & BIT(i)))
			continue;
		if (!(pending_mask & BIT(i)))
			continue;

		if (!dw->user_irq_vec_table || !dw->user_irq_vec_table[i].handler)
			break;

		event_id = dw->user_irq_vec_table[i].event_id;
		if (event_id < dw->event_irq_idx)
			user_irq_service(irq, &dw_irq->user_irqs[event_id]);
		else
			user_irq_events(dw_irq, &dw_irq->user_irqs[event_id]);
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
		if (!(dw->sw_active_mask & BIT(i)))
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
	set_reg = dx_sw_irq_latch_and_clear(dw, blk);
	set_count = hweight32(set_reg);  /* Count number of set bits from EP */

	if (set_reg) {
		dbg_irq("Latched set bits=0x%x (EP_generated=%u)\n", set_reg, set_count);
	}
	(void)set_count;

	/* Get pending interrupts */
	pending_mask = dx_sw_irq_get_pending(blk, enable_mask);
	pending_count = hweight32(pending_mask);  /* Count number of pending bits to process */

	if (!pending_mask) {
		dx_sw_irq_dump_state(dw, "IRQ_NO_PENDING");
		return IRQ_NONE;
	}

	/* Log multiple interrupt coalescing case */
	if (pending_count > 1) {
		dbg_irq("COALESCED: Processing %u events (pending=0x%x)\n", pending_count, pending_mask);
	}

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
			if (!(dw->sw_active_mask & BIT(i)))
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
	if (!dw->user_irq_vec_table) {
		set_user_irq_vec_table(dw);
		if (!dw->user_irq_vec_table) {
			pr_err("SWIRQ[%s] user_irq_vec_table setup failed\n", dw->name);
			return;
		}
	}

	/* Build enable mask from valid handlers */
	for (i = 0; i < USER_IRQ_NUMS; i++) {
		if (!dw->user_irq_vec_table[i].handler)
			break;
		enable |= dw->user_irq_vec_table[i].bit;
		active++;
	}

	/* Must have valid handlers for proper operation */
	if (enable == 0) {
		pr_err("SWIRQ[%s] no valid handlers found - INITIALIZATION FAILED\n", dw->name);
		return;
	}

	/* Initialize the shared interrupt block */
	memset_io(dx_sw_irq(dw), 0, sizeof(struct dx_sw_irq_block));
	iowrite32(enable, &dx_sw_irq(dw)->enable);
	dw->sw_active_mask = enable;
	dw->sw_active_count = active;
	pr_debug("SWIRQ[%s] active events=%d mask=0x%x\n", dw->name, dw->sw_active_count, dw->sw_active_mask);
}


static inline int get_irq_to_dma_num(struct dw_edma *dw, int irq_n)
{
	int i, dma_n = -1;
	for (i = 0; i < USER_IRQ_NUMS; i++) {
		if (!dw->user_irq_vec_table[i].handler)
			break;
		if (dw->user_irq_vec_table[i].irq_pos == irq_n) {
			dma_n = dw->user_irq_vec_table[i].dma_ch_n;
			break;
		}
	}
	return dma_n;
}

void set_user_irq_vec_table(struct dw_edma *dw)
{
	if (dw->dx_ver == 3)
		dw->user_irq_vec_table = user_irq_vec_table_v3;
	else
		pr_err("Undefined version error(%d)\n", dw->dx_ver);
}

int get_nr_user_irqs(struct dw_edma *dw)
{
	int i, nr_user_irqs = 0;
	for (i = 0; i < USER_IRQ_NUMS; i++) {
		if (dw->user_irq_vec_table) {
			if (dw->user_irq_vec_table[i].handler)
				nr_user_irqs++;
		}
	}
	return nr_user_irqs;
}

int get_pos_user_irqs(struct dw_edma *dw, int event_id)
{
	int pos = -1, i;
	for(i = 0; i < USER_IRQ_NUMS; i++) {
		if (dw->user_irq_vec_table[i].handler) {
			if (dw->user_irq_vec_table[i].event_id == event_id) {
				pos = dw->user_irq_vec_table[i].irq_pos;
				break;
			}
		}
	}
	return pos;
}

bool check_event_id(struct dw_edma *dw, int event_id)
{
	bool match = false;
	int i;
	for(i = 0; i < USER_IRQ_NUMS; i++) {
		if (dw->user_irq_vec_table[i].handler) {
			if (dw->user_irq_vec_table[i].event_id == event_id) {
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
			if (dw->user_irq_vec_table[i].handler) {
				event_id = dw->user_irq_vec_table[i].event_id;
				strncpy(dw->irq[0].user_irqs[event_id].name,
					dw->user_irq_vec_table[i].name,
					sizeof(dw->user_irq_vec_table[i].name));
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
static void dw_edma_unregister_dma_device(struct dma_device *dma)
{
	dma_async_device_unregister(dma);
}

static inline
struct device *chan2dev(struct dw_edma_chan *chan)
{
	return &chan->vc.chan.dev->device;
}





/* Deferred desc free — process context (cancel_work_sync may sleep) */
static void dw_edma_deferred_free_desc_work(struct work_struct *work)
{
	struct dw_edma_desc *desc = container_of(work, struct dw_edma_desc,
						 deferred_free_work);
	dw_edma_free_desc(desc);
}

static void vchan_free_desc(struct virt_dma_desc *vdesc)
{
	struct dw_edma_desc *desc = vd2dw_edma_desc(vdesc);
	struct dw_edma *dw = desc->chan->chip->dw;

	if (in_interrupt() || in_atomic()) {
		/* Defer: free_desc synchronizes descriptor work items. */
		INIT_WORK(&desc->deferred_free_work,
			  dw_edma_deferred_free_desc_work);
		if (dw->shadow_wq)
			queue_work(dw->shadow_wq, &desc->deferred_free_work);
		else
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

static void dw_edma_start_transfer(struct dw_edma_chan *chan);

static struct dw_edma_burst *dw_edma_last_data_burst(struct dw_edma_chunk *chunk)
{
	if (!chunk->burst || list_empty(&chunk->burst->list))
		return NULL;

	return list_last_entry(&chunk->burst->list,
			       struct dw_edma_burst, list);
}

static bool dw_edma_can_merge_sg_burst(struct dw_edma_chan *chan,
				       struct dw_edma_burst *last,
				       dma_addr_t host_addr, u64 dev_addr,
				       u32 len)
{
	u32 merged_sz;

	if (!last || !len || len > U32_MAX - last->sz)
		return false;

	merged_sz = last->sz + len;
	if (last->sar > U64_MAX - merged_sz ||
	    last->dar > U64_MAX - merged_sz)
		return false;

	if (chan->dir == EDMA_DIR_WRITE)
		return last->dar + last->sz == host_addr &&
		       last->sar + last->sz == dev_addr;

	return last->sar + last->sz == host_addr &&
	       last->dar + last->sz == dev_addr;
}

static void dw_edma_extend_sg_burst(struct dw_edma_chunk *chunk,
				    struct dw_edma_burst *burst,
				    u32 len, u32 *alloc_accum)
{
	burst->sz += len;
	chunk->ll_region.sz += len;
	if (alloc_accum)
		*alloc_accum += len;
}

static void dw_edma_init_sg_burst(struct dw_edma_desc *desc,
				  struct dw_edma_chunk *chunk,
				  struct dw_edma_burst *burst,
				  dma_addr_t host_addr, u64 dev_addr,
				  u32 len, u32 *alloc_accum,
				  bool already_linked)
{
	if (!already_linked) {
		INIT_LIST_HEAD(&burst->list);
		list_add_tail(&burst->list, &chunk->burst->list);
		chunk->bursts_alloc++;
	}

	burst->sz = len;
	chunk->ll_region.sz += len;
	if (alloc_accum)
		*alloc_accum += len;

	if (desc->chan->dir == EDMA_DIR_WRITE) {
		burst->sar = dev_addr;
		burst->dar = host_addr;
	} else {
		burst->dar = dev_addr;
		burst->sar = host_addr;
	}
}

static int dw_edma_fill_sg_bursts(struct dw_edma_desc *desc,
				  struct dw_edma_chunk *chunk,
				  struct scatterlist **sgp,
				  u32 *remainingp, u64 *addrp,
				  u32 *alloc_accum,
				  struct dw_edma_burst **batch,
				  u32 batch_max)
{
	struct dw_edma_chan *chan = desc->chan;
	struct scatterlist *sg = *sgp;
	u32 remaining = *remainingp;
	u64 addr = *addrp;
	u32 capacity, need, allocated, used = 0, consumed = 0;
	struct dw_edma_burst *burst, *last;
	dma_addr_t host_addr;
	int ret = 0;
	u32 len;

	if (!sg || !remaining || chunk->bursts_alloc >= chan->ll_max ||
	    !batch || !batch_max)
		return 0;

	capacity = chan->ll_max - chunk->bursts_alloc;
	need = min3(remaining, capacity, batch_max);
	allocated = dw_edma_alloc_burst_batch(chunk, need, batch);

	while (sg && consumed < need && remaining > 0) {
		len = sg_dma_len(sg);
		host_addr = sg_dma_address(sg);
		if (addr > U64_MAX - len ||
		    (u64)host_addr > U64_MAX - len) {
			ret = -EINVAL;
			break;
		}
		last = dw_edma_last_data_burst(chunk);

		if (dw_edma_can_merge_sg_burst(chan, last, host_addr, addr, len)) {
			dw_edma_extend_sg_burst(chunk, last, len, alloc_accum);
		} else {
			bool already_linked = false;

			if (used < allocated) {
				burst = batch[used++];
			} else {
				burst = dw_edma_alloc_burst(chunk);
				if (!burst)
					break;
				already_linked = true;
			}

			dw_edma_init_sg_burst(desc, chunk, burst, host_addr,
					       addr, len, alloc_accum,
					       already_linked);
		}

		addr += len;
		sg = sg_next(sg);
		remaining--;
		consumed++;
	}

	if (allocated > used)
		dw_edma_free_burst_batch(chan, &batch[used], allocated - used);

	if (ret)
		return ret;
	if (!sg && remaining)
		return -EINVAL;

	*sgp = sg;
	*remainingp = remaining;
	*addrp = addr;

	return consumed;
}

/*
 * dw_edma_shadow_fill_chunk - Populate a shadow chunk's burst list from SG.
 *
 * Reads from shadow_next_sg / shadow_next_remaining / shadow_next_addr
 * stored in @desc, fills bursts into @chunk using batch alloc from
 * @batch scratch array.
 *
 * Updates desc->shadow_next_* and desc->shadow_has_more.
 * Returns number of data bursts filled (children in list), 0 on error.
 *
 * Context: process (workqueue). Pool + dynamic alloc both safe.
 */
static int dw_edma_shadow_fill_chunk(struct dw_edma_desc *desc,
				     struct dw_edma_chunk *chunk,
				     struct dw_edma_burst **batch)
{
	struct dw_edma_chan *chan = desc->chan;
	struct scatterlist *sg = desc->shadow_next_sg;
	u64 addr = desc->shadow_next_addr;
	u32 remaining = desc->shadow_next_remaining;
	int filled;
	struct dw_edma_burst *burst;

	if (!sg || !remaining)
		return 0;

	/*
	 * Allocate HEAD burst as empty sentinel (list head).
	 * write_chunk iterates chunk->burst->list for LLI generation
	 * but NEVER processes the HEAD burst itself — only children
	 * in the list get LLI entries. So data must go into children.
	 */
	burst = dw_edma_alloc_burst(chunk);
	if (!burst)
		return 0;
	/* HEAD burst: no SG data, just serves as list anchor */

	filled = dw_edma_fill_sg_bursts(desc, chunk, &sg, &remaining,
					 &addr, &desc->shadow_alloc_sz,
					 batch, chan->ll_max);

	if (unlikely(filled <= 0)) {
		dev_err(chan->chip->dev,
			"[SHADOW] fill failed: no bursts populated (remaining=%u, HEAD allocated but unusable)\n",
			remaining);
		return 0;
	}

	desc->shadow_next_sg = sg;
	desc->shadow_next_remaining = remaining;
	desc->shadow_next_addr = addr;
	desc->shadow_has_more = (remaining > 0);

	return filled;
}

static void dw_edma_reset_chunk_region(struct dw_edma_chunk *chunk)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;

	chunk->burst = NULL;
	chunk->bursts_alloc = 0;
	if (chan->dir == EDMA_DIR_WRITE) {
		chunk->ll_region.paddr = dw->ll_region_wr[chan->id].paddr;
		chunk->ll_region.vaddr = dw->ll_region_wr[chan->id].vaddr;
		chunk->ll_region.sz = dw->ll_region_wr[chan->id].sz;
	} else {
		chunk->ll_region.paddr = dw->ll_region_rd[chan->id].paddr;
		chunk->ll_region.vaddr = dw->ll_region_rd[chan->id].vaddr;
		chunk->ll_region.sz = dw->ll_region_rd[chan->id].sz;
	}
}

struct dw_edma_lazy_refill_plan {
	struct scatterlist	*sg;
	u32			remaining;
	u64			addr;
	u32			alloc_sz;
};

/* Refill the completed active chunk from saved SG state.
 *
 * This fixed-buffer path is used for SG transfers that exceed one descriptor
 * table but do not use shadow WQ.  The reusable chunk is detached from the
 * descriptor while this runs, so burst allocation and descriptor generation can
 * happen in process context without holding vc.lock.  The descriptor cursor is
 * returned in @plan and committed only immediately before the data doorbell. */
static int dw_edma_lazy_refill_chunk_prepare(struct dw_edma_desc *desc,
					     struct dw_edma_chunk *chunk,
					     struct dw_edma_lazy_refill_plan *plan)
{
	struct dw_edma_chan *chan = desc->chan;
	struct scatterlist *sg = desc->sg_cur;
	u64 addr = desc->addr_accum;
	u32 remaining = desc->sg_remaining;
	u32 alloc_sz = desc->alloc_sz;
	int filled;
	struct dw_edma_burst *burst;

	if (!sg || !remaining || !plan)
		return 0;

	dw_edma_reset_chunk_region(chunk);
	chunk->cb = !chunk->cb;

	burst = dw_edma_alloc_burst(chunk);
	if (!burst)
		return 0;

	filled = dw_edma_fill_sg_bursts(desc, chunk, &sg, &remaining,
					 &addr, &alloc_sz,
					 chan->burst_batch, chan->ll_max);

	if (unlikely(filled <= 0)) {
		dw_edma_free_burst(chunk);
		return 0;
	}

	plan->sg = sg;
	plan->remaining = remaining;
	plan->addr = addr;
	plan->alloc_sz = alloc_sz;

	return filled;
}

static int dw_edma_wait_stopped_for_lazy_refill(struct dw_edma_chan *chan)
{
	struct device *dev = chan->chip->dev;
	unsigned long deadline = jiffies +
		msecs_to_jiffies(DW_EDMA_LAZY_REFILL_WAIT_MS);
	bool delayed = false;
	u32 cs;
	int ret;

	for (;;) {
		if (READ_ONCE(chan->hw_err) || READ_ONCE(chan->aborted))
			return -EIO;

		ret = dw_edma_v0_core_ch_status_checked(chan, &cs);
		if (ret)
			return ret;
		if (cs == 0 || cs == DMA_STOP)
			return 0;
		if (cs == DMA_ERR)
			return -EIO;

		if (!delayed) {
			dev_warn_ratelimited(dev,
				"[LAZY] ch%d Done observed before CS stopped (CS=%u), deferring refill\n",
				chan->id, cs);
			delayed = true;
		}
		if (time_after_eq(jiffies, deadline))
			return -ETIMEDOUT;

		usleep_range(100, 200);
	}
}

static void dw_edma_lazy_refill_work(struct work_struct *work)
{
	struct dw_edma_desc *desc = container_of(work, struct dw_edma_desc,
						 lazy_work);
	struct dw_edma_chan *chan = desc->chan;
	struct dw_edma_lazy_refill_plan plan = {0};
	struct virt_dma_desc *vd;
	struct dw_edma_chunk *child;
	unsigned long flags;
	int ret;
	bool wake_transfer = false;
	bool free_unlinked = false;

	ret = dw_edma_wait_stopped_for_lazy_refill(chan);

	spin_lock_irqsave(&chan->vc.lock, flags);
	if (desc->lazy_refill_state != LAZY_REFILL_PENDING) {
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		return;
	}
	desc->lazy_refill_state = LAZY_REFILL_IDLE;

	vd = vchan_next_desc(&chan->vc);
	if (!vd || vd2dw_edma_desc(vd) != desc) {
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		return;
	}

	child = list_first_entry_or_null(&desc->chunk->list,
					 struct dw_edma_chunk, list);
	if (ret || !child || !READ_ONCE(desc->lazy_mode)) {
		WRITE_ONCE(desc->lazy_mode, false);
		WRITE_ONCE(chan->hw_err, true);
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		if (READ_ONCE(chan->transfer_wq))
			wake_up(READ_ONCE(chan->transfer_wq));
		return;
	}

	if (chan->request == EDMA_REQ_STOP) {
		WRITE_ONCE(desc->lazy_mode, false);
		dw_edma_free_burst(child);
		list_del(&child->list);
		desc->chunks_alloc--;
		dw_edma_defer_chunk_free(chan, child);
		list_del(&vd->node);
		vchan_cookie_complete(vd);
		chan->request = EDMA_REQ_NONE;
		chan->status = EDMA_ST_IDLE;
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		return;
	}

	if (chan->request != EDMA_REQ_NONE || !desc->sg_remaining) {
		WRITE_ONCE(desc->lazy_mode, false);
		WRITE_ONCE(chan->hw_err, true);
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		if (READ_ONCE(chan->transfer_wq))
			wake_up(READ_ONCE(chan->transfer_wq));
		return;
	}

	/* Claim the reusable fixed-buffer chunk and detach it while the worker
	 * rebuilds bursts, writes LLI host memory, syncs caches, and helper-copies
	 * into device LL SRAM.  No other path may mutate this chunk while it is in
	 * LAZY_REFILL_PREPARING; timeout/recovery code treats that state as active. */
	list_del_init(&child->list);
	desc->chunks_alloc--;
	desc->lazy_refill_state = LAZY_REFILL_PREPARING;
	spin_unlock_irqrestore(&chan->vc.lock, flags);

	dw_edma_free_burst(child);
	if (!dw_edma_lazy_refill_chunk_prepare(desc, child, &plan)) {
		ret = -ENOMEM;
	} else {
		ret = dw_edma_v0_core_prepare_start(child, false, chan->is_llm);
	}

	spin_lock_irqsave(&chan->vc.lock, flags);
	vd = vchan_next_desc(&chan->vc);
	if (!vd || vd2dw_edma_desc(vd) != desc) {
		desc->lazy_refill_state = LAZY_REFILL_IDLE;
		free_unlinked = true;
		goto out_unlock;
	}

	if (desc->lazy_refill_state != LAZY_REFILL_PREPARING) {
		free_unlinked = true;
		goto out_unlock;
	}

	if (ret) {
		dw_edma_v0_core_cancel_prepared(child, chan->is_llm);
		WRITE_ONCE(desc->lazy_mode, false);
		WRITE_ONCE(chan->hw_err, true);
		desc->lazy_refill_state = LAZY_REFILL_IDLE;
		dw_edma_defer_chunk_free(chan, child);
		wake_transfer = true;
		goto out_unlock;
	}

	if (chan->request == EDMA_REQ_STOP) {
		dw_edma_v0_core_cancel_prepared(child, chan->is_llm);
		WRITE_ONCE(desc->lazy_mode, false);
		desc->lazy_refill_state = LAZY_REFILL_IDLE;
		dw_edma_defer_chunk_free(chan, child);
		list_del(&vd->node);
		vchan_cookie_complete(vd);
		chan->request = EDMA_REQ_NONE;
		chan->status = EDMA_ST_IDLE;
		goto out_unlock;
	}

	if (chan->request != EDMA_REQ_NONE || !READ_ONCE(desc->lazy_mode) ||
	    READ_ONCE(chan->aborted) || READ_ONCE(chan->hw_err)) {
		dw_edma_v0_core_cancel_prepared(child, chan->is_llm);
		desc->lazy_refill_state = LAZY_REFILL_IDLE;
		dw_edma_defer_chunk_free(chan, child);
		goto out_unlock;
	}

	desc->sg_cur = plan.sg;
	desc->sg_remaining = plan.remaining;
	desc->addr_accum = plan.addr;
	desc->alloc_sz = plan.alloc_sz;
	if (!plan.remaining)
		WRITE_ONCE(desc->lazy_mode, false);

	list_add(&child->list, &desc->chunk->list);
	desc->chunks_alloc++;
	desc->lazy_refill_state = LAZY_REFILL_IDLE;
	chan->status = EDMA_ST_BUSY;
	dw_edma_v0_core_launch_prepared(child, chan->is_llm);

out_unlock:
	spin_unlock_irqrestore(&chan->vc.lock, flags);

	if (free_unlinked) {
		if (!ret)
			dw_edma_v0_core_cancel_prepared(child, chan->is_llm);
		dw_edma_free_unlinked_chunk(chan, child);
	}
	if (wake_transfer && READ_ONCE(chan->transfer_wq))
		wake_up(READ_ONCE(chan->transfer_wq));
}

static void dw_edma_lazy_error_locked(struct dw_edma_chan *chan,
					      struct dw_edma_desc *desc,
					      bool *wake_transfer)
{
	WRITE_ONCE(desc->lazy_mode, false);
	WRITE_ONCE(chan->hw_err, true);
	*wake_transfer = true;
}

/*
 * Keep the real Done IRQ path lightweight for lazy transfers.  The ISR only
 * records the safe handoff point and queues the relevant worker; descriptor
 * refill, helper-copy, CS waiting, and next doorbell are process-context work.
 *
 * Returns true when the lazy path consumed this Done event and the caller must
 * skip standard chunk free/completion.  Returns false to continue with the
 * normal completion path.
 */
static bool dw_edma_defer_lazy_done_locked(struct dw_edma_chan *chan,
						   struct dw_edma_desc *desc,
						   bool *wake_transfer)
{
	struct dw_edma *dw = chan->chip->dw;

	if (!READ_ONCE(desc->lazy_mode))
		return false;

	/* Stop must complete the current descriptor instead of launching more. */
	if (chan->request == EDMA_REQ_STOP) {
		if (desc->shadow_state == SHADOW_BUILDING ||
		    desc->shadow_state == SHADOW_ISR_PENDING ||
		    desc->shadow_state == SHADOW_READY)
			desc->shadow_state = SHADOW_CANCELLED;
		WRITE_ONCE(desc->lazy_mode, false);
		return false;
	}

	switch (desc->shadow_state) {
	case SHADOW_READY:
	case SHADOW_BUILDING:
		/* Shadow WQ either already built the next host LLI or is still
		 * building it.  In both cases Done only means "safe to copy/launch".
		 */
		desc->shadow_state = SHADOW_ISR_PENDING;
		if (dw->shadow_wq) {
			queue_work(dw->shadow_wq, &desc->shadow_work);
			return true;
		}
		dw_edma_lazy_error_locked(chan, desc, wake_transfer);
		return true;

	case SHADOW_IDLE:
		if (desc->sg_remaining > 0) {
			/* Fixed-buffer lazy refill: the just-completed chunk owns the
			 * reusable descriptor buffer until the worker waits for CS=STOP.
			 */
			desc->lazy_refill_state = LAZY_REFILL_PENDING;
			if (desc->lazy_work_initialized && dw->shadow_wq) {
				queue_work(dw->shadow_wq, &desc->lazy_work);
				return true;
			}
			dw_edma_lazy_error_locked(chan, desc, wake_transfer);
			return true;
		}

		/* No SG remains: this is the final lazy chunk. */
		return false;

	default:
		dev_err(chan->chip->dev,
			"[SHADOW] ISR: unexpected state %d, aborting ch%d\n",
			desc->shadow_state, chan->id);
		WRITE_ONCE(desc->lazy_mode, false);
		chan->aborted = true;
		return false;
	}
}

/*
 * dw_edma_shadow_build_work - Workqueue callback: build next chunk's LLI
 * in host memory while current DMA transfer is in progress.
 *
 * State machine:
 *   BUILDING → fill bursts + write_chunk + cache flush → READY
 *                                                        (or ISR_PENDING if ISR arrived first)
 *   CANCELLED → discard and clean up
 *
 * After completing a cycle, pre-allocates the NEXT shadow chunk so the
 * post-Done WQ launch can queue the following build without allocating under
 * vc.lock.
 */
static bool dw_edma_shadow_launch_allowed(struct dw_edma_chan *chan);

static void dw_edma_shadow_build_work(struct work_struct *work)
{
	struct dw_edma_desc *desc = container_of(work, struct dw_edma_desc,
						 shadow_work);
	struct dw_edma_chan *chan = desc->chan;
	struct dw_edma_chunk *shadow;
	struct dw_edma_chunk *next_shadow = NULL;
	struct dw_edma_chunk *old_chunk;
	struct dw_edma_chunk *queued_shadow;
	unsigned long flags;
	int filled;
	int ret;
	bool isr_pending = false;
	u8 shadow_cb;

	spin_lock_irqsave(&chan->vc.lock, flags);
	shadow = desc->shadow_chunk;
	if (desc->shadow_state == SHADOW_ISR_PENDING &&
	    shadow && shadow->burst && shadow->bursts_alloc) {
		/* The chunk was already built in an earlier WQ pass.  The ISR
		 * has now confirmed that the active DMA chunk is stopped, so it
		 * is finally safe to copy this LLI into the single device LL SRAM
		 * region and launch it. */
		next_shadow = desc->shadow_next_chunk;
		desc->shadow_next_chunk = NULL;
		isr_pending = true;
	}
	spin_unlock_irqrestore(&chan->vc.lock, flags);

	if (isr_pending)
		goto precopy_and_launch;

	if (!shadow) {
		WRITE_ONCE(chan->hw_err, true);
		if (READ_ONCE(chan->transfer_wq))
			wake_up(READ_ONCE(chan->transfer_wq));
		return;
	}

	/* Fill the shadow chunk with the next SG batch */
	filled = dw_edma_shadow_fill_chunk(desc, shadow,
					   chan->shadow_burst_batch);
	if (filled == 0) {
		/* Fill failed — free shadow chunk and cancel */
		bool fatal;

		dev_err(chan->chip->dev,
			"[SHADOW] fill returned 0, freeing shadow chunk %p\n",
			shadow);
		dw_edma_free_unlinked_chunk(chan, shadow);
		spin_lock_irqsave(&chan->vc.lock, flags);
		desc->shadow_chunk = NULL;
		fatal = desc->shadow_state != SHADOW_CANCELLED &&
			!READ_ONCE(chan->aborted) &&
			chan->request == EDMA_REQ_NONE;
		if (fatal) {
			desc->lazy_mode = false;
			WRITE_ONCE(chan->hw_err, true);
		}
		desc->shadow_state = SHADOW_IDLE;
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		if (fatal && READ_ONCE(chan->transfer_wq))
			wake_up(READ_ONCE(chan->transfer_wq));
		return;
	}

	/* Write LLI to host memory + cache flush (NO BAR0, NO doorbell) */
	if (dw_edma_v0_core_prebuild_chunk(shadow)) {
		bool fatal;

		dev_err(chan->chip->dev,
			"[SHADOW] prebuild failed (bursts=%d), freeing chunk %p\n",
			shadow->bursts_alloc, shadow);
		dw_edma_free_unlinked_chunk(chan, shadow);
		spin_lock_irqsave(&chan->vc.lock, flags);
		desc->shadow_chunk = NULL;
		fatal = desc->shadow_state != SHADOW_CANCELLED &&
			!READ_ONCE(chan->aborted) &&
			chan->request == EDMA_REQ_NONE;
		if (fatal) {
			desc->lazy_mode = false;
			WRITE_ONCE(chan->hw_err, true);
		}
		desc->shadow_state = SHADOW_IDLE;
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		if (fatal && READ_ONCE(chan->transfer_wq))
			wake_up(READ_ONCE(chan->transfer_wq));
		return;
	}

	/*
	 * Pre-allocate next shadow chunk NOW (process context, safe to sleep).
	 * The post-Done WQ pass will use this pre-allocated chunk when it queues
	 * the next build, avoiding allocation while vc.lock is held.
	 */
	if (desc->shadow_has_more)
		next_shadow = dw_edma_alloc_unlinked_chunk(chan);

	/* Transition BUILDING → READY (or handle ISR_PENDING / CANCELLED) */
	spin_lock_irqsave(&chan->vc.lock, flags);
	switch (desc->shadow_state) {
	case SHADOW_BUILDING:
		desc->shadow_state = SHADOW_READY;
		/*
		 * Store the pre-allocated next shadow for the post-Done WQ pass.
		 * We pass it via a new pointer so desc->shadow_chunk stays
		 * pointing to the built chunk until the WQ consumes it.
		 */
		desc->shadow_next_chunk = next_shadow;
		next_shadow = NULL; /* ownership transferred */
		spin_unlock_irqrestore(&chan->vc.lock, flags);

		/*
		 * If the active chunk already stopped while this WQ was building and
		 * no Done MSI/status is pending, finish the BUILDING -> ISR_PENDING
		 * handoff in this same worker.  Do not call
		 * dw_edma_process_done_if_stopped() here: that path queues this same
		 * work item from inside dw_edma_done_interrupt(), which can leave the
		 * descriptor stuck in SHADOW_ISR_PENDING when queue_work() races a
		 * currently-running work item.
		 */
		if (READ_ONCE(chan->xfer_started) &&
		    dw_edma_v0_core_ch_status_raw(chan) == DMA_STOP) {
			bool handoff = false;

			dw_edma_v0_core_clear_done_int(chan);
			WRITE_ONCE(chan->xfer_started, false);

			spin_lock_irqsave(&chan->vc.lock, flags);
			if (desc->lazy_mode && desc->shadow_state == SHADOW_READY &&
			    desc->shadow_chunk == shadow) {
				old_chunk = list_first_entry_or_null(&desc->chunk->list,
							     struct dw_edma_chunk, list);
				if (old_chunk) {
					desc->xfer_sz += old_chunk->ll_region.sz;
					desc->shadow_state = SHADOW_ISR_PENDING;
					next_shadow = desc->shadow_next_chunk;
					desc->shadow_next_chunk = NULL;
					handoff = true;
				}
			}
			spin_unlock_irqrestore(&chan->vc.lock, flags);

			if (handoff) {
				isr_pending = true;
				goto precopy_and_launch;
			}
		}

		/* The Done ISR may have raced this currently-running worker after
		 * BUILDING became READY.  queue_work() cannot queue a work item that
		 * is already running, so consume ISR_PENDING here before returning. */
		spin_lock_irqsave(&chan->vc.lock, flags);
		if (desc->lazy_mode &&
		    desc->shadow_state == SHADOW_ISR_PENDING &&
		    desc->shadow_chunk == shadow) {
			next_shadow = desc->shadow_next_chunk;
			desc->shadow_next_chunk = NULL;
			isr_pending = true;
		}
		spin_unlock_irqrestore(&chan->vc.lock, flags);

		if (isr_pending)
			goto precopy_and_launch;

		return;
	case SHADOW_ISR_PENDING:
		isr_pending = true;
		break;
	case SHADOW_CANCELLED:
		desc->shadow_state = SHADOW_IDLE;
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		/* Free pre-allocated chunk since it won't be used */
		dw_edma_free_unlinked_chunk(chan, next_shadow);
		return;
	default:
		pr_err("[SHADOW] unexpected state %d in build_work\n",
		       desc->shadow_state);
		desc->shadow_state = SHADOW_IDLE;
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		dw_edma_free_unlinked_chunk(chan, next_shadow);
		return;
	}
	spin_unlock_irqrestore(&chan->vc.lock, flags);

	if (!isr_pending)
		return;

		/*
		 * Device LL SRAM is single-buffered per channel.  Copying the
		 * shadow LLI while the previous chunk is still running corrupts
		 * the active descriptor table.  Therefore pre-copy is deferred
		 * until the ISR has changed the state to SHADOW_ISR_PENDING.
		 */
	precopy_and_launch:
		ret = dw_edma_v0_core_precopy_lli(shadow, desc);
		if (ret) {
			bool cancelled = ret == -ECANCELED &&
				(READ_ONCE(desc->shadow_state) == SHADOW_CANCELLED ||
				 READ_ONCE(chan->aborted) ||
				 READ_ONCE(chan->request) != EDMA_REQ_NONE);

			if (!cancelled && ret != -EOPNOTSUPP && ret != -ENODEV)
				dev_err(chan->chip->dev,
					"[SHADOW] LLI precopy failed (%d), aborting lazy mode\n",
					ret);
			dw_edma_free_unlinked_chunk(chan, shadow);
			dw_edma_free_unlinked_chunk(chan, next_shadow);
			spin_lock_irqsave(&chan->vc.lock, flags);
			desc->shadow_chunk = NULL;
			desc->shadow_next_chunk = NULL;
			desc->lazy_mode = false;
			if (!cancelled)
				WRITE_ONCE(chan->hw_err, true);
			desc->shadow_state = SHADOW_IDLE;
			spin_unlock_irqrestore(&chan->vc.lock, flags);
			if (READ_ONCE(chan->transfer_wq))
				wake_up(READ_ONCE(chan->transfer_wq));
			return;
		}

		/*
		 * Deferred ISR handling: swap chunks, launch copied LLI,
		 * then queue next shadow build if more SG entries remain.
		 */
		spin_lock_irqsave(&chan->vc.lock, flags);

		/* Check if transfer was aborted/cancelled while we built */
		if (desc->shadow_state == SHADOW_CANCELLED ||
		    READ_ONCE(chan->aborted) ||
		    chan->request != EDMA_REQ_NONE) {
			desc->shadow_state = SHADOW_IDLE;
			spin_unlock_irqrestore(&chan->vc.lock, flags);
			dw_edma_free_unlinked_chunk(chan, next_shadow);
			return;
		}

		/* Swap: shadow becomes active, old active goes for reuse */
		shadow_cb = shadow->cb;
		desc->shadow_chunk = NULL; /* consumed — prevent stale ref in free_desc */
		old_chunk = list_first_entry_or_null(&desc->chunk->list,
						     struct dw_edma_chunk,
						     list);
		/*
		 * xfer_sz was already accumulated by the ISR when it set
		 * ISR_PENDING, so don't accumulate again here.
		 */
		desc->alloc_sz += desc->shadow_alloc_sz;
		desc->shadow_alloc_sz = 0;

		/* Free old chunk's bursts and queue for deferred free */
		if (old_chunk) {
			dw_edma_free_burst(old_chunk);
			list_del(&old_chunk->list);
			desc->chunks_alloc--;
			dw_edma_defer_chunk_free(chan, old_chunk);
		}

		/* Insert shadow chunk into desc's chunk list as active */
		shadow->cb = shadow_cb;
		list_add(&shadow->list, &desc->chunk->list);
		desc->chunks_alloc++;

		desc->shadow_state = SHADOW_IDLE;
		/* Next shadow toggles CB from this one */
		desc->shadow_expected_cb = !shadow_cb;
		dbg_core("[SHADOW] WQ ISR_PENDING: ch%d swap old=%p new=%p(burst=%p bursts=%d)\n",
			 chan->id, old_chunk, shadow, shadow->burst, shadow->bursts_alloc);
		chan->status = EDMA_ST_BUSY;
		if (!dw_edma_shadow_launch_allowed(chan)) {
			desc->lazy_mode = false;
			WRITE_ONCE(chan->hw_err, true);
			spin_unlock_irqrestore(&chan->vc.lock, flags);
			dw_edma_free_unlinked_chunk(chan, next_shadow);
			if (READ_ONCE(chan->transfer_wq))
				wake_up(READ_ONCE(chan->transfer_wq));
			return;
		}

		/*
		 * Launch doorbell UNDER vc.lock.  Ensures shadow_state transitions to
		 * BUILDING before the next Done ISR can see IDLE.
		 *
		 * LLI was pre-copied outside lock before entering this block,
		 * so this IRQ-disabled section only programs LLP + doorbell.
		 */
		if (!desc->shadow_has_more) {
			WRITE_ONCE(desc->lazy_mode, false);
			desc->sg_remaining = 0;
			desc->shadow_next_remaining = 0;
		}
		ret = dw_edma_v0_core_launch_precopied(shadow);
		if (ret) {
			desc->lazy_mode = false;
			spin_unlock_irqrestore(&chan->vc.lock, flags);
			dw_edma_free_unlinked_chunk(chan, next_shadow);
			if (READ_ONCE(chan->transfer_wq))
				wake_up(READ_ONCE(chan->transfer_wq));
			return;
		}

		/* Queue next shadow build if SG entries remain */
		if (desc->shadow_has_more && next_shadow) {
			next_shadow->cb = desc->shadow_expected_cb;
			queued_shadow = next_shadow;
			desc->shadow_chunk = next_shadow;
			desc->shadow_next_chunk = NULL;
			desc->shadow_alloc_sz = 0;
			desc->shadow_state = SHADOW_BUILDING;
			spin_unlock_irqrestore(&chan->vc.lock, flags);
			next_shadow = NULL; /* ownership transferred */
			if (chan->chip->dw->shadow_wq) {
				queue_work(chan->chip->dw->shadow_wq,
					   &desc->shadow_work);
			} else {
				dev_err(chan->chip->dev,
					"[SHADOW] failed to queue next build, aborting lazy mode\n");
				spin_lock_irqsave(&chan->vc.lock, flags);
				if (desc->shadow_state == SHADOW_BUILDING &&
				    desc->shadow_chunk == queued_shadow) {
					desc->shadow_chunk = NULL;
					desc->shadow_state = SHADOW_IDLE;
					desc->lazy_mode = false;
					WRITE_ONCE(chan->hw_err, true);
				}
				spin_unlock_irqrestore(&chan->vc.lock, flags);
				dw_edma_free_unlinked_chunk(chan, queued_shadow);
				if (READ_ONCE(chan->transfer_wq))
					wake_up(READ_ONCE(chan->transfer_wq));
				return;
			}
		} else if (desc->shadow_has_more && !next_shadow) {
			/*
			 * Shadow chunk pre-alloc failed — cannot continue
			 * lazy mode. Signal error so user-space gets -EIO
			 * rather than silent partial transfer.
			 */
			spin_unlock_irqrestore(&chan->vc.lock, flags);
			dev_err(chan->chip->dev,
				"[SHADOW] chunk pre-alloc failed, aborting\n");
			WRITE_ONCE(chan->aborted, true);
			WRITE_ONCE(desc->lazy_mode, false);
		} else {
			spin_unlock_irqrestore(&chan->vc.lock, flags);
		}

		/* Free unused pre-allocated chunk if any */
		dw_edma_free_unlinked_chunk(chan, next_shadow);
}

/*
 * dw_edma_shadow_prepare_build - Prepare the next shadow chunk.
 * Called from PREP/process context before the current transfer is issued.
 * Transitions: IDLE → BUILDING.  The actual work item is queued by
 * dw_edma_start_transfer() once the descriptor is active.
 */
static bool dw_edma_shadow_prepare_build(struct dw_edma_desc *desc)
{
	struct dw_edma_chan *chan = desc->chan;
	struct dw_edma_chunk *shadow;

	if (!desc->lazy_mode || desc->shadow_next_remaining == 0)
		return false;

	/* Use pre-allocated chunk from previous WQ cycle if available */
	shadow = desc->shadow_next_chunk;
	desc->shadow_next_chunk = NULL;

	if (!shadow) {
		/*
		 * No pre-allocated chunk (first build or alloc failed). Allocation is
		 * safe only from PREP/process context; keep the atomic-context guard
		 * so this helper is not accidentally reused from hardirq.
		 */
		if (in_atomic() || irqs_disabled()) {
			dev_err(chan->chip->dev,
				"[SHADOW] missing preallocated chunk in IRQ context\n");
			return false;
		}
		shadow = dw_edma_alloc_unlinked_chunk(chan);
		if (!shadow)
			return false;
	}

	shadow->cb = desc->shadow_expected_cb;
	desc->shadow_chunk = shadow;
	desc->shadow_alloc_sz = 0;
	desc->shadow_state = SHADOW_BUILDING;
	dbg_core("[SHADOW] queue_build: ch%d remaining=%u cb=%d chunk=%p\n",
		 chan->id, desc->shadow_next_remaining, shadow->cb, shadow);
	if (!chan->chip->dw->shadow_wq) {
		desc->shadow_state = SHADOW_IDLE;
		if (in_atomic() || irqs_disabled())
			return false;
		desc->shadow_chunk = NULL;
		dw_edma_free_unlinked_chunk(chan, shadow);
		return false;
	}

	return true;
}

static bool dw_edma_shadow_launch_allowed(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;

	return atomic_read(&dw->dev_state) == DX_DEV_LIVE &&
		atomic_read(&dw->link_state) == DX_LINK_UP &&
		!atomic_read(&dw->background_recovery_paused);
}

static void dw_edma_start_transfer(struct dw_edma_chan *chan)
{
	struct dw_edma_chunk *child;
	struct dw_edma_desc *desc;
	struct virt_dma_desc *vd;
	struct dw_edma *dw = chan->chip->dw;
	dbg_core("dw_edma_start_transfer!!\n");

	if (unlikely(READ_ONCE(dw->shutting_down) || !READ_ONCE(dw->shadow_wq))) {
		WRITE_ONCE(chan->hw_err, true);
		chan->status = EDMA_ST_IDLE;
		if (READ_ONCE(chan->transfer_wq))
			wake_up(READ_ONCE(chan->transfer_wq));
		return;
	}

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
		dev_warn(chan->chip->dev, "Child is null (chunks_alloc:%d). Forcing completion.\n", desc->chunks_alloc);
		chan->aborted = true;
		list_del(&vd->node);
		vchan_cookie_complete(vd);
		chan->status = EDMA_ST_IDLE;
		return;
	}

	dbg_core("[START] ch%d child=%p burst=%p alloc=%d\n",
		 chan->id, child, child->burst, child->bursts_alloc);

	/* Guard: reject chunk with empty burst list (prevents NULL deref in write_chunk) */
	if (unlikely(!child->burst || list_empty(&child->burst->list))) {
		dev_err(chan->chip->dev,
			"[DMA] ch%d: child %p has empty burst (burst=%p alloc=%d). Forcing completion.\n",
			chan->id, child, child->burst, child->bursts_alloc);
		chan->aborted = true;
		list_del(&vd->node);
		vchan_cookie_complete(vd);
		chan->status = EDMA_ST_IDLE;
		return;
	}

	/*
	 * Stage the chunk for the launch worker and queue it on shadow_wq.
	 *
	 * Why off-CPU: dw_edma_v0_core_prepare_start() acquires shared helper READ
	 * channels (ch2/ch3) for the host->device LLI copy.  Under multi-
	 * channel concurrency (public data channels competing for 2 helpers),
	 * the acquire path must be free to sleep so the scheduler can let helper
	 * holders complete.  Calling it directly here would run under vc.lock
	 * in atomic context (busy-wait udelay), losing fairness and burning
	 * EBUSY on transient contention.
	 *
	 * pending_launch_chunk is single-slot but safe: start_transfer is
	 * only invoked when the channel is moving IDLE/PAUSE -> BUSY, or
	 * when the previous chunk's Done ISR has just completed.  There is
	 * no path that stages a second chunk while a launch is still in
	 * flight.  cancel_work_sync() in terminate_all guarantees the worker
	 * is quiesced before any chunk free.
	 */
	chan->pending_launch_chunk = child;
	chan->pending_launch_first = !desc->xfer_sz;
	queue_work(dw->shadow_wq, &chan->launch_work);
	/* Chunk remains valid while DMA runs; freed in done_interrupt() */
}

/*
 * Launch worker: runs in process context on shadow_wq.
 *
 * Consumes chan->pending_launch_chunk.  Heavy preparation (LLI write,
 * cache sync, helper-channel descriptor copy) runs with vc.lock dropped
 * so helper acquire may sleep.  The final data doorbell is issued only
 * after re-taking vc.lock and revalidating that terminate/recovery did
 * not cancel the transfer while preparation was running.
 */
static void dw_edma_launch_work_fn(struct work_struct *work)
{
	struct dw_edma_chan *chan = container_of(work, struct dw_edma_chan,
						 launch_work);
	struct dw_edma *dw = chan->chip->dw;
	struct dw_edma_chunk *child;
	struct dw_edma_chunk *active;
	struct dw_edma_desc *desc;
	struct virt_dma_desc *vd;
	bool first;
	bool wake_transfer = false;
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&chan->vc.lock, flags);
	child = chan->pending_launch_chunk;
	first = chan->pending_launch_first;
	chan->pending_launch_chunk = NULL;

	if (!child) {
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		return;
	}

	/* Late cancellation: aborted/hw_err/recovery already handled by
	 * the path that set them; the chunk lives on desc->chunk->list and
	 * will be freed by terminate_all's drain or recovery cleanup. */
	if (!dw_edma_shadow_launch_allowed(chan) ||
	    READ_ONCE(chan->aborted) || READ_ONCE(chan->hw_err) ||
	    chan->request == EDMA_REQ_STOP ||
	    chan->status != EDMA_ST_BUSY) {
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		wake_transfer = true;
		goto out_wake;
	}

	spin_unlock_irqrestore(&chan->vc.lock, flags);

	ret = dw_edma_v0_core_prepare_start(child, first, chan->is_llm);

	spin_lock_irqsave(&chan->vc.lock, flags);
	if (ret) {
		/* prepare_start signals hw_err for real HW failures.  -EBUSY
		 * means helper acquisition still failed even from process
		 * context; no upper retry path remains for initial launch, so
		 * fail fast instead of leaving the wait thread to time out. */
		if (ret == -EBUSY)
			WRITE_ONCE(chan->hw_err, true);
		wake_transfer = true;
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		goto out_wake;
	}

	if (!dw_edma_shadow_launch_allowed(chan) ||
	    READ_ONCE(chan->aborted) || READ_ONCE(chan->hw_err) ||
	    chan->request == EDMA_REQ_STOP ||
	    chan->status != EDMA_ST_BUSY) {
		dw_edma_v0_core_cancel_prepared(child, chan->is_llm);
		wake_transfer = true;
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		goto out_wake;
	}

	vd = vchan_next_desc(&chan->vc);
	if (!vd) {
		dw_edma_v0_core_cancel_prepared(child, chan->is_llm);
		WRITE_ONCE(chan->hw_err, true);
		wake_transfer = true;
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		goto out_wake;
	}
	desc = vd2dw_edma_desc(vd);
	active = list_first_entry_or_null(&desc->chunk->list,
					 struct dw_edma_chunk, list);
	if (active != child) {
		dw_edma_v0_core_cancel_prepared(child, chan->is_llm);
		WRITE_ONCE(chan->hw_err, true);
		wake_transfer = true;
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		goto out_wake;
	}

	dw_edma_v0_core_launch_prepared(child, chan->is_llm);

	/* Queue the shadow pre-build for the NEXT chunk only after the
	 * current chunk's doorbell has been rung.  Match the original
	 * (synchronous) ordering of dw_edma_start_transfer(). */
	if (desc->lazy_mode && desc->shadow_state == SHADOW_BUILDING) {
		if (dw->shadow_wq) {
			queue_work(dw->shadow_wq, &desc->shadow_work);
		} else {
			desc->lazy_mode = false;
			desc->shadow_state = SHADOW_IDLE;
			WRITE_ONCE(chan->hw_err, true);
			wake_transfer = true;
		}
	}
	spin_unlock_irqrestore(&chan->vc.lock, flags);

out_wake:
	if (wake_transfer && READ_ONCE(chan->transfer_wq))
		wake_up(READ_ONCE(chan->transfer_wq));
}

static int dw_edma_device_config(struct dma_chan *dchan,
				 struct dma_slave_config *config)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	dbg_core("[%s] start!!\n", __func__);

	memcpy(&chan->config, config, sizeof(*config));
	chan->configured = true;
	chan->aborted = false;  /* Clear stale abort flag from previous transfer */
	WRITE_ONCE(chan->xfer_started, false);

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
	bool drain_desc = false;
	int err = 0;

	dbg_core("[%s] start for channel %s!!\n", __func__, dma_chan_name(dchan));

	/*
	 * Quiesce any pending async launch first.  The worker may be in
	 * flight (helper acquire / doorbell), so wait for it to finish or
	 * cancel before mutating channel state.  cancel_work_sync() is
	 * safe here — terminate_all is called from process context (user
	 * thread or sg_process).
	 */
	cancel_work_sync(&chan->launch_work);

	/*
	 * Hold vc.lock across the entire state machine to prevent races
	 * with issue_pending(), which checks configured/request/status
	 * under the same lock.  Without this, a concurrent issue_pending
	 * can see a partially-updated state and start a transfer on a
	 * channel that is being torn down.
	 */
	spin_lock_irqsave(&vc->lock, flags);

	/* Drop any chunk that was staged but not launched. */
	chan->pending_launch_chunk = NULL;

	if (!chan->configured) {
		/* Do nothing */
	} else if (chan->status == EDMA_ST_PAUSE) {
		chan->status = EDMA_ST_IDLE;
		chan->configured = false;
		drain_desc = true;
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
		drain_desc = true;
	} else if (dw_edma_v0_core_ch_status(chan) == DMA_COMPLETE) {
		/*
		 * The channel is in a false BUSY state, probably didn't
		 * receive or lost an interrupt
		 */
		chan->status = EDMA_ST_IDLE;
		chan->configured = false;
		drain_desc = true;
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
		} else if (READ_ONCE(chan->hw_err)) {
			/*
			 * hw_err is already set AND CS != 2.  This means one
			 * of two things, both of which require us NOT to do
			 * another engine_en cycle:
			 *
			 *   (a) Peer-killed: another channel on the same
			 *       direction issued engine_en cycle and woke us
			 *       via notify_peer_channels().  The HW is already
			 *       reset; the CS readout we just did may be a
			 *       stale latched value (not all eDMA versions
			 *       clear per-channel CS on engine cycle).
			 *
			 *   (b) signal_hw_err from a chunk-launch failure
			 *       (e.g. wait_channel_idle returned EIO at
			 *       doorbell time).  The launch never happened,
			 *       so there is nothing in flight to stop.
			 *
			 * In both cases an additional engine_en cycle would
			 * just kill more peers and trigger a cascade — at
			 * 1GB+ multi-channel stress this avalanches into 4
			 * back-to-back resets and destabilises the PCIe link
			 * ("PCIe read error!(-5)" in dmesg).  Skip HW touch
			 * and only do SW cleanup below.
			 */
			pr_info("Channel %d: hw_err already signaled (CS=%u), skipping HW reset (cascade prevention)\n",
				chan->id, cs);
			dw_edma_v0_core_clear_done_int(chan);
			dw_edma_v0_core_clear_abort_int(chan);
		} else {
			/*
			 * CS!=2 and no prior hw_err: channel may still be
			 * running or in an unknown state.  Engine reset is
			 * the only way to force-stop it.
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
		drain_desc = true;
	} else {
		chan->request = EDMA_REQ_STOP;
		dbg_core("Channel %d: EDMA_REQ_STOP set, Done ISR will complete.\n", chan->id);
	}

	if (drain_desc) {
		/* vchan_complete() can already have detached desc_completed into its
		 * private list and be about to invoke callbacks.  Synchronize the
		 * tasklet first so terminate_all cannot free state underneath a late
		 * callback, then drain every remaining descriptor list. */
		spin_unlock_irqrestore(&vc->lock, flags);
		vchan_synchronize(vc);
		spin_lock_irqsave(&vc->lock, flags);
		vchan_get_all_descriptors(vc, &head);

		/* Drop lock before freeing descriptors -- free callbacks
		 * may do memory operations incompatible with spinlock. */
		spin_unlock_irqrestore(&vc->lock, flags);
		if (!list_empty(&head))
			vchan_dma_desc_free_list(vc, &head);
		return err;
	}

	spin_unlock_irqrestore(&vc->lock, flags);
	return err;
}

static void dw_edma_device_issue_pending(struct dma_chan *dchan)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	struct dw_edma *dw = chan->chip->dw;
	unsigned long flags;

	spin_lock_irqsave(&chan->vc.lock, flags);
	dbg_core("[%s] start!!\n", __func__);

	if (!READ_ONCE(dw->shutting_down) && chan->configured && chan->request == EDMA_REQ_NONE &&
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
	struct dw_edma_burst *prep_batch[DW_EDMA_PREP_BURST_BATCH_SIZE];
	struct dw_edma_desc *desc;
	u32 shadow_min_sg;
	u32 remaining;
	u32 cnt = 0;
	u64 addr;
	int filled;
	int i;

	if (!chan->configured) {
		pr_err("DMA channel configure is missed\n");
		return NULL;
	}
	if (chan->dir == EDMA_DIR_READ && chan->id >= DX_H2C_DATA_CH_CNT) {
		dev_err(chan->chip->dev,
			"READ channel %d is helper-reserved; normal DMA is not allowed\n",
			chan->id);
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
	shadow_min_sg = READ_ONCE(shadow_wq_min_sg);

	if (xfer->type == EDMA_XFER_SCATTER_GATHER) {
		remaining = cnt;
		addr = (chan->dir == EDMA_DIR_WRITE) ? src_addr : dst_addr;

		while (remaining > 0) {
			if (!sg) {
				pr_err("Scatter Gather list ended early (remaining=%u)\n",
				       remaining);
				goto err_alloc;
			}

			if (chunk->bursts_alloc == chan->ll_max) {
				/*
				 * Lazy burst mode with shadow pre-build: save SG state
				 * for the shadow workqueue to build the next chunk's LLI
				 * while the current chunk is being DMA'd.
				 */
				if (chan->dir == EDMA_DIR_WRITE &&
				    READ_ONCE(shadow_wq) &&
				    (!shadow_min_sg || cnt >= shadow_min_sg) &&
				    dw_edma_v0_core_shadow_precopy_available(chan->chip->dw) &&
				    desc->chunks_alloc >= 1 &&
				    remaining >= 2) {
					desc->sg_cur = sg;
					desc->sg_remaining = remaining;
					desc->addr_accum = addr;
					desc->lazy_mode = true;

					/* Init shadow state for WQ pre-build */
					desc->shadow_next_sg = sg;
					desc->shadow_next_remaining = remaining;
					desc->shadow_next_addr = addr;
					desc->shadow_alloc_sz = 0;
					desc->shadow_has_more = true;
					desc->shadow_state = SHADOW_IDLE;
					desc->shadow_expected_cb = !chunk->cb;
					desc->shadow_chunk = NULL;
					desc->shadow_next_chunk = NULL;
					INIT_WORK(&desc->shadow_work,
						  dw_edma_shadow_build_work);
					desc->shadow_work_initialized = true;

					if (dw_edma_shadow_prepare_build(desc))
						break;
					desc->lazy_mode = false;
					desc->shadow_has_more = false;
					desc->shadow_state = SHADOW_IDLE;
				}

				/* Fixed-buffer lazy mode: save the remaining SG state
				 * and refill this channel's active chunk after the
				 * current chunk completes. */
				desc->sg_cur = sg;
				desc->sg_remaining = remaining;
				desc->addr_accum = addr;
				WRITE_ONCE(desc->lazy_mode, true);
				desc->shadow_state = SHADOW_IDLE;
				desc->shadow_has_more = false;
				INIT_WORK(&desc->lazy_work,
					  dw_edma_lazy_refill_work);
				desc->lazy_work_initialized = true;
				break;
			}

			filled = dw_edma_fill_sg_bursts(desc, chunk, &sg,
							 &remaining, &addr,
							 &desc->alloc_sz,
							 prep_batch,
							 ARRAY_SIZE(prep_batch));
			if (unlikely(filled <= 0))
				goto err_alloc;
		}

		return vchan_tx_prep(&chan->vc, &desc->vd, xfer->flags);
	}

	for (i = 0; i < cnt; i++) {
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
		else if (xfer->type == EDMA_XFER_INTERLEAVED)
			burst->sz = xfer->xfer.il->sgl[i].size;

		chunk->ll_region.sz += burst->sz;
		desc->alloc_sz += burst->sz;

		if (chan->dir == EDMA_DIR_WRITE) {
			burst->sar = src_addr;
			if (xfer->type == EDMA_XFER_CYCLIC) {
				burst->dar = xfer->xfer.cyclic.paddr;
			}
		} else {
			burst->dar = dst_addr;
			if (xfer->type == EDMA_XFER_CYCLIC) {
				burst->sar = xfer->xfer.cyclic.paddr;
			}
		}

		if ((i == 0) || (i == cnt-1)) {
			dbg_tfr("[%s][%d/%d]: Type:%d, DIR:%d, SAR:%llx DAR:%llx \n", \
				__func__, i+1, cnt,
				xfer->type, chan->dir, burst->sar, burst->dar);
		}

		if (xfer->type == EDMA_XFER_INTERLEAVED &&
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

	if (!user_irq) {
		pr_err("Invalid user_irq\n");
		return IRQ_NONE;
	}
	dbg_irq("event irq data : %d[%p,%p]\n", user_irq->events_irq, &user_irq->events_irq, &user_irq->events_wq);

#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
	{
		int dma_n = get_irq_to_dma_num(user_irq->dw, user_irq->user_idx);
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

/*
 * Complete one Done event after the caller has already acknowledged or
 * validated the hardware condition.  Called with vc.lock held and
 * chan->xfer_started known true; releases vc.lock before returning.
 */
static bool dw_edma_complete_done_locked(struct dw_edma_chan *chan,
					 unsigned long flags)
	__releases(&chan->vc.lock)
{
	struct dw_edma_desc *desc;
	struct virt_dma_desc *vd;
	struct dw_edma_chunk *child;
	bool wake_transfer = false;

	WRITE_ONCE(chan->xfer_started, false);
	atomic64_inc(&chan->done_isr_cnt);
	WRITE_ONCE(chan->last_done_isr_jiffies, jiffies);

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

			if (dw_edma_defer_lazy_done_locked(chan, desc,
								&wake_transfer)) {
				spin_unlock_irqrestore(&chan->vc.lock, flags);
				if (wake_transfer && READ_ONCE(chan->transfer_wq))
					wake_up(READ_ONCE(chan->transfer_wq));
				return true;
			}

			dw_edma_free_burst(child);

			/* Queue chunk for deferred free outside the completion path. */
			list_del(&child->list);
			desc->chunks_alloc--;
			dw_edma_defer_chunk_free(chan, child);
		}

		switch (chan->request) {
		case EDMA_REQ_NONE:
			if (desc->chunks_alloc) {
				chan->status = EDMA_ST_BUSY;
				dw_edma_start_transfer(chan);
			} else {
				list_del(&vd->node);
				vchan_cookie_complete(vd);
				chan->status = EDMA_ST_IDLE;
			}
			break;

		case EDMA_REQ_STOP:
			/* Cancel shadow pre-build if active */
			if (desc->shadow_state == SHADOW_BUILDING ||
			    desc->shadow_state == SHADOW_ISR_PENDING)
				desc->shadow_state = SHADOW_CANCELLED;
			else if (desc->shadow_state == SHADOW_READY)
				desc->shadow_state = SHADOW_IDLE;

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
	return true;
}

static bool dw_edma_done_interrupt(struct dw_edma_chan *chan)
{
	unsigned long flags;

	dbg_core("[%s] start!!\n", __func__);

	/* Real IRQ path: int_status already reported this channel's Done bit. */
	dw_edma_v0_core_clear_done_int(chan);

	spin_lock_irqsave(&chan->vc.lock, flags);
	if (!READ_ONCE(chan->xfer_started)) {
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		return false;
	}

	return dw_edma_complete_done_locked(chan, flags);
}

int dw_edma_process_done_if_stopped(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	unsigned long flags;
	u32 cs;

	if (atomic_read(&dw->dev_state) != DX_DEV_LIVE ||
	    atomic_read(&dw->link_state) != DX_LINK_UP ||
	    atomic_read(&dw->background_recovery_paused))
		return -ENODEV;

	/*
	 * Replay path: polling/recovery only knows the channel appears stopped.
	 * Do not write int_clear until software still owns an active transfer and
	 * CS has been stopped for at least one jiffy.  Clearing before those guards
	 * can consume a genuine Done bit for a very fast neighbouring transfer.
	 */
	spin_lock_irqsave(&chan->vc.lock, flags);
	if (!READ_ONCE(chan->xfer_started)) {
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		return -EAGAIN;
	}
	if (time_before(jiffies,
			READ_ONCE(chan->last_xfer_start_jiffies) + 1)) {
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		return -EAGAIN;
	}
	if (dw_edma_v0_core_ch_status_checked(chan, &cs)) {
		WRITE_ONCE(chan->hw_err, true);
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		if (READ_ONCE(chan->transfer_wq))
			wake_up(READ_ONCE(chan->transfer_wq));
		return -EAGAIN;
	}
	if (cs != DMA_STOP) {
		spin_unlock_irqrestore(&chan->vc.lock, flags);
		return -EAGAIN;
	}

	dw_edma_v0_core_clear_done_int(chan);
	dw_edma_complete_done_locked(chan, flags);

	atomic64_inc(&chan->done_replay_cnt);
	WRITE_ONCE(chan->last_done_replay_jiffies, jiffies);
	return 0;
}
EXPORT_SYMBOL_GPL(dw_edma_process_done_if_stopped);

bool dw_edma_shadow_transfer_active(struct dw_edma_chan *chan)
{
	struct virt_dma_desc *vd;
	struct dw_edma_desc *desc;
	unsigned long flags;
	bool active = false;
	int lazy_refill_state;

	spin_lock_irqsave(&chan->vc.lock, flags);
	vd = vchan_next_desc(&chan->vc);
	if (!vd)
		goto out;

	desc = vd2dw_edma_desc(vd);
	if (!READ_ONCE(desc->lazy_mode))
		goto out;
	if (READ_ONCE(chan->xfer_started)) {
		active = true;
		goto out;
	}
	lazy_refill_state = READ_ONCE(desc->lazy_refill_state);
	if (lazy_refill_state == LAZY_REFILL_PENDING ||
	    lazy_refill_state == LAZY_REFILL_PREPARING) {
		active = true;
		goto out;
	}

	/*
	 * Shadow WQ progress is represented by the SW state machine, not by
	 * channel CS.  CS may report the LL-fetch/error encoding at a normal
	 * chunk boundary while the Done ISR/WQ handshake is still in progress.
	 */
	switch (desc->shadow_state) {
	case SHADOW_BUILDING:
	case SHADOW_READY:
	case SHADOW_ISR_PENDING:
		active = true;
		break;
	case SHADOW_IDLE:
		active = desc->shadow_has_more;
		break;
	default:
		break;
	}

out:
	spin_unlock_irqrestore(&chan->vc.lock, flags);
	return active;
}

static void dw_edma_abort_interrupt(struct dw_edma_chan *chan)
{
	struct virt_dma_desc *vd;
	unsigned long flags;
	struct dw_edma *dw = chan->chip->dw;
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
	u32 err_status;
#endif

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
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
	err_status = dw_edma_v0_core_ch_recover_abort(chan);
#else
	dw_edma_v0_core_ch_recover_abort(chan);
#endif

	/*
	 * Mark channel as aborted BEFORE completing the descriptor.
	 * dw_edma_sg_process checks this flag after wakeup so it can
	 * report -EIO instead of success.
	 */
	WRITE_ONCE(chan->aborted, true);

	spin_lock_irqsave(&chan->vc.lock, flags);
	vd = vchan_next_desc(&chan->vc);
	if (vd) {
		struct dw_edma_desc *desc = vd2dw_edma_desc(vd);

		/* Cancel any active shadow pre-build */
		if (desc->shadow_state == SHADOW_BUILDING ||
		    desc->shadow_state == SHADOW_ISR_PENDING)
			desc->shadow_state = SHADOW_CANCELLED;

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
	if (val && g_perf_enabled) {
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

		atomic64_inc(&chan->done_status_seen_cnt);
		WRITE_ONCE(chan->last_done_status_jiffies, jiffies);
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
	irqreturn_t ret = IRQ_NONE;
	irqreturn_t r;

	r = dw_edma_interrupt(irq, data, true);
	if (r == IRQ_HANDLED)
		ret = IRQ_HANDLED;

	r = dw_edma_interrupt(irq, data, false);
	if (r == IRQ_HANDLED)
		ret = IRQ_HANDLED;

	return ret;
}

static int dw_edma_alloc_chan_resources(struct dma_chan *dchan)
{
	struct dw_edma_chan *chan = dchan2dw_edma_chan(dchan);
	dbg_core("[%s] start!!\n", __func__);

	if (chan->dir == EDMA_DIR_READ && chan->id >= DX_H2C_DATA_CH_CNT) {
		dev_dbg(chan->chip->dev,
			"READ channel %d is reserved for descriptor-copy helper\n",
			chan->id);
		return -EBUSY;
	}

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
	u32 alloc;
	u32 i, j, cnt;
	int err = 0;
	u32 pos;

	if (write) {
		i = 0;
		cnt = dw->wr_ch_cnt;
		dma = &dw->wr_edma;
		alloc = wr_alloc;
	} else {
		i = dw->wr_ch_cnt;
		cnt = dw->rd_ch_cnt;
		dma = &dw->rd_edma;
		alloc = rd_alloc;
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
		atomic64_set(&chan->done_status_seen_cnt, 0);
		atomic64_set(&chan->done_isr_cnt, 0);
		atomic64_set(&chan->done_replay_cnt, 0);
		chan->last_done_status_jiffies = 0;
		chan->last_done_isr_jiffies = 0;
		chan->last_done_replay_jiffies = 0;

		if (write)
			chan->ll_max = (dw->ll_region_wr[j].sz / EDMA_LL_SZ);
		else
			chan->ll_max = (dw->ll_region_rd[j].sz / EDMA_LL_SZ);
		chan->ll_max -= 1;

		chan->burst_batch = kvzalloc(chan->ll_max * sizeof(*chan->burst_batch),
					    GFP_KERNEL);
		if (!chan->burst_batch)
			return -ENOMEM;

		chan->shadow_burst_batch = kvzalloc(chan->ll_max * sizeof(*chan->shadow_burst_batch),
						   GFP_KERNEL);
		if (!chan->shadow_burst_batch) {
			kvfree(chan->burst_batch);
			chan->burst_batch = NULL;
			return -ENOMEM;
		}

		/*
		 * Per-channel 1MB descriptor staging buffer.
		 * Skipped automatically for helper RD ch2/3.
		 */
		if (dx_dma_chan_alloc_desc_buf(chan)) {
			kvfree(chan->burst_batch);
			chan->burst_batch = NULL;
			kvfree(chan->shadow_burst_batch);
			chan->shadow_burst_batch = NULL;
			return -ENOMEM;
		}

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

		INIT_WORK(&chan->launch_work, dw_edma_launch_work_fn);
		chan->pending_launch_chunk = NULL;

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
		if (dw->user_irq_vec_table[i].handler) {
			user_irq = &(dw->irq[dma_irq_cnt + i].user_irq);
			user_irq->handler = dw->user_irq_vec_table[i].handler;
			snprintf(user_irq->name, sizeof(user_irq->name), 
				"%s_%s", dw->name, dw->user_irq_vec_table[i].name);
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
	bool wr_registered = false;
	bool rd_registered = false;
	int i, err;

	if (!chip) {
		pr_err("Chip pointer error!(%p)\n", chip);
		return -EINVAL;
	}

	dev = chip->dev;
	if (!dev) {
		dev_err(dev, "Device pointer error!(%p)\n", dev);
		return -EINVAL;
	}

	dw = chip->dw;
	if (!dw) {
		dev_err(dev, "dw pointer error!(%p)\n", dw);
		return -EINVAL;
	}
	if (!dw->irq || !dw->ops || !dw->ops->irq_vector) {
		dev_err(dev, "dw setting errors!(dw:%p,irq:%p,ops:%p,vec:%p)\n",
			dw, dw->irq, dw->ops,
			dw->ops ? dw->ops->irq_vector : NULL);
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
	WRITE_ONCE(dw->shutting_down, false);

	/*
	 * Dedicated WQ for off-CPU DMA work:
	 *   - launch_work   (per channel, initial chunk launch off vc.lock)
	 *   - shadow_work   (per descriptor, next-chunk LLI pre-build)
	 *   - lazy_work     (per descriptor, fixed-buffer refill)
	 *
	 * Each channel can hold up to ~3 concurrent work items.  Provision
	 * max_active to (wr+rd) * 4 so a slow helper-acquire on one channel
	 * does not throttle independent channels' workers.  WQ_UNBOUND
	 * yields the scheduler when usleep_range is used inside acquire.
	 */
	dw->shadow_wq = alloc_workqueue("edma_shadow_%d", WQ_UNBOUND | WQ_HIGHPRI,
					(dw->wr_ch_cnt + dw->rd_ch_cnt) * 4,
					dw->idx);
	if (!dw->shadow_wq) {
		dev_err(dev, "Failed to create shadow workqueue\n");
		dw_edma_mem_deinit(dw);
		return -ENOMEM;
	}

	// snprintf(dw->name, sizeof(dw->name), "dx-dma_%d", chip->id);
	snprintf(dw->name, sizeof(dw->name), "dx-dma_%d", dw->idx);

	/* Disable eDMA, only to establish the ideal initial conditions */
	dw_edma_v0_core_off(dw);

	/* Request IRQs */
	err = dw_edma_irq_request(chip, &wr_alloc, &rd_alloc);
	if (err)
		goto err_irq_free;

	/* Setup write channels */
	err = dw_edma_channel_setup(chip, true, wr_alloc, rd_alloc);
	if (err)
		goto err_irq_free;
	wr_registered = true;

	/* Setup read channels */
	err = dw_edma_channel_setup(chip, false, wr_alloc, rd_alloc);
	if (err)
		goto err_irq_free;
	rd_registered = true;

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
	if (rd_registered)
		dw_edma_unregister_dma_device(&dw->rd_edma);
	if (wr_registered)
		dw_edma_unregister_dma_device(&dw->wr_edma);

	/* Free burst_batch and shadow_burst_batch allocated during channel_setup */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		kvfree(dw->chan[i].burst_batch);
		dw->chan[i].burst_batch = NULL;
		kvfree(dw->chan[i].shadow_burst_batch);
		dw->chan[i].shadow_burst_batch = NULL;
		dx_dma_chan_free_desc_buf(&dw->chan[i]);
	}

	for (i = (dw->nr_irqs - 1); i >= 0; i--) {
		int irq = dw->ops->irq_vector(dev, i);
		free_irq(irq, &dw->irq[i]);
	}

	dw->nr_irqs = 0;

	if (dw->shadow_wq) {
		destroy_workqueue(dw->shadow_wq);
		dw->shadow_wq = NULL;
	}

	dw_edma_mem_deinit(dw);

	return err;
}


int dx_dma_remove(struct dw_edma_chip *chip)
{
	struct dw_edma_chan *chan, *_chan;
	struct device *dev = chip->dev;
	struct dw_edma *dw = chip->dw;
	int i;

	WRITE_ONCE(dw->shutting_down, true);

	/* Disable eDMA */
	dw_edma_v0_core_off(dw);

	/*
	 * Flush in-flight shadow work early so terminate_all doesn't
	 * race with running WQ callbacks.  Do NOT destroy yet — ISRs
	 * are still registered and a stale interrupt could queue_work.
	 */
	if (dw->shadow_wq)
		flush_workqueue(dw->shadow_wq);

	/*
	 * Terminate all channels and kill tasklets before pool free.  A tasklet
	 * completion can queue deferred descriptor free work on shadow_wq, so
	 * per-channel resources are released only after IRQs are freed and the
	 * final post-kill flush below completes.  Use the same two-pass terminate
	 * pattern as recovery: the first pass may only set EDMA_REQ_STOP for a BUSY
	 * channel, while the second pass performs the forced drain/reset path.
	 */
	list_for_each_entry_safe(chan, _chan, &dw->wr_edma.channels,
				 vc.chan.device_node) {
		dw_edma_device_terminate_all(&chan->vc.chan);
		dw_edma_device_terminate_all(&chan->vc.chan);
		tasklet_kill(&chan->vc.task);
	}

	list_for_each_entry_safe(chan, _chan, &dw->rd_edma.channels,
				 vc.chan.device_node) {
		dw_edma_device_terminate_all(&chan->vc.chan);
		dw_edma_device_terminate_all(&chan->vc.chan);
		tasklet_kill(&chan->vc.task);
	}

	if (dw->shadow_wq)
		flush_workqueue(dw->shadow_wq);

	/* Free IRQs — no more ISRs after this */
	for (i = (dw->nr_irqs - 1); i >= 0; i--) {
		int irq = dw->ops->irq_vector(dev, i);
		free_irq(irq, &dw->irq[i]);
	}
	dw->nr_irqs = 0;

	/* Drain any tasklet/work that raced with IRQ teardown. */
	list_for_each_entry_safe(chan, _chan, &dw->wr_edma.channels,
				 vc.chan.device_node)
		tasklet_kill(&chan->vc.task);

	list_for_each_entry_safe(chan, _chan, &dw->rd_edma.channels,
				 vc.chan.device_node)
		tasklet_kill(&chan->vc.task);

	if (dw->shadow_wq)
		flush_workqueue(dw->shadow_wq);

	/* Unregister DMA devices before freeing channel buffers/workqueues. */
	dw_edma_unregister_dma_device(&dw->wr_edma);
	dw_edma_unregister_dma_device(&dw->rd_edma);

	if (dw->shadow_wq)
		flush_workqueue(dw->shadow_wq);

	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		chan = &dw->chan[i];
		kvfree(chan->burst_batch);
		chan->burst_batch = NULL;
		kvfree(chan->shadow_burst_batch);
		chan->shadow_burst_batch = NULL;
		dx_dma_chan_free_desc_buf(chan);
	}

	/*
	 * Destroy shadow WQ AFTER IRQ free — guarantees no new
	 * queue_work() calls.  NULL guards at call sites protect
	 * the narrow window between engine_off and free_irq.
	 */
	if (dw->shadow_wq) {
		destroy_workqueue(dw->shadow_wq);
		dw->shadow_wq = NULL;
	}

	/* Turn debugfs off */
	dw_edma_v0_core_debugfs_off(chip);

	/* Free Global Memory Pools — LAST, after all users are gone */
	dw_edma_mem_deinit(dw);

	/* Power management */
	if (dw->pm_runtime_managed)
		pm_runtime_disable(dev);
	dw_edma_v0_core_engine_disable(dw->chan);

	return 0;
}
