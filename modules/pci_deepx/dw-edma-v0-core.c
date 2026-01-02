// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#include <linux/bitfield.h>
#include <linux/delay.h>

#include "dw-edma-core.h"
#include "dw-edma-v0-core.h"
#include "dw-edma-v0-regs.h"
#include "dw-edma-v0-debugfs.h"

#include "dx_lib.h"
#include "dx_util.h"

/* Minimal anomaly diagnostics (avoid verbose normal path logging) */
#ifndef EDMA_ANOMALY_DIAG
#define EDMA_ANOMALY_DIAG 1
#endif

#if EDMA_ANOMALY_DIAG
#define edma_anom_warn(fmt, ...) \
	printk_ratelimited(KERN_WARNING pr_fmt("%s:%d: " fmt), __func__, __LINE__, ##__VA_ARGS__)
#else
#define edma_anom_warn(fmt, ...) do { } while (0)
#endif

enum dw_edma_control {
	DW_EDMA_V0_CB					= BIT(0),
	DW_EDMA_V0_TCB					= BIT(1),
	DW_EDMA_V0_LLP					= BIT(2),
	DW_EDMA_V0_LIE					= BIT(3),
	DW_EDMA_V0_RIE					= BIT(4),
	DW_EDMA_V0_CCS					= BIT(8),
	DW_EDMA_V0_LLE					= BIT(9),
};

/* iATU Register */
static inline struct dw_iatu_reg_t __iomem *__dw_iatu_regs(struct dw_edma *dw)
{
	return dw->iatu_region.vaddr;
}
#define SET_IATU_IN32(dw, idx, name, value)				\
	writel(value, &(__dw_iatu_regs(dw)->reg[idx].in.name))

#define GET_IATU_IN32(dw, idx, name)				\
	readl(&(__dw_iatu_regs(dw)->reg[idx].in.name))

/* NPU0 SW Register */
static inline struct dw_npu_sw_reg_t __iomem *__dw_npu0_regs(struct dw_edma *dw)
{
	return dw->npu_region[0].vaddr;
}
#define SET_NPU0_SW(dw, idx, value)				\
	writel(value, &(__dw_npu0_regs(dw)->reg[idx]))

#define GET_NPU0_SW(dw, idx)				\
	readl(&(__dw_npu0_regs(dw)->reg[idx]))

static inline struct dw_edma_v0_regs __iomem *__dw_regs(struct dw_edma *dw)
{
	return dw->rg_region.vaddr;
}

#define SET_32(dw, name, value)				\
	writel(value, &(__dw_regs(dw)->name))

#define GET_32(dw, name)				\
	readl(&(__dw_regs(dw)->name))

#define SET_RW_32(dw, dir, name, value)			\
	do {						\
		if ((dir) == EDMA_DIR_WRITE)		\
			SET_32(dw, wr_##name, value);	\
		else					\
			SET_32(dw, rd_##name, value);	\
	} while (0)

#define GET_RW_32(dw, dir, name)			\
	((dir) == EDMA_DIR_WRITE			\
	  ? GET_32(dw, wr_##name)			\
	  : GET_32(dw, rd_##name))

#define SET_BOTH_32(dw, name, value)			\
	do {						\
		SET_32(dw, wr_##name, value);		\
		SET_32(dw, rd_##name, value);		\
	} while (0)

#ifdef CONFIG_64BIT

#define SET_64(dw, name, value)				\
	writeq(value, &(__dw_regs(dw)->name))

#define GET_64(dw, name)				\
	readq(&(__dw_regs(dw)->name))

#define SET_RW_64(dw, dir, name, value)			\
	do {						\
		if ((dir) == EDMA_DIR_WRITE)		\
			SET_64(dw, wr_##name, value);	\
		else					\
			SET_64(dw, rd_##name, value);	\
	} while (0)

#define GET_RW_64(dw, dir, name)			\
	((dir) == EDMA_DIR_WRITE			\
	  ? GET_64(dw, wr_##name)			\
	  : GET_64(dw, rd_##name))

#define SET_BOTH_64(dw, name, value)			\
	do {						\
		SET_64(dw, wr_##name, value);		\
		SET_64(dw, rd_##name, value);		\
	} while (0)

#endif /* CONFIG_64BIT */

#define SET_COMPAT(dw, name, value)			\
	writel(value, &(__dw_regs(dw)->type.unroll.name))

#define SET_RW_COMPAT(dw, dir, name, value)		\
	do {						\
		if ((dir) == EDMA_DIR_WRITE)		\
			SET_COMPAT(dw, wr_##name, value); \
		else					\
			SET_COMPAT(dw, rd_##name, value); \
	} while (0)

static inline struct dw_edma_v0_ch_regs __iomem *
__dw_ch_regs(struct dw_edma *dw, enum dw_edma_dir dir, u16 ch)
{
	if (dw->mf == DX_DMA_MF_EDMA_LEGACY)
		return &(__dw_regs(dw)->type.legacy.ch);

	if (dir == EDMA_DIR_WRITE)
		return &__dw_regs(dw)->type.unroll.ch[ch].wr;

	return &__dw_regs(dw)->type.unroll.ch[ch].rd;
}

static inline void writel_ch(struct dw_edma *dw, enum dw_edma_dir dir, u16 ch,
			     u32 value, void __iomem *addr)
{
	if (dw->mf == DX_DMA_MF_EDMA_LEGACY) {
		u32 viewport_sel;
		unsigned long flags;

		raw_spin_lock_irqsave(&dw->lock, flags);

		viewport_sel = FIELD_PREP(EDMA_V0_VIEWPORT_MASK, ch);
		if (dir == EDMA_DIR_READ)
			viewport_sel |= BIT(31);

		writel(viewport_sel,
		       &(__dw_regs(dw)->type.legacy.viewport_sel));
		writel(value, addr);

		raw_spin_unlock_irqrestore(&dw->lock, flags);
	} else {
		writel(value, addr);
	}
}

static inline u32 readl_ch(struct dw_edma *dw, enum dw_edma_dir dir, u16 ch,
			   const void __iomem *addr)
{
	u32 value;

	if (dw->mf == DX_DMA_MF_EDMA_LEGACY) {
		u32 viewport_sel;
		unsigned long flags;

		raw_spin_lock_irqsave(&dw->lock, flags);

		viewport_sel = FIELD_PREP(EDMA_V0_VIEWPORT_MASK, ch);
		if (dir == EDMA_DIR_READ)
			viewport_sel |= BIT(31);

		writel(viewport_sel,
		       &(__dw_regs(dw)->type.legacy.viewport_sel));
		value = readl(addr);

		raw_spin_unlock_irqrestore(&dw->lock, flags);
	} else {
		value = readl(addr);
	}

	return value;
}

#define SET_CH_32(dw, dir, ch, name, value) \
	writel_ch(dw, dir, ch, value, &(__dw_ch_regs(dw, dir, ch)->name))

#define GET_CH_32(dw, dir, ch, name) \
	readl_ch(dw, dir, ch, &(__dw_ch_regs(dw, dir, ch)->name))

#define SET_LL_32(ll, value) \
	writel(value, ll)

#ifdef CONFIG_64BIT

static inline void writeq_ch(struct dw_edma *dw, enum dw_edma_dir dir, u16 ch,
			     u64 value, void __iomem *addr)
{
	if (dw->mf == DX_DMA_MF_EDMA_LEGACY) {
		u32 viewport_sel;
		unsigned long flags;

		raw_spin_lock_irqsave(&dw->lock, flags);

		viewport_sel = FIELD_PREP(EDMA_V0_VIEWPORT_MASK, ch);
		if (dir == EDMA_DIR_READ)
			viewport_sel |= BIT(31);

		writel(viewport_sel,
		       &(__dw_regs(dw)->type.legacy.viewport_sel));
		writeq(value, addr);

		raw_spin_unlock_irqrestore(&dw->lock, flags);
	} else {
		writeq(value, addr);
	}
}

static inline u64 readq_ch(struct dw_edma *dw, enum dw_edma_dir dir, u16 ch,
			   const void __iomem *addr)
{
	u32 value;

	if (dw->mf == DX_DMA_MF_EDMA_LEGACY) {
		u32 viewport_sel;
		unsigned long flags;

		raw_spin_lock_irqsave(&dw->lock, flags);

		viewport_sel = FIELD_PREP(EDMA_V0_VIEWPORT_MASK, ch);
		if (dir == EDMA_DIR_READ)
			viewport_sel |= BIT(31);

		writel(viewport_sel,
		       &(__dw_regs(dw)->type.legacy.viewport_sel));
		value = readq(addr);

		raw_spin_unlock_irqrestore(&dw->lock, flags);
	} else {
		value = readq(addr);
	}

	return value;
}

#define SET_CH_64(dw, dir, ch, name, value) \
	writeq_ch(dw, dir, ch, value, &(__dw_ch_regs(dw, dir, ch)->name))

#define GET_CH_64(dw, dir, ch, name) \
	readq_ch(dw, dir, ch, &(__dw_ch_regs(dw, dir, ch)->name))

#define SET_LL_64(ll, value) \
	writeq(value, ll)

#endif /* CONFIG_64BIT */

/* eDMA management callbacks */
void dw_edma_v0_core_off(struct dw_edma *dw)
{
	SET_BOTH_32(dw, int_mask,
		    EDMA_V0_DONE_INT_MASK | EDMA_V0_ABORT_INT_MASK);
	SET_BOTH_32(dw, int_clear,
		    EDMA_V0_DONE_INT_MASK | EDMA_V0_ABORT_INT_MASK);
	SET_BOTH_32(dw, engine_en, 0);
}

u16 dw_edma_v0_core_ch_count(struct dw_edma *dw, enum dw_edma_dir dir)
{
	u32 num_ch;

	if (dir == EDMA_DIR_WRITE)
		num_ch = FIELD_GET(EDMA_V0_WRITE_CH_COUNT_MASK,
				   GET_32(dw, ctrl));
	else
		num_ch = FIELD_GET(EDMA_V0_READ_CH_COUNT_MASK,
				   GET_32(dw, ctrl));

	if (num_ch > EDMA_V0_MAX_NR_CH)
		num_ch = EDMA_V0_MAX_NR_CH;

	return (u16)num_ch;
}

enum dma_status dw_edma_v0_core_ch_status(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	u32 tmp;

	tmp = FIELD_GET(EDMA_V0_CH_STATUS_MASK,
			GET_CH_32(dw, chan->dir, chan->id, ch_control1));

	if (tmp == 1) {
		dbg_tfr("[%s] status is progress\n", dma_chan_name(&chan->vc.chan));
		return DMA_IN_PROGRESS;
	}
	else if (tmp == 3)
		return DMA_COMPLETE;
	else {
		pr_err("[%s] status is error\n", dma_chan_name(&chan->vc.chan));
		return DMA_ERROR;
	}
}

#define EDMA_ENGINE_HALT_TIMEOUT_US 	(10000) // 10ms
int dw_edma_v0_core_ch_halt(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	unsigned long timeout_us;
	int ret = 0;
	u32 ch_status = 1;

	dbg_core("Channel %s is stuck. Starting global flush sequence.\n",
		 dma_chan_name(&chan->vc.chan));

	SET_RW_32(dw, chan->dir, engine_en, EDMA_ENG_DIS);
	timeout_us = EDMA_ENGINE_HALT_TIMEOUT_US;
	while (timeout_us > 0) {
		ch_status = GET_RW_32(dw, chan->dir, engine_en);
		if (ch_status == EDMA_ENG_DIS) {
			dbg_core("[%s] Channel %s %s\n",
			 __func__, dma_chan_name(&chan->vc.chan), 
			 ch_status ? "enabled" : "disabled");
			goto cleanup;
		}
		usleep_range(10, 20);
		timeout_us -= 10;
	}

	dev_err(chan->vc.chan.device->dev, "FATAL: Channel %s did not stop even after engine disable!\n",
		dma_chan_name(&chan->vc.chan));
	ret = -ETIMEDOUT;
	goto re_enable_engine;

cleanup:
	ch_status = GET_RW_32(dw, chan->dir, engine_en);
	dbg_core("[%s][%s] Clearing residual status.\n",
		 __func__, dma_chan_name(&chan->vc.chan));
	dw_edma_v0_core_clear_done_int(chan);
	dw_edma_v0_core_clear_abort_int(chan);

re_enable_engine:
	SET_RW_32(dw, chan->dir, engine_en, 1);
	dbg_core("[%s] Channel %s %s\n",
		__func__, dma_chan_name(&chan->vc.chan), 
		GET_RW_32(dw, chan->dir, engine_en) ? "enabled" : "disabled");

	return ret;
}


void dw_edma_v0_core_clear_done_int(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;

	SET_RW_32(dw, chan->dir, int_clear,
		  FIELD_PREP(EDMA_V0_DONE_INT_MASK, BIT(chan->id)));
}

void dw_edma_v0_core_clear_abort_int(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;

	SET_RW_32(dw, chan->dir, int_clear,
		  FIELD_PREP(EDMA_V0_ABORT_INT_MASK, BIT(chan->id)));
}

u32 dw_edma_v0_core_status_done_int(struct dw_edma *dw, enum dw_edma_dir dir)
{
	return FIELD_GET(EDMA_V0_DONE_INT_MASK,
			 GET_RW_32(dw, dir, int_status));
}

u32 dw_edma_v0_core_status_abort_int(struct dw_edma *dw, enum dw_edma_dir dir)
{
	return FIELD_GET(EDMA_V0_ABORT_INT_MASK,
			 GET_RW_32(dw, dir, int_status));
}

static void dw_edma_v0_gen_lli(struct dw_edma_v0_lli __iomem *lli,
							   struct dw_edma_chunk *chunk,
							   struct dw_edma_burst *burst,
							   int ch_n, u32 idx, int remain) {
	u32 control = 0;

	if (chunk->cb) {
		control = DW_EDMA_V0_CB;
	}

	if (!remain) { /* Last element */
		control |= (DW_EDMA_V0_LIE | DW_EDMA_V0_RIE);
	}

	/* Channel control */
	SET_LL_32(&lli[idx].control, control);
	/* Transfer size */
	SET_LL_32(&lli[idx].transfer_size, burst->sz);
	/* SAR */
	#if defined(CONFIG_64BIT) && defined(CONFIG_X86_64)
		SET_LL_64(&lli[idx].sar.reg, burst->sar);
	#else /* CONFIG_64BIT */
		SET_LL_32(&lli[idx].sar.lsb, lower_32_bits(burst->sar));
		SET_LL_32(&lli[idx].sar.msb, upper_32_bits(burst->sar));
	#endif /* CONFIG_64BIT */
	/* DAR */
	#if defined(CONFIG_64BIT) && defined(CONFIG_X86_64)
		SET_LL_64(&lli[idx].dar.reg, burst->dar);
	#else /* CONFIG_64BIT */
		SET_LL_32(&lli[idx].dar.lsb, lower_32_bits(burst->dar));
		SET_LL_32(&lli[idx].dar.msb, upper_32_bits(burst->dar));
	#endif /* CONFIG_64BIT */

	#ifdef DUMP_DESC_TABLE
	if ((idx < 1) || (remain < 3)) {
		pr_err("[%s][%d][LLI_%u] CB:%d,TCB:%d,LLP:%d,LIE:%d,RIE:%d,CCS:%d,LLE:%d, size:0x%x, sar:0x%x%08x, dar:0x%x%08x, Off:0x%x\n",
			(ch_n == EDMA_DIR_WRITE) ? "W" : "R", 
			chunk->chan->id, idx,
			(control & DW_EDMA_V0_CB)  ? 1:0,
			(control & DW_EDMA_V0_TCB) ? 1:0,
			(control & DW_EDMA_V0_LLP) ? 1:0,
			(control & DW_EDMA_V0_LIE) ? 1:0,
			(control & DW_EDMA_V0_RIE) ? 1:0,
			(control & DW_EDMA_V0_CCS) ? 1:0,
			(control & DW_EDMA_V0_LLE) ? 1:0,
			burst->sz,
			upper_32_bits(burst->sar), lower_32_bits(burst->sar),
			upper_32_bits(burst->dar), lower_32_bits(burst->dar),
			idx*24
		);
	}
	#endif /* DUMP_DESC_TABLE */
}

static void dw_edma_v0_gen_llp(struct dw_edma_v0_llp __iomem *llp,
							   struct dw_edma_chunk *chunk) {
	u32 control = 0;

	control = DW_EDMA_V0_LLP | DW_EDMA_V0_TCB;
	if (!chunk->cb)
		control |= DW_EDMA_V0_CB;
    /* Channel control */
	SET_LL_32(&llp->control, control);
	/* Linked list */
	#if defined(CONFIG_64BIT) && defined(CONFIG_X86_64)
		SET_LL_64(&llp->llp.reg, chunk->ll_region.paddr);
	#else /* CONFIG_64BIT */
		SET_LL_32(&llp->llp.lsb, lower_32_bits(chunk->ll_region.paddr));
		SET_LL_32(&llp->llp.msb, upper_32_bits(chunk->ll_region.paddr));
	#endif /* CONFIG_64BIT */

	#ifdef DUMP_DESC_TABLE
		pr_err("[LLP] CB:%d,TCB:%d,LLP:%d,LIE:%d,RIE:%d,CCS:%d,LLE:%d, ll_region:0x%x%08x\n",
			(control & DW_EDMA_V0_CB)  ? 1:0,
			(control & DW_EDMA_V0_TCB) ? 1:0,
			(control & DW_EDMA_V0_LLP) ? 1:0,
			(control & DW_EDMA_V0_LIE) ? 1:0,
			(control & DW_EDMA_V0_RIE) ? 1:0,
			(control & DW_EDMA_V0_CCS) ? 1:0,
			(control & DW_EDMA_V0_LLE) ? 1:0,
			upper_32_bits(chunk->ll_region.paddr), lower_32_bits(chunk->ll_region.paddr)
		);
	#endif /* DUMP_DESC_TABLE */
}

static void dw_edma_v0_core_wait_channel_idle(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	uint32_t ch_control;
	unsigned long start_jiffies;
	unsigned long timeout_jiffies;
	int retry_count;
	bool status_changed;
	unsigned long elapsed_jiffies;
	unsigned int elapsed_msecs;
	u32 int_status, done_status, abort_status;

	ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);
	if ((ch_control & (BIT(5) | BIT(6))) == BIT(5)) {
		dbg_tfr("Channel %d is busy[%llx, %llx]\n", chan->id,
			GET_CH_32(dw, chan->dir, chan->id, sar.lsb) |
			((u64)GET_CH_32(dw, chan->dir, chan->id, sar.msb) << 32),
			GET_CH_32(dw, chan->dir, chan->id, dar.lsb) |
			((u64)GET_CH_32(dw, chan->dir, chan->id, dar.msb) << 32));

		/* Start monitoring busy channel for status change */
		start_jiffies = jiffies;
		timeout_jiffies = start_jiffies + usecs_to_jiffies(5000); /* 5s timeout (5000 us) */
		retry_count = 0;
		status_changed = false;

		/* Poll until status changes to "11" (both bits set) or timeout */
		while (time_before(jiffies, timeout_jiffies)) {
			retry_count++;
			ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);

			if ((ch_control & (BIT(5) | BIT(6))) == (BIT(5) | BIT(6))) {
				status_changed = true;
				break;
			}
			/* Don't burn CPU - yield and wait a bit */
			if (retry_count % 20 == 0)
				cond_resched();
			udelay(10);
		}

		/* Report results in microseconds */
		{
			unsigned int elapsed_usecs;
			elapsed_jiffies = jiffies - start_jiffies;
			elapsed_usecs = jiffies_to_usecs(elapsed_jiffies);

			if (status_changed) {
				dbg_tfr("CH%d status changed to 0x%x after %u ms (%d retries)\n", 
						chan->id, ch_control, elapsed_msecs, retry_count);
			} else {
				pr_warn("CH%d status still busy (0x%x) after %u ms (%d retries)\n", 
						chan->id, ch_control, elapsed_msecs, retry_count);

				/* Check interrupt status to see if it was triggered */
				int_status = GET_RW_32(dw, chan->dir, int_status);
				done_status = FIELD_GET(EDMA_V0_DONE_INT_MASK, int_status);
				abort_status = FIELD_GET(EDMA_V0_ABORT_INT_MASK, int_status);
				
				pr_warn("CH%d interrupt status: done=0x%lx abort=0x%lx\n",
						chan->id, (unsigned long)(done_status & BIT(chan->id)), 
						(unsigned long)(abort_status & BIT(chan->id)));
				/* Force channel reset as a recovery measure */
				pr_warn("Forcing CH%d reset due to stuck status\n", chan->id);
				SET_CH_32(dw, chan->dir, chan->id, ch_control1, 0x0);
			}
		}
	}
}

static int wait_for_dma_channel_idle(struct dw_edma *dw, int channel, bool is_write)
{
	enum dw_edma_dir dir = is_write ? EDMA_DIR_READ : EDMA_DIR_WRITE;
	DMA_CH_CONTROL1_OFF_t ctrl1;
	int timeout = DMA_POLL_TIMEOUT_US;
	int elapsed = 0;

	while (elapsed < timeout) {
		ctrl1.U = GET_CH_32(dw, dir, channel, ch_control1);
		
		/* CS (Channel Status): DMA_ERR or DMA_STOP means idle */
		if (ctrl1.CS == DMA_ERR || ctrl1.CS == DMA_STOP)
			return 0;

		elapsed++;
	}

	pr_err("Channel %d (%s) not idle after %dms (CS=%d)\n",
	       channel, is_write ? "R" : "W", DMA_POLL_TIMEOUT_US, ctrl1.CS);
	return -ETIMEDOUT;
}

static void dw_edma_v0_core_write_chunk(struct dw_edma_chunk *chunk, int dev_n, int dma_n, int ch_n)
{
	struct dw_edma_burst *child, *curr, *next, *ptr;
	struct dw_edma_v0_lli __iomem *lli;
	struct dw_edma_v0_llp __iomem *llp;
	u32 i = 0;
	int j = 0;
	int orig_bursts;
	u32 last_ctrl = 0;
	bool last_ctrl_checked = false;

	if (unlikely(!chunk->ll_region.vaddr)) {
		pr_err("[edma] ch%d dev%d dma%d NULL ll_region.vaddr\n", chunk->chan->id, dev_n, dma_n);
	}
	if (unlikely(!chunk->burst || list_empty(&chunk->burst->list))) {
		pr_err("[edma] ch%d dev%d dma%d empty burst list\n", chunk->chan->id, dev_n, dma_n);
	}

	dx_pcie_start_profile(PCIE_DESC_GEN_T, 0, dev_n, dma_n, ch_n);

	orig_bursts = chunk->bursts_alloc;

	ptr = list_first_entry(&chunk->burst->list, struct dw_edma_burst, list);
	list_for_each_entry_safe(curr, next,&chunk->burst->list, list) {
		bool contd = false;
		if(ptr == curr) continue;

		if (ch_n == EDMA_DIR_WRITE && curr->dar == (ptr->dar + ptr->sz)) {
			contd = true;
		} else if (ch_n == EDMA_DIR_READ && curr->sar == (ptr->sar + ptr->sz)) {
			contd = true;
		}
			
		if(contd) {
			ptr->sz += curr->sz;
			list_del(&curr->list);
			dw_edma_free_single_burst(chunk->chan, curr);
			chunk->bursts_alloc--;
		} else {
			ptr = curr;
		}
	}

	lli = chunk->host_region.vaddr;
	j = chunk->bursts_alloc;
	if (unlikely(j <= 0)) {
		pr_err("[edma] ch%d merge produced zero bursts (orig=%d)\n", chunk->chan->id, orig_bursts);
	}

	list_for_each_entry(child, &chunk->burst->list, list) {
		j--;
		dw_edma_v0_gen_lli(lli, chunk, child, ch_n, i, j);
		if (j == 0) { /* last element just written */
			/* Read back control field to ensure LIE/RIE bits present for completion interrupt */
			last_ctrl = readl(&lli[i].control);
			last_ctrl_checked = true;
		}
		i++;
	}

	llp = (void __iomem *)&lli[i];
	dw_edma_v0_gen_llp(llp, chunk);

	if (last_ctrl_checked) {
		if (!(last_ctrl & (DW_EDMA_V0_LIE | DW_EDMA_V0_RIE)))
			pr_err("[edma] ch%d last desc missing LIE/RIE ctrl=0x%x desc_cnt=%u\n", chunk->chan->id, last_ctrl, i);
	}
	if (i == 0)
		pr_err("[edma] ch%d no descriptors written (orig=%d)\n", chunk->chan->id, orig_bursts);

	dx_pcie_end_profile(PCIE_DESC_GEN_T, 0, dev_n, dma_n, ch_n);
}

static void dw_edma_v0_xfer_llm(struct dw_edma_chunk *chunk)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	u32 tmp;
	u32 ch_control1;

	tmp = GET_RW_32(dw, chan->dir, int_mask);
	tmp &= ~FIELD_PREP(EDMA_V0_DONE_INT_MASK, BIT(chan->id));
	tmp &= ~FIELD_PREP(EDMA_V0_ABORT_INT_MASK, BIT(chan->id));
	SET_RW_32(dw, chan->dir, int_mask, tmp);
	
	/* Channel control */
	ch_control1 = DW_EDMA_V0_LLE;
	if (chunk->cb)
		ch_control1 |= DW_EDMA_V0_CCS;

	SET_CH_32(dw, chan->dir, chan->id, ch_control1, ch_control1);
	/* Linked list */
	SET_CH_32(dw, chan->dir, chan->id, llp.lsb,
		lower_32_bits(chunk->ll_region.paddr));
	SET_CH_32(dw, chan->dir, chan->id, llp.msb,
		upper_32_bits(chunk->ll_region.paddr));

	dbg_tfr("[LLM:%s] dir:%d llp:0x%x%08x sar:0x%x%08x, dar:0x%x%08x\n",
		dma_chan_name(&chan->vc.chan), chan->dir,
		GET_CH_32(dw, chan->dir, chan->id, llp.msb),
		GET_CH_32(dw, chan->dir, chan->id, llp.lsb),
		GET_CH_32(dw, chan->dir, chan->id, sar.msb),
		GET_CH_32(dw, chan->dir, chan->id, sar.lsb),
		GET_CH_32(dw, chan->dir, chan->id, dar.msb),
		GET_CH_32(dw, chan->dir, chan->id, dar.lsb));
	
	/* Doorbell */
	SET_RW_32(dw, chan->dir, doorbell,
		FIELD_PREP(EDMA_V0_DOORBELL_CH_MASK, chan->id));
}

static int dx_dma_polling_wait(struct dw_edma *dw, int channel, enum dw_edma_dir dir)
{
	DMA_CH_CONTROL1_OFF_t ctrl1;
	u32 xfer_size;
	int elapsed = 0;
	int max_wait = DMA_POLL_TIMEOUT_US;

	while (elapsed < max_wait) {
		ctrl1.U = GET_CH_32(dw, dir, channel, ch_control1);
		xfer_size = GET_CH_32(dw, dir, channel, transfer_size);

		if (ctrl1.CS == DMA_STOP && xfer_size == 0) {
			dbg_tfr("[%s][%d] Done (%d us)\n",
				(dir == EDMA_DIR_WRITE) ? "W" : "R", channel, elapsed);
			return 0;
		}

		if (ctrl1.CS == DMA_ERR && xfer_size != 0) {
			dbg_tfr("[%s][%d] Error: CS=%d, size=%u\n",
				(dir == EDMA_DIR_WRITE) ? "W" : "R", channel, ctrl1.CS, xfer_size);
			return -EIO;
		}

		elapsed++;
	}

	pr_err("[%s][%d] CS=%d size=%u Timeout (%d ms) \n",
		(dir == EDMA_DIR_WRITE) ? "W" : "R", channel, ctrl1.CS, xfer_size, max_wait);
	return -ETIMEDOUT;
}

static int dw_edma_v0_core_xfer_llm_desc(struct dw_edma_chunk *chunk)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	DMA_CH_CONTROL1_OFF_t ctrl1;
	int ret = 0;
	int channel = -1;
	unsigned long flags;
	bool use_dedicated = false;
	struct dma_chan_lock *target_lock = NULL;

    /* Round-Robin Toggle for Load Balancing */
    static atomic_t rr_toggle = ATOMIC_INIT(0);
    int first_ch, second_ch;

    /* Try Channel 3 first */
    if (chan->dir == EDMA_DIR_WRITE) {
        /* Toggle priority: Odd->(3,2), Even->(2,3) */
        if (atomic_inc_return(&rr_toggle) & 1) {
            first_ch = EDMA_CH_ID_3;
            second_ch = EDMA_CH_ID_2;
        } else {
            first_ch = EDMA_CH_ID_2;
            second_ch = EDMA_CH_ID_3;
        }

        while (!use_dedicated) {
            target_lock = &dw->rd_dma_chan_locks[first_ch];
            if (!target_lock->ch_in_use && spin_trylock_irqsave(&target_lock->ch_lock, flags)) {
                target_lock->ch_in_use = true;
                use_dedicated = true;
                channel = first_ch;
                spin_unlock_irqrestore(&target_lock->ch_lock, flags);
                break;
            }
            target_lock = &dw->rd_dma_chan_locks[second_ch];
            if (!target_lock->ch_in_use && spin_trylock_irqsave(&target_lock->ch_lock, flags)) {
                target_lock->ch_in_use = true;
                use_dedicated = true;
                channel = second_ch;
                spin_unlock_irqrestore(&target_lock->ch_lock, flags);
                break;
            }
        }
    }

	if (!use_dedicated) {
		ret = wait_for_dma_channel_idle(dw, chan->id, EDMA_DIR_READ);
		if (ret != 0) {
			pr_err("[R][%d] Channel not idle\n", chan->id);
			return ret;
		}
		channel = chan->id;
	}

	ctrl1.U = 0;
	SET_CH_32(dw, EDMA_DIR_READ, channel, ch_control1, ctrl1.U);
	SET_CH_32(dw, EDMA_DIR_READ, channel, ch_control2, 0);
	
	SET_CH_32(dw, EDMA_DIR_READ, channel, transfer_size,
			(chunk->bursts_alloc + 1) * EDMA_LL_SZ);

	SET_CH_32(dw, EDMA_DIR_READ, channel, sar.lsb,
			lower_32_bits(chunk->host_region.paddr));
	SET_CH_32(dw, EDMA_DIR_READ, channel, sar.msb,
			upper_32_bits(chunk->host_region.paddr));
	SET_CH_32(dw, EDMA_DIR_READ, channel, dar.lsb,
			lower_32_bits(chunk->ll_region.paddr));
	SET_CH_32(dw, EDMA_DIR_READ, channel, dar.msb,
			upper_32_bits(chunk->ll_region.paddr));
	
	dbg_tfr(">> [R][%d] > [%s][%d]", channel,
			(chan->dir== EDMA_DIR_WRITE) ? "W" : "R", chan->id);
	dbg_tfr("[NON-LLM:%d] sz:0x%x sar:0x%x%08x dar:0x%x%08x\n",
		channel,
		(chunk->bursts_alloc + 1) * EDMA_LL_SZ,
		GET_CH_32(dw, EDMA_DIR_READ, channel, sar.msb),
		GET_CH_32(dw, EDMA_DIR_READ, channel, sar.lsb),
		GET_CH_32(dw, EDMA_DIR_READ, channel, dar.msb),
		GET_CH_32(dw, EDMA_DIR_READ, channel, dar.lsb));

	SET_RW_32(dw, EDMA_DIR_READ, doorbell,
		FIELD_PREP(EDMA_V0_DOORBELL_CH_MASK, channel));

	ret = dx_dma_polling_wait(dw, channel, EDMA_DIR_READ);
	if (ret) {
		pr_err("[R][%d] LLM desc xfer fail\n", channel);
	}

	if (use_dedicated) {
		target_lock = &dw->rd_dma_chan_locks[channel];
		spin_lock_irqsave(&target_lock->ch_lock, flags);
		target_lock->ch_in_use = false;
		spin_unlock_irqrestore(&target_lock->ch_lock, flags);
	}

	return ret;
}

static void dw_edma_v0_xfer_llm_not(struct dw_edma_chunk *chunk)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	struct list_head *f = chunk->burst->list.next;
	struct dw_edma_burst *child = list_entry(f, struct dw_edma_burst, list);
	/* Channel control & size */
	SET_CH_32(dw, chan->dir, chan->id, ch_control1,
		(DW_EDMA_V0_RIE | DW_EDMA_V0_LIE));
	SET_CH_32(dw, chan->dir, chan->id, transfer_size, child->sz);
	/* SAR */
	SET_CH_32(dw, chan->dir, chan->id, sar.lsb, lower_32_bits(child->sar));
	SET_CH_32(dw, chan->dir, chan->id, sar.msb, upper_32_bits(child->sar));
	/* DAR */
	SET_CH_32(dw, chan->dir, chan->id, dar.lsb, lower_32_bits(child->dar));
	SET_CH_32(dw, chan->dir, chan->id, dar.msb, upper_32_bits(child->dar));

	dbg_tfr("[NON-LLM:%s] sz:0x%x dir:%d sar:0x%x%08x dar:0x%x%08x\n",
		dma_chan_name(&chan->vc.chan), child->sz, chan->dir,
		GET_CH_32(dw, chan->dir, chan->id, sar.msb),
		GET_CH_32(dw, chan->dir, chan->id, sar.lsb),
		GET_CH_32(dw, chan->dir, chan->id, dar.msb),
		GET_CH_32(dw, chan->dir, chan->id, dar.lsb));

	/* Doorbell */
	SET_RW_32(dw, chan->dir, doorbell,
		  FIELD_PREP(EDMA_V0_DOORBELL_CH_MASK, chan->id));
}

void dw_edma_v0_core_engine_disable(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;

	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch0_pwr_en, EDMA_ENG_DIS);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch0_pwr_en, EDMA_ENG_DIS);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch1_pwr_en, EDMA_ENG_DIS);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch1_pwr_en, EDMA_ENG_DIS);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch2_pwr_en, EDMA_ENG_DIS);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch2_pwr_en, EDMA_ENG_DIS);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch3_pwr_en, EDMA_ENG_DIS);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch3_pwr_en, EDMA_ENG_DIS);
	SET_RW_32(dw, chan->dir, engine_en, EDMA_ENG_DIS);

	dbg_tfr("DMA engines disabled (Status: 0x%x)\n", GET_RW_32(dw, chan->dir, engine_en));
}

void dw_edma_v0_core_engine_enable(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;

	SET_RW_32(dw, chan->dir, engine_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch0_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch0_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch1_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch1_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch2_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch2_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch3_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch3_pwr_en, EDMA_ENG_EN);

	dbg_tfr("DMA engines enabled (Status: 0x%x)\n", GET_RW_32(dw, chan->dir, engine_en));
}

void dw_edma_v0_core_start(struct dw_edma_chunk *chunk, bool first, bool set_desc, bool is_llm)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	int ret;
	u32 tmp;

	if (first) {
		/* Interrupt unmask - done, abort */
		tmp = GET_RW_32(dw, chan->dir, int_mask);
		tmp &= ~FIELD_PREP(EDMA_V0_DONE_INT_MASK, BIT(chan->id));
		tmp &= ~FIELD_PREP(EDMA_V0_ABORT_INT_MASK, BIT(chan->id));
		SET_RW_32(dw, chan->dir, int_mask, tmp);
	}

	dw_edma_v0_core_wait_channel_idle(chan);
	if (is_llm) {
		dw_edma_v0_core_write_chunk(chunk, dw->idx, chan->id, chan->dir);
		
		/* Sync for device if using Buddy Allocator (Streaming DMA) */
		if (chunk->is_buddy) {
			dma_sync_single_for_device(chan->chip->dev,
						   chunk->host_region.paddr,
						   chunk->host_region.sz,
						   DMA_TO_DEVICE);
		}
		ret = dw_edma_v0_core_xfer_llm_desc(chunk);
		if (ret == 0) {
			dw_edma_v0_xfer_llm(chunk);
		} else {
			pr_err(">> LLM descriptor transfer failed, aborting\n");
		}
	} else {
		dw_edma_v0_xfer_llm_not(chunk);
	}

	/* Check Channel status */
	dbg_tfr("%s, ch:%d, status:0x%x\n",
		(chan->dir == EDMA_DIR_WRITE) ? "W" : "R",
		chunk->chan->id,
		GET_CH_32(dw, chan->dir, chan->id, ch_control1));
}

int dw_edma_v0_core_device_config(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	u32 tmp = 0;

	/* MSI done addr - low, high */
	SET_RW_32(dw, chan->dir, done_imwr.lsb, chan->msi.address_lo);
	SET_RW_32(dw, chan->dir, done_imwr.msb, chan->msi.address_hi);
	/* MSI abort addr - low, high */
	SET_RW_32(dw, chan->dir, abort_imwr.lsb, chan->msi.address_lo);
	SET_RW_32(dw, chan->dir, abort_imwr.msb, chan->msi.address_hi);
	/* MSI data - low, high */
	switch (chan->id) {
	case 0:
	case 1:
		tmp = GET_RW_32(dw, chan->dir, ch01_imwr_data);
		break;

	case 2:
	case 3:
		tmp = GET_RW_32(dw, chan->dir, ch23_imwr_data);
		break;

	case 4:
	case 5:
		tmp = GET_RW_32(dw, chan->dir, ch45_imwr_data);
		break;

	case 6:
	case 7:
		tmp = GET_RW_32(dw, chan->dir, ch67_imwr_data);
		break;
	}

	if (chan->id & BIT(0)) {
		/* Channel odd {1, 3, 5, 7} */
		tmp &= EDMA_V0_CH_EVEN_MSI_DATA_MASK;
		tmp |= FIELD_PREP(EDMA_V0_CH_ODD_MSI_DATA_MASK,
				  chan->msi.data);
	} else {
		/* Channel even {0, 2, 4, 6} */
		tmp &= EDMA_V0_CH_ODD_MSI_DATA_MASK;
		tmp |= FIELD_PREP(EDMA_V0_CH_EVEN_MSI_DATA_MASK,
				  chan->msi.data);
	}

	switch (chan->id) {
	case 0:
	case 1:
		SET_RW_32(dw, chan->dir, ch01_imwr_data, tmp);
		break;

	case 2:
	case 3:
		SET_RW_32(dw, chan->dir, ch23_imwr_data, tmp);
		break;

	case 4:
	case 5:
		SET_RW_32(dw, chan->dir, ch45_imwr_data, tmp);
		break;

	case 6:
	case 7:
		SET_RW_32(dw, chan->dir, ch67_imwr_data, tmp);
		break;
	}

	return 0;
}

/* eDMA debugfs callbacks */
void dw_edma_v0_core_debugfs_on(struct dw_edma_chip *chip)
{
	dw_edma_v0_debugfs_on(chip);
}

void dw_edma_v0_core_debugfs_off(struct dw_edma_chip *chip)
{
	dw_edma_v0_debugfs_off(chip);
}

/* iATU function */
static inline void dx_iatu_inbound_address(struct dw_edma *dw, u32 idx, u64 base_addr, u64 tgt_addr, u32 size)
{
	u32 retries, val;

	SET_IATU_IN32(dw, idx, base_lo_addr, lower_32_bits(base_addr));
	SET_IATU_IN32(dw, idx, base_hi_addr, upper_32_bits(base_addr));

	SET_IATU_IN32(dw, idx, limit_lo_addr, lower_32_bits(base_addr + size - 1));
	SET_IATU_IN32(dw, idx, limit_hi_addr, upper_32_bits(base_addr + size - 1));

	SET_IATU_IN32(dw, idx, targ_lo_addr, lower_32_bits(tgt_addr));
	SET_IATU_IN32(dw, idx, targ_hi_addr, upper_32_bits(tgt_addr));

	SET_IATU_IN32(dw, idx, region_ctl2, PCIE_ATU_ENABLE | PCIE_ATU_DMA_BYPASS);
	/*
	 * Make sure ATU enable takes effect before any subsequent config
	 * and I/O accesses.
	 */
	for (retries = 0; retries < LINK_WAIT_MAX_IATU_RETRIES; retries++) {
		val = GET_IATU_IN32(dw, idx, region_ctl2);
		if (val & PCIE_ATU_ENABLE)
			return;

		mdelay(LINK_WAIT_IATU);
	}
	printk("[ERR] Inbound iATU is not being enabled[ADDRESS_MODE]\n");
}

static inline void dx_iatu_inbound_bar(struct dw_edma *dw, u32 idx, u8 bar_no, u64 tgt_addr)
{
	u32 retries, val;

	SET_IATU_IN32(dw, idx, targ_lo_addr, lower_32_bits(tgt_addr));
	SET_IATU_IN32(dw, idx, targ_hi_addr, upper_32_bits(tgt_addr));

	SET_IATU_IN32(dw, idx, region_ctl2,
			PCIE_ATU_ENABLE |
			PCIE_ATU_BAR_MODE_ENABLE |
			(bar_no << 8));
	/*
	 * Make sure ATU enable takes effect before any subsequent config
	 * and I/O accesses.
	 */
	for (retries = 0; retries < LINK_WAIT_MAX_IATU_RETRIES; retries++) {
		val = GET_IATU_IN32(dw, idx, region_ctl2);
		if (val & PCIE_ATU_ENABLE)
			return;

		mdelay(LINK_WAIT_IATU);
	}
	printk("[ERR] Inbound iATU is not being enabled[BAR_MODE]\n");
}

void dw_iatu_config_inbound(struct dw_edma *dw, u8 mode, u64 base_addr, u64 tgt_addr, u32 size, u32 idx, u8 bar_no)
{
	dbg_init("iATU_IN#%d-mode:%d, b_addr:0x%llX, t_addr:0x%llX, size:0x%X, BAR:%d\n",
		idx, mode, base_addr, tgt_addr, size, bar_no);

	/* TODO - unroll check (CDM[0x900, 0xFFFFFFFF] -> Enable UNROLL) */
	if (idx >= IATU_BOUND_MAX) {
		printk("[ERROR] Exceed the number of iatu bound(%d/%d)\n", idx+1, IATU_BOUND_MAX);
		return;
	}

	switch (mode)
	{
		case DW_PCIE_IATU_BAR_MATCH:
			dx_iatu_inbound_bar(dw, idx, bar_no, tgt_addr);
			break;
		case DW_PCIE_IATU_ADDRESS_MATCH:
			dx_iatu_inbound_address(dw, idx, base_addr, tgt_addr, size);
			break;
		default:
			break;
	}
}

inline void dw_iatu_set_datas(struct dx_iatu_inbound *iatu_inb, u8 mode, u64 base_addr, u64 tgt_addr, u64 size, u32 idx, u8 bar_no)
{
	iatu_inb->mode		= mode;
	iatu_inb->base_addr	= base_addr;
	iatu_inb->tgt_addr	= tgt_addr;
	iatu_inb->size		= size;
	iatu_inb->idx		= idx;
	iatu_inb->bar_no	= bar_no;
}

/* Set Inbound Region as default */
void dw_iatu_default_config_set(struct dw_edma *dw)
{
	int i;

	/* configure for dma descriptor region */
	dw_iatu_set_datas(&dw->iatu_inb[IATU_INB_DMA_DESC],
		DW_PCIE_IATU_BAR_MATCH,
		0x00,              /*Dummy*/
		dw->dma_desc_base_addr,	/* Target Base address */
		0x00,              /*Dummy*/
		IATU_INB_DMA_DESC, /*Inbound-0*/
		dw->dma_desc_bar_num);

	/* configure for USER BAR 0 / 1 region */
	for (i = 0; i < dw->user_bar_cnt; i++) {
		dw_iatu_set_datas(&dw->iatu_inb[IATU_INB_USER0+i],
		DW_PCIE_IATU_BAR_MATCH,
		0x00,                     /*Dummy*/
		dw->npu_region[i].ep_addr,
		0x00,                     /*Dummy*/
		IATU_INB_USER0 + i,	      /*Inbound-1*/
		dw->npu_region[i].bar_num);
	}

	/* Write config iATU Register */
	for (i = 0; i < IATU_INB_MAX; i++) {
		if (dw->iatu_inb[i].mode)
			dw_iatu_config_inbound(dw,
				dw->iatu_inb[i].mode,
				dw->iatu_inb[i].base_addr,
				dw->iatu_inb[i].tgt_addr,
				dw->iatu_inb[i].size,
				dw->iatu_inb[i].idx,
				dw->iatu_inb[i].bar_no);
	}
}