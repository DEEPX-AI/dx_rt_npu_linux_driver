// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/msi.h>
#include <linux/pci.h>

#include "dw-edma-core.h"
#include "dw-edma-v0-core.h"
#include "dw-edma-v0-regs.h"
#include "dw-edma-v0-debugfs.h"

#include "dx_lib.h"
#include "dx_util.h"

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
	u64 value;

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

	switch (tmp) {
	case 0: /* CS=00: Idle — channel completed or was never started */
		return DMA_COMPLETE;
	case 1: /* CS=01: Running */
		dbg_tfr("[%s] status is progress\n", dma_chan_name(&chan->vc.chan));
		return DMA_IN_PROGRESS;
	case 3: /* CS=11: Stopped (normal completion) */
		return DMA_COMPLETE;
	case 2: /* CS=10: Halted on error */
	default:
		pr_err("[%s] status is error (CS=%u)\n",
		       dma_chan_name(&chan->vc.chan), tmp);
		return DMA_ERROR;
	}
}

/*
 * Read raw CS bits [6:5] from ch_control1.  Returns 0-3.
 * Useful for diagnostic logging without side-effects.
 */
u32 dw_edma_v0_core_ch_status_raw(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;

	return FIELD_GET(EDMA_V0_CH_STATUS_MASK,
			GET_CH_32(dw, chan->dir, chan->id, ch_control1));
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

u32 dw_edma_v0_core_ch_recover_abort(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	u32 ch_control, err_status = 0;

	ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);

	/* Not in error state — nothing to do */
	if ((ch_control & (BIT(5) | BIT(6))) != BIT(6))
		return 0;

	/* Read err_status to acknowledge the error (Read-Clear register) */
	if (chan->dir == EDMA_DIR_WRITE)
		err_status = GET_32(dw, wr_err_status);
	else
		err_status = GET_32(dw, rd_err_status.lsb);

	pr_info("CH%d: abort ack (ch_control1=0x%x, err_status=0x%x)\n",
		chan->id, ch_control, err_status);

	dw_edma_v0_core_clear_abort_int(chan);
	dw_edma_v0_core_clear_done_int(chan);

	return err_status;
}

/**
 * notify_peer_channels - Wake peer channels after engine_en cycle.
 *
 * engine_en=0->1 resets ALL channels on the same direction.
 * Peer channels that had active transfers will be silently killed.
 * Set hw_err and wake their waitqueues so their transfer threads
 * detect the failure immediately instead of waiting for timeout.
 */
static void notify_peer_channels(struct dw_edma_chan *initiator)
{
	struct dw_edma *dw = initiator->chip->dw;
	int i, start, end;

	if (initiator->dir == EDMA_DIR_WRITE) {
		start = 0;
		end = dw->wr_ch_cnt;
	} else {
		start = dw->wr_ch_cnt;
		end = dw->wr_ch_cnt + dw->rd_ch_cnt;
	}

	for (i = start; i < end; i++) {
		struct dw_edma_chan *peer = &dw->chan[i];

		if (peer == initiator)
			continue;

		if (peer->status == EDMA_ST_BUSY) {
			pr_warn("CH%d: peer channel killed by engine_en cycle (initiator CH%d)\n",
				peer->id, initiator->id);
			WRITE_ONCE(peer->hw_err, true);
			{
				wait_queue_head_t *wq = READ_ONCE(peer->transfer_wq);
				if (wq)
					wake_up_interruptible(wq);
			}
		}
	}
}

int dw_edma_v0_core_ch_soft_reset(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	u32 ch_control, err_status;
	unsigned long flags;

	ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);

	/* Not in error state — nothing to do */
	if ((ch_control & (BIT(5) | BIT(6))) != BIT(6))
		return 0;

	/* Acknowledge error (Read-Clear register) */
	if (chan->dir == EDMA_DIR_WRITE)
		err_status = GET_32(dw, wr_err_status);
	else
		err_status = GET_32(dw, rd_err_status.lsb);

	dw_edma_v0_core_clear_abort_int(chan);
	dw_edma_v0_core_clear_done_int(chan);

	pr_warn("CH%d: CS=2 (err_status=0x%x), engine_en cycle\n",
		chan->id, err_status);

	/*
	 * Serialize engine_en cycles per device.
	 * engine_en is a shared register per direction — concurrent
	 * engine_en cycles from two channels race and cancel each
	 * other's reset (Thread A: en=0, Thread B: en=0, A: en=1,
	 * B: en=0 → A's reset undone).  Hold engine_reset_lock
	 * across the entire cycle to prevent this.
	 */
	spin_lock_irqsave(&dw->engine_reset_lock, flags);

	/* Re-check under lock — another channel may have already reset */
	ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);
	if ((ch_control & (BIT(5) | BIT(6))) != BIT(6)) {
		spin_unlock_irqrestore(&dw->engine_reset_lock, flags);
		pr_info("CH%d: CS cleared by peer reset, skip\n", chan->id);
		return 0;
	}

	/* engine_en=0 → 1 resets DMA logic (datasheet Table 6-8) */
	SET_RW_32(dw, chan->dir, engine_en, EDMA_ENG_DIS);
	udelay(200);
	SET_RW_32(dw, chan->dir, engine_en, EDMA_ENG_EN);
	udelay(100);

	spin_unlock_irqrestore(&dw->engine_reset_lock, flags);

	/* Notify peer channels that were killed by engine_en cycle */
	notify_peer_channels(chan);

	/* Verify */
	ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);
	if (((ch_control >> 5) & 0x3) == 2) {
		pr_warn("CH%d: soft reset failed, CS still 2\n", chan->id);
		return -EIO;
	}

	return 0;
}

/**
 * dw_edma_v0_core_engine_cycle - Unconditional engine_en=0 -> 1 reset.
 *
 * Resets the DMA engine for the given direction regardless of channel
 * status.  Used as a last-resort cleanup when a graceful EDMA_REQ_STOP
 * timed out and the channel HW is still running.
 *
 * WARNING: This kills ALL channels on that direction (read or write).
 */
void dw_edma_v0_core_engine_cycle(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	unsigned long flags;

	pr_warn("CH%d %s: engine_en cycle (force reset)\n",
		chan->id, (chan->dir == EDMA_DIR_WRITE) ? "WR" : "RD");

	spin_lock_irqsave(&dw->engine_reset_lock, flags);
	SET_RW_32(dw, chan->dir, engine_en, 0);
	udelay(200);
	SET_RW_32(dw, chan->dir, engine_en, BIT(0));
	udelay(100);
	spin_unlock_irqrestore(&dw->engine_reset_lock, flags);

	/* Notify peer channels killed by engine_en cycle */
	notify_peer_channels(chan);
}

/*
 * PCIe Secondary Bus Reset — PROCESS CONTEXT ONLY.
 *
 * Resets the ENTIRE PCIe endpoint by toggling the Secondary Bus Reset
 * bit on the upstream bridge.  This is electrically equivalent to
 * asserting PERST# and resets ALL HW state including DW eDMA.
 *
 * IMPORTANT: ALL DMA channels must be quiesced (dmaengine_terminate_all)
 * BEFORE calling this function.  SBR kills all in-flight transfers on
 * ALL channels.
 *
 * After SBR:
 *   - All eDMA channels are in CS=3 (Stopped, power-on default)
 *   - DMA engines, channel power, iATU are re-initialized
 *   - PCI config space + MSI-X restored from saved state
 *   - New transfers can be started normally
 *
 * Return: 0 on success, -EIO on failure.
 */
int dw_edma_v0_core_pcie_reset(struct dw_edma *dw)
{
	struct pci_dev *pdev;
	struct pci_dev *bridge;
	u16 bridge_ctrl;
	struct msi_msg saved_msi = {0};
	int i;

	pdev = dw->pdev;
	if (!pdev) {
		pr_err("[pcie_reset] no pci_dev\n");
		return -EIO;
	}

	bridge = pdev->bus->self;
	if (!bridge) {
		pr_err("[pcie_reset] no upstream bridge\n");
		return -EIO;
	}

	pr_info("[pcie_reset] performing PCIe Secondary Bus Reset\n");

	/*
	 * Step 0: Save MSI from PCI config space BEFORE SBR.
	 *
	 * This is the ground truth — the actual MSI address/data that the
	 * APIC/IOMMU programmed at IRQ activation time.  We must save it
	 * now because:
	 *
	 *   (a) pci_restore_msi_state() (called inside pci_restore_state)
	 *       writes msi_desc->msg back to PCI config, but on some
	 *       kernels/platforms msi_desc->msg.data is 0 (kernel bug or
	 *       IOMMU remapping artifact).
	 *
	 *   (b) get_cached_msi_msg() reads msi_desc->msg which has the
	 *       same problem — data=0.
	 *
	 * By reading PCI config directly before SBR, we capture the
	 * working MSI values and can restore them reliably after SBR.
	 */
	if (dx_pci_read_msi_msg(pdev, &saved_msi) != 0) {
		/* Fallback: use driver's cached copy from probe/last-known-good */
		if (dw->nr_irqs > 0)
			memcpy(&saved_msi, &dw->irq[0].msi, sizeof(saved_msi));
	}
	pr_info("[pcie_reset] saving MSI before SBR: addr=0x%x_%x data=0x%x\n",
		saved_msi.address_hi, saved_msi.address_lo, saved_msi.data);

	/* Save PCI state (config space + MSI table entries) */
	pci_save_state(pdev);

	/* Assert Secondary Bus Reset on parent bridge */
	pci_read_config_word(bridge, PCI_BRIDGE_CONTROL, &bridge_ctrl);
	pci_write_config_word(bridge, PCI_BRIDGE_CONTROL,
			      bridge_ctrl | PCI_BRIDGE_CTL_BUS_RESET);

	/* Hold reset for 2ms (PCIe spec minimum) */
	msleep(2);

	/* De-assert reset */
	pci_write_config_word(bridge, PCI_BRIDGE_CONTROL,
			      bridge_ctrl & ~PCI_BRIDGE_CTL_BUS_RESET);

	/* Wait for link re-training and device ready (500ms conservative) */
	msleep(500);

	/* Restore PCI state + re-enable bus mastering.
	 * NOTE: pci_restore_msi_state() inside pci_restore_state() may
	 * write msi_desc->msg.data=0 to PCI config — we fix this below. */
	pci_restore_state(pdev);
	pci_set_master(pdev);

	/*
	 * Full eDMA re-initialization — mirrors the probe sequence:
	 *   1. off()  : mask/clear interrupts, engines off
	 *   2. device_config: IMWR (MSI interrupt routing) per channel
	 *   3. engine_en + ch_pwr_en
	 *
	 * SBR resets ALL eDMA registers to power-on defaults (zero).
	 * pci_restore_state restores PCI config space (incl. MSI cap),
	 * but MMIO-mapped eDMA registers (IMWR, int_mask, engine_en,
	 * ch_pwr_en, etc.) must be re-programmed explicitly.
	 */

	/*
	 * Restore MSI: Write the pre-SBR MSI message to PCI config and
	 * update all driver-cached copies.  This fixes the case where
	 * pci_restore_msi_state wrote data=0 from a stale msi_desc->msg.
	 *
	 * NOTE: On platforms with Interrupt Remapping (Intel VT-d / AMD-Vi),
	 * MSI data=0x0 in PCI config is NORMAL â the actual vector delivery
	 * is handled by the IR table in DRAM, not the PCI MSI data field.
	 * The IMWR registers also use data=0 in this case, which is correct.
	 */
	if (saved_msi.data != 0) {
		dx_pci_write_msi_msg(pdev, &saved_msi);
	} else {
		/*
		 * Even the pre-SBR PCI config had data=0 — read from
		 * kernel's IRQ framework as last resort, and if that also
		 * fails, the driver's probe-time cache is already in
		 * dw->irq[0].msi.
		 */
		int irq = pci_irq_vector(pdev, 0);

		if (irq >= 0 && irq_get_msi_desc(irq)) {
			get_cached_msi_msg(irq, &saved_msi);
			pr_info("[pcie_reset] pre-SBR MSI data was 0, "
				"kernel cached: data=0x%x\n", saved_msi.data);
		}
		if (saved_msi.data == 0 && dw->nr_irqs > 0 &&
		    dw->irq[0].msi.data != 0) {
			memcpy(&saved_msi, &dw->irq[0].msi,
			       sizeof(saved_msi));
			pr_info("[pcie_reset] using probe-time MSI: "
				"addr=0x%x_%x data=0x%x\n",
				saved_msi.address_hi,
				saved_msi.address_lo, saved_msi.data);
		}
		if (saved_msi.data != 0)
			dx_pci_write_msi_msg(pdev, &saved_msi);
		else
			pr_info("[pcie_reset] MSI data=0 on all sources — "
				"normal if Interrupt Remapping is active\n");
	}

	/* Update driver's cached MSI for all IRQs and channels */
	for (i = 0; i < dw->nr_irqs; i++)
		memcpy(&dw->irq[i].msi, &saved_msi, sizeof(saved_msi));
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++)
		memcpy(&dw->chan[i].msi, &saved_msi, sizeof(saved_msi));

	/* Step 1: Clean slate — same as dw_edma_v0_core_off() */
	SET_BOTH_32(dw, int_mask,
		    EDMA_V0_DONE_INT_MASK | EDMA_V0_ABORT_INT_MASK);
	SET_BOTH_32(dw, int_clear,
		    EDMA_V0_DONE_INT_MASK | EDMA_V0_ABORT_INT_MASK);

	/* Step 2: Re-enable channel power (all channels, both directions).
	 * ch_pwr_en gates the channel clock — must be ON before IMWR writes
	 * so that per-channel registers are accessible. */
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch0_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch0_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch1_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch1_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch2_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch2_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_READ, ch3_pwr_en, EDMA_ENG_EN);
	SET_RW_COMPAT(dw, EDMA_DIR_WRITE, ch3_pwr_en, EDMA_ENG_EN);

	/* Step 3: Enable DMA engines BEFORE IMWR programming.
	 * Some HW revisions require engine_en=1 for per-channel register
	 * writes (IMWR, ch_control1) to take effect.  Enable both
	 * directions explicitly with readback verification. */
	SET_32(dw, wr_engine_en, EDMA_ENG_EN);
	udelay(100);
	SET_32(dw, rd_engine_en, EDMA_ENG_EN);
	udelay(100);

	/* Readback verify — retry if rd_engine_en didn't stick */
	if (!GET_32(dw, rd_engine_en)) {
		pr_warn("[pcie_reset] rd_engine_en=0 after first write, retrying\n");
		SET_32(dw, rd_engine_en, EDMA_ENG_EN);
		udelay(200);
		if (!GET_32(dw, rd_engine_en))
			pr_err("[pcie_reset] rd_engine_en STILL 0 after retry!\n");
	}

	/* Step 4: Re-program IMWR (MSI interrupt routing) for ALL channels.
	 * chan->msi was updated in Step 0 with the correct kernel-cached
	 * MSI data (address + vector).  Without this, DMA completion/abort
	 * generates no MSI interrupt because IMWR registers are zeroed
	 * by SBR. */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++)
		dw_edma_v0_core_device_config(&dw->chan[i]);

	/* Step 5: Re-configure iATU inbound mappings from saved config */
	dw_iatu_default_config_set(dw);

	/* Clear SW flags on all channels */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		dw->chan[i].hw_err = false;
		dw->chan[i].aborted = false;
	}

	/* Step 6: Unmask interrupts */
	SET_BOTH_32(dw, int_mask, 0);

	/* Diagnostic: verify final state */
	{
		struct msi_msg pci_msi = {0};
		u32 wr_imwr_data = GET_RW_32(dw, EDMA_DIR_WRITE, ch01_imwr_data);
		u32 rd_imwr_data = GET_RW_32(dw, EDMA_DIR_READ, ch01_imwr_data);

		dx_pci_read_msi_msg(pdev, &pci_msi);

		pr_info("[pcie_reset] done: wr_eng=0x%x rd_eng=0x%x "
			"wr_imwr_data=0x%x rd_imwr_data=0x%x "
			"PCI_MSI_data=0x%x int_mask=0x%x/0x%x\n",
			GET_32(dw, wr_engine_en), GET_32(dw, rd_engine_en),
			wr_imwr_data, rd_imwr_data,
			pci_msi.data,
			GET_RW_32(dw, EDMA_DIR_WRITE, int_mask),
			GET_RW_32(dw, EDMA_DIR_READ, int_mask));
	}

	return 0;
}

static int dw_edma_v0_core_wait_channel_idle(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	u32 ch_control;
	unsigned long start_jiffies, elapsed_jiffies, timeout_jiffies;
	unsigned int elapsed_us;
	int retry_count;
	bool status_changed;

	ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);

	/* CS=2 (Fatal Error) */
	if ((ch_control & (BIT(5) | BIT(6))) == BIT(6)) {
		pr_warn("CH%d: channel in error state (CS=2) at transfer start\n",
			chan->id);
		return -EIO;
	}

	/* CS=3 (Stopped): normal state after SBR or power-on */
	if ((ch_control & (BIT(5) | BIT(6))) == (BIT(5) | BIT(6))) {
		dbg_tfr("CH%d: channel in Stopped state (CS=3)\n", chan->id);
		return 0;
	}

	if ((ch_control & (BIT(5) | BIT(6))) == BIT(5)) {
		dbg_tfr("Channel %d is busy[%llx, %llx]\n", chan->id,
			GET_CH_32(dw, chan->dir, chan->id, sar.lsb) |
			((u64)GET_CH_32(dw, chan->dir, chan->id, sar.msb) << 32),
			GET_CH_32(dw, chan->dir, chan->id, dar.lsb) |
			((u64)GET_CH_32(dw, chan->dir, chan->id, dar.msb) << 32));

		start_jiffies = jiffies;
		timeout_jiffies = start_jiffies + usecs_to_jiffies(5000);
		retry_count = 0;
		status_changed = false;

		while (time_before(jiffies, timeout_jiffies)) {
			retry_count++;
			ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);

			if ((ch_control & (BIT(5) | BIT(6))) != BIT(5)) {
				status_changed = true;
				break;
			}
			if (retry_count % 20 == 0)
				cond_resched();
			udelay(10);
		}

		elapsed_jiffies = jiffies - start_jiffies;
		elapsed_us = jiffies_to_usecs(elapsed_jiffies);

		if (status_changed) {
			u32 new_cs = (ch_control >> 5) & 0x3;

			dbg_tfr("CH%d status changed to CS=%u (0x%x) after %u us (%d retries)\n",
				chan->id, new_cs, ch_control, elapsed_us, retry_count);

			if (new_cs == 2)
				dw_edma_v0_core_ch_recover_abort(chan);
		} else {
			pr_warn("CH%d status still busy after %u us (%d retries)\n",
				chan->id, elapsed_us, retry_count);
		}
	}

	return 0;
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

	ret = dw_edma_v0_core_wait_channel_idle(chan);
	if (ret) {
		/*
		 * Channel is stuck in error state (CS=2) and HW rejected
		 * the engine soft reset.  Do NOT issue doorbell — it will
		 * be ignored, causing a timeout hang.  Mark channel so the
		 * DMA thread can detect it and fail immediately.
		 */
		pr_err("CH%d: skipping doorbell — channel in irrecoverable error\n",
		       chan->id);
		WRITE_ONCE(chan->hw_err, true);
		return;
	}
	if (is_llm) {
		dw_edma_v0_core_write_chunk(chunk, dw->idx, chan->id, chan->dir);
		
		/* Sync for device if using Buddy Allocator (Streaming DMA).
		 * Only sync the actual descriptor data size, not the entire
		 * 1MB host_region, to avoid excessive cache flush overhead. */
		if (chunk->is_buddy) {
			size_t desc_sz = (chunk->bursts_alloc + 1) * EDMA_LL_SZ;
			/* Include the LLP terminator entry (sizeof lli + llp) */
			desc_sz += sizeof(struct dw_edma_v0_llp);
			if (desc_sz > chunk->host_region.sz)
				desc_sz = chunk->host_region.sz;
			dma_sync_single_for_device(chan->chip->dev,
						   chunk->host_region.paddr,
						   desc_sz,
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