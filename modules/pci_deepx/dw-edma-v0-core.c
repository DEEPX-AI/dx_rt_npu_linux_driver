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
#include <linux/moduleparam.h>
#include <linux/msi.h>
#include <linux/pci.h>

static inline bool dx_pcie_has_flr(struct pci_dev *dev)
{
	u32 cap;

	if (!pci_is_pcie(dev))
		return false;
	if (dev->dev_flags & PCI_DEV_FLAGS_NO_FLR_RESET)
		return false;
	pcie_capability_read_dword(dev, PCI_EXP_DEVCAP, &cap);
	return !!(cap & PCI_EXP_DEVCAP_FLR);
}

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

static bool dx_dma_mmio_poll_allowed(struct dw_edma *dw);
static bool dx_dma_shadow_precopy_allowed(struct dw_edma *dw);

static bool dx_dma_msi_msg_valid(const struct msi_msg *msg)
{
	/* MSI data can legitimately be zero on interrupt-remapped systems.
	 * Treat only an all-zero address as invalid. */
	return msg && (msg->address_lo || msg->address_hi);
}

static bool dx_dma_get_kernel_cached_msi(struct pci_dev *pdev,
						 struct msi_msg *msg)
{
	int irq;

	if (!pdev || !msg)
		return false;

	irq = pci_irq_vector(pdev, 0);
	if (irq < 0 || !irq_get_msi_desc(irq))
		return false;

	memset(msg, 0, sizeof(*msg));
	get_cached_msi_msg(irq, msg);
	return dx_dma_msi_msg_valid(msg);
}

#define DX_DMA_HELPER_ACQUIRE_TIMEOUT_US	50000
#define DX_DMA_HELPER_ACQUIRE_ATOMIC_TIMEOUT_US	1000
#define DX_DMA_HELPER_ACQUIRE_STEP_US	10

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

int dw_edma_v0_core_ch_status_checked(struct dw_edma_chan *chan, u32 *cs)
{
	struct dw_edma *dw = chan->chip->dw;
	u32 control1;

	control1 = GET_CH_32(dw, chan->dir, chan->id, ch_control1);
	if (control1 == (u32)~0u)
		return -ENODEV;

	*cs = FIELD_GET(EDMA_V0_CH_STATUS_MASK, control1);
	return 0;
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

static void dw_edma_v0_core_mask_done_int(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	u32 tmp;

	tmp = GET_RW_32(dw, chan->dir, int_mask);
	tmp |= FIELD_PREP(EDMA_V0_DONE_INT_MASK, BIT(chan->id));
	SET_RW_32(dw, chan->dir, int_mask, tmp);
}

static void dw_edma_v0_core_unmask_done_abort_int(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	u32 tmp;

	tmp = GET_RW_32(dw, chan->dir, int_mask);
	tmp &= ~FIELD_PREP(EDMA_V0_DONE_INT_MASK, BIT(chan->id));
	tmp &= ~FIELD_PREP(EDMA_V0_ABORT_INT_MASK, BIT(chan->id));
	SET_RW_32(dw, chan->dir, int_mask, tmp);
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

/*
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
					wake_up(wq);
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

/*
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

/*
 * edma_reprogram_hw - Reprogram eDMA registers from scratch
 *
 * Common helper called after any reset (SBR or link recovery).
 * Programs: int_mask/clear, ch_pwr_en, engine_en, IMWR, iATU, unmask.
 * Caller must ensure PCI config (BARs, MSI) is already restored and
 * chan->msi is populated with correct MSI data.
 */
static void edma_reprogram_hw(struct dw_edma *dw)
{
	int i;

	/* Clean slate — mask and clear all pending interrupts */
	SET_BOTH_32(dw, int_mask,
		    EDMA_V0_DONE_INT_MASK | EDMA_V0_ABORT_INT_MASK);
	SET_BOTH_32(dw, int_clear,
		    EDMA_V0_DONE_INT_MASK | EDMA_V0_ABORT_INT_MASK);

	/* Re-enable channel power (all channels, both directions).
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

	/* Enable DMA engines BEFORE IMWR programming.
	 * Some HW revisions require engine_en=1 for per-channel register
	 * writes (IMWR, ch_control1) to take effect. */
	SET_32(dw, wr_engine_en, EDMA_ENG_EN);
	udelay(100);
	SET_32(dw, rd_engine_en, EDMA_ENG_EN);
	udelay(100);

	/* Readback verify — retry if rd_engine_en didn't stick */
	if (!GET_32(dw, rd_engine_en)) {
		pr_warn("[edma_restore] rd_engine_en=0 after first write, retrying\n");
		SET_32(dw, rd_engine_en, EDMA_ENG_EN);
		udelay(200);
		if (!GET_32(dw, rd_engine_en))
			pr_err("[edma_restore] rd_engine_en STILL 0 after retry!\n");
	}

	/* Re-program IMWR (MSI interrupt routing) for ALL channels.
	 * Without this, DMA completion/abort generates no MSI interrupt. */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++)
		dw_edma_v0_core_device_config(&dw->chan[i]);

	/* Re-configure iATU inbound mappings from saved config */
	dw_iatu_default_config_set(dw);

	/* Clear SW flags on all channels */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		dw->chan[i].hw_err = false;
		dw->chan[i].aborted = false;
		/*
		 * Reset SW channel state to IDLE.  The engine has just been
		 * restored, so no transfer is actually running; a channel left
		 * EDMA_ST_BUSY by an aborted in-flight transfer would otherwise
		 * be rejected by dw_edma_alloc_chan_resources() (-EBUSY) on the
		 * next dma_request_channel(), blocking reopen permanently.
		 */
		dw->chan[i].status = EDMA_ST_IDLE;
	}

	/* Unmask interrupts */
	SET_BOTH_32(dw, int_mask, 0);
}

/*
 * dw_edma_v0_core_edma_restore - Restore eDMA after link recovery (no SBR)
 *
 * Used by health worker recovery when the link comes back on its own
 * (after external reset, hot-plug, etc.).  Does NOT issue SBR — the EP
 * is already out of reset and the link is trained.
 *
 * Sequence: PCI config restore → MSI write → eDMA reprogram.
 * Requires pci_save_state() to have been called at probe or before
 * the link went down.
 *
 * Return: 0 on success, negative errno on failure.
 */
int dw_edma_v0_core_edma_restore(struct dw_edma *dw)
{
	struct pci_dev *pdev = dw->pdev;
	struct msi_msg cached_msi = {0};
	unsigned long timeout;
	u32 test_val;
	int i;

	if (!pdev)
		return -EIO;

	pr_info("[edma_restore] restoring PCI config + eDMA (no SBR)\n");

	/* Restore PCI config space (BARs, command, MSI cap) from saved state */
	pci_restore_state(pdev);
	pci_set_master(pdev);

	/*
	 * Wait for BAR MMIO to become accessible.
	 * After external SBR, the EP's internal bus fabric may still be
	 * initializing even though the PCIe link has re-trained.
	 * Poll a known eDMA register — 0xFFFFFFFF means BAR is not
	 * decoded yet.
	 */
	timeout = jiffies + msecs_to_jiffies(2000);
	do {
		test_val = GET_32(dw, wr_engine_en);
		if (test_val != (u32)~0u)
			break;
		msleep(50);
	} while (time_before(jiffies, timeout));

	if (test_val == (u32)~0u) {
		pr_err("[edma_restore] BAR MMIO inaccessible after 2s, "
		       "aborting restore\n");
		return -EIO;
	}

	/* PCI config MSI may be stale after EP reset. Prefer the kernel IRQ
	 * cache over the driver's older last-known-good cache, but never
	 * reprogram IMWR with an all-zero address. MSI data may legitimately
	 * be zero on interrupt-remapped hosts. */
	if (dx_dma_get_kernel_cached_msi(pdev, &cached_msi)) {
		pr_warn("[edma_restore] using kernel cached MSI addr=0x%x_%x data=0x%x\n",
			cached_msi.address_hi, cached_msi.address_lo,
			cached_msi.data);
	} else if (dw->nr_irqs > 0 && dx_dma_msi_msg_valid(&dw->irq[0].msi)) {
		memcpy(&cached_msi, &dw->irq[0].msi, sizeof(cached_msi));
	}

	if (!dx_dma_msi_msg_valid(&cached_msi)) {
		pr_err("[edma_restore] no valid cached MSI; aborting restore to avoid all-zero IMWR address\n");
		return -EIO;
	}

	dx_pci_write_msi_msg(pdev, &cached_msi);

	/* Update channel MSI caches */
	for (i = 0; i < dw->nr_irqs; i++)
		memcpy(&dw->irq[i].msi, &cached_msi,
		       sizeof(cached_msi));
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++)
		memcpy(&dw->chan[i].msi, &cached_msi,
		       sizeof(cached_msi));

	/* Reprogram all eDMA registers */
	edma_reprogram_hw(dw);

	/*
	 * Verify engine_en after programming.
	 * If writes didn't stick, EP may need more time. Retry once
	 * with extended delay.
	 */
	{
		u32 wr = GET_32(dw, wr_engine_en);
		u32 rd = GET_32(dw, rd_engine_en);

		if (wr == (u32)~0u || rd == (u32)~0u) {
			pr_err("[edma_restore] MMIO dead after reprogram "
			       "(wr=0x%x rd=0x%x)\n", wr, rd);
			return -EIO;
		}

		if (!wr || !rd) {
			pr_warn("[edma_restore] engine_en not active "
				"(wr=0x%x rd=0x%x), retrying in 500ms\n",
				wr, rd);
			msleep(500);

			SET_32(dw, wr_engine_en, EDMA_ENG_EN);
			udelay(100);
			SET_32(dw, rd_engine_en, EDMA_ENG_EN);
			msleep(100);

			wr = GET_32(dw, wr_engine_en);
			rd = GET_32(dw, rd_engine_en);

			if (wr == (u32)~0u || rd == (u32)~0u) {
				pr_err("[edma_restore] MMIO died during "
				       "retry (wr=0x%x rd=0x%x)\n", wr, rd);
				return -EIO;
			}
			if (!wr || !rd) {
				pr_err("[edma_restore] engine_en STILL "
				       "not active (wr=0x%x rd=0x%x)\n",
				       wr, rd);
				return -EIO;
			}
			pr_info("[edma_restore] engine_en recovered "
				"after retry (wr=0x%x rd=0x%x)\n", wr, rd);
		}
	}

	/* Diagnostic */
	{
		struct msi_msg pci_msi = {0};

		dx_pci_read_msi_msg(pdev, &pci_msi);
		pr_info("[edma_restore] done: wr_eng=0x%x rd_eng=0x%x "
			"PCI_MSI_data=0x%x int_mask=0x%x/0x%x\n",
			GET_32(dw, wr_engine_en), GET_32(dw, rd_engine_en),
			pci_msi.data,
			GET_RW_32(dw, EDMA_DIR_WRITE, int_mask),
			GET_RW_32(dw, EDMA_DIR_READ, int_mask));
	}

	/*
	 * Re-save PCI state for the NEXT recovery.
	 * pci_restore_state() above consumed the saved state (the kernel
	 * sets dev->state_saved = false after restore).  Without this
	 * re-save, a subsequent pci_restore_state() is a no-op and the
	 * next link-flap recovery will see 0xFFFFFFFF on MMIO.
	 */
	pci_save_state(pdev);

	return 0;
}

/*
 * dx_edma_host_supports_sbr - Can the upstream host controller perform a
 * meaningful PCIe Secondary Bus Reset on this endpoint?
 *
 * SBR toggles PCI_BRIDGE_CTL_BUS_RESET on the upstream bridge, which is
 * electrically equivalent to asserting PERST#.  This is only useful if the
 * host controller (root complex / root port) actually re-trains the link
 * after the reset is de-asserted.  Some ARM root complexes never do, and a
 * topology with no software-visible upstream bridge has nothing to toggle.
 * On those hosts SBR is meaningless: it just leaves the link down and the
 * endpoint unreachable.  Detect that here so the caller can fall back to a
 * link-preserving Function Level Reset instead.
 *
 * Return: true only when a secondary bus reset is viable on this host.
 */
static bool dx_edma_host_supports_sbr(struct pci_dev *pdev)
{
	struct pci_dev *bridge;

	if (!pdev || !pdev->bus)
		return false;

	bridge = pdev->bus->self;

	/* No upstream bridge to toggle the Secondary Bus Reset bit on. */
	if (!bridge)
		return false;

	/* Bridge / root-port explicitly marked as not supporting bus reset
	 * (kernel quirk or platform flag). */
	if (bridge->dev_flags & PCI_DEV_FLAGS_NO_BUS_RESET)
		return false;

	/* Kernel topology probe: a non-zero return means a bus reset is not
	 * possible for this bus. */
	if (pci_probe_reset_bus(pdev->bus) != 0)
		return false;

	return true;
}

/*
 * SBR (Secondary Bus Reset) policy.
 *
 * Driver-initiated SBR toggles PCI_BRIDGE_CTL_BUS_RESET on the upstream bridge
 * and then restores the endpoint's saved PCI config state (BARs, command, MSI)
 * with pci_restore_state().  It is not a PCI remove/rescan and does not ask the
 * kernel to re-assign BAR resources.  That distinction is important on hosts
 * whose bridge windows cannot grow at runtime: remove/rescan can leave BAR0
 * "<unassigned>", but the driver SBR path preserves the saved BAR value.
 *
 *   -1 = auto (default): allow SBR when the host topology reports that a bus
 *        reset is supported.
 *    0 = never use SBR (FLR-only).
 *    1 = explicitly allow SBR when the host topology supports it.
 */
static int allow_sbr = -1;
module_param(allow_sbr, int, 0644);
MODULE_PARM_DESC(allow_sbr,
	"PCIe Secondary Bus Reset policy: -1=auto (host-topology based), 0=FLR-only, 1=allow when host supports it");

/*
 * dx_edma_sbr_viable - Is a Secondary Bus Reset both possible AND safe on
 * this endpoint?
 *
 * Combines the host-topology check (dx_edma_host_supports_sbr) with the
 * allow_sbr policy.  Use this — not dx_edma_host_supports_sbr — at every
 * point that may escalate to SBR.
 */
static bool dx_edma_sbr_viable(struct pci_dev *pdev)
{
	switch (allow_sbr) {
	case 0:
		return false;
	case 1:
		break;
	default:
		break;
	}

	return dx_edma_host_supports_sbr(pdev);
}

/*
 * dx_edma_select_reset_method - Pick the softest reset that works.
 *
 * FLR is preferred (resets only this function, keeps the PCIe link up).
 * SBR is the fallback when the device has no FLR.  Returns 0 and sets
 * *use_sbr, or a negative errno when neither reset is possible.
 */
static int dx_edma_select_reset_method(struct pci_dev *pdev, bool *use_sbr)
{
	if (dx_pcie_has_flr(pdev)) {
		*use_sbr = false;
	} else if (dx_edma_sbr_viable(pdev)) {
		*use_sbr = true;
	} else {
		pr_err("[pcie_reset] device has no FLR and host cannot perform a safe SBR; cannot reset endpoint\n");
		return -ENOTTY;
	}
	return 0;
}

/*
 * dx_edma_save_msi_before_reset - Snapshot the MSI message before the reset
 * clears it.  PCI config is the ground truth when enabled; fall back to the
 * driver's cached copy.  MSI data may be zero on interrupt-remapped hosts, so
 * callers validate by address, not data.
 */
static void dx_edma_save_msi_before_reset(struct dw_edma *dw,
					  struct msi_msg *saved_msi)
{
	struct pci_dev *pdev = dw->pdev;

	if (dx_pci_read_msi_msg(pdev, saved_msi) != 0) {
		if (dw->nr_irqs > 0)
			memcpy(saved_msi, &dw->irq[0].msi, sizeof(*saved_msi));
	}
	pr_debug("[pcie_reset] saving MSI before SBR: addr=0x%x_%x data=0x%x\n",
		saved_msi->address_hi, saved_msi->address_lo, saved_msi->data);
}

/*
 * dx_edma_do_sbr - Toggle Secondary Bus Reset on the upstream bridge, then
 * wait until the endpoint's config space responds again.
 *
 * pci_restore_state() (run by the caller after this) must execute against a
 * live config space.  For some endpoints (e.g. the first port behind a
 * switch) the link re-trains but config space stays unresponsive (Vendor ID
 * reads 0xFFFF) longer than the fixed settle delay.  If the restore runs
 * against a dead config space the BAR/command restore is silently lost, yet
 * the kernel still clears dev->state_saved — the follow-up health-worker
 * edma_restore() then finds no saved state to restore (no-op), BAR MMIO stays
 * 0xFFFFFFFF, and recovery loops until rate-limited.  The readiness poll here
 * makes the restore effective.
 */
static void dx_edma_do_sbr(struct pci_dev *pdev, struct pci_dev *bridge)
{
	u16 bridge_ctrl;
	u16 vid = 0xffff;
	unsigned long to;

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

	/* Poll until config space is accessible before pci_restore_state(). */
	to = jiffies + msecs_to_jiffies(2000);
	do {
		pci_read_config_word(pdev, PCI_VENDOR_ID, &vid);
		if (vid != 0xffff && vid != 0x0000)
			break;
		msleep(20);
	} while (time_before(jiffies, to));

	if (vid == 0xffff || vid == 0x0000)
		pr_warn("[pcie_reset] config space still unresponsive after SBR (vid=0x%04x); restore may be ineffective\n",
			vid);
}

/*
 * dx_edma_restore_msi_after_reset - Re-program a validated MSI message to PCI
 * config + channel IMWR after a reset and refresh the driver MSI caches.
 *
 * pci_restore_msi_state() may restore an empty MSI address after reset, so a
 * validated source (pre-reset snapshot → kernel IRQ cache → probe-time cache)
 * is re-written here.  Returns -EIO if no source has a non-zero address
 * (programming an all-zero IMWR address would silently drop MSIs).
 */
static int dx_edma_restore_msi_after_reset(struct dw_edma *dw,
					   struct msi_msg *saved_msi)
{
	struct pci_dev *pdev = dw->pdev;
	int i;

	if (!dx_dma_msi_msg_valid(saved_msi)) {
		/* Pre-reset MSI address invalid: try kernel IRQ cache, then
		 * probe-time cache. */
		struct msi_msg kernel_msi = {0};

		if (dx_dma_get_kernel_cached_msi(pdev, &kernel_msi)) {
			memcpy(saved_msi, &kernel_msi, sizeof(*saved_msi));
			pr_debug("[pcie_reset] using kernel cached MSI: addr=0x%x_%x data=0x%x\n",
				saved_msi->address_hi, saved_msi->address_lo,
				saved_msi->data);
		}
		if (!dx_dma_msi_msg_valid(saved_msi) && dw->nr_irqs > 0 &&
		    dx_dma_msi_msg_valid(&dw->irq[0].msi)) {
			memcpy(saved_msi, &dw->irq[0].msi, sizeof(*saved_msi));
			pr_debug("[pcie_reset] using probe-time MSI: addr=0x%x_%x data=0x%x\n",
				saved_msi->address_hi, saved_msi->address_lo,
				saved_msi->data);
		}
		if (!dx_dma_msi_msg_valid(saved_msi)) {
			pr_err("[pcie_reset] no valid MSI source after SBR; refusing to program all-zero IMWR address\n");
			return -EIO;
		}
	}

	dx_pci_write_msi_msg(pdev, saved_msi);

	/* Update driver's cached MSI for all IRQs and channels */
	for (i = 0; i < dw->nr_irqs; i++)
		memcpy(&dw->irq[i].msi, saved_msi, sizeof(*saved_msi));
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++)
		memcpy(&dw->chan[i].msi, saved_msi, sizeof(*saved_msi));

	return 0;
}

/* dx_edma_pcie_reset_diag - Dump final eDMA/MSI state after a reset. */
static void dx_edma_pcie_reset_diag(struct dw_edma *dw, struct pci_dev *pdev)
{
	struct msi_msg pci_msi = {0};
	u32 wr_imwr_data = GET_RW_32(dw, EDMA_DIR_WRITE, ch01_imwr_data);
	u32 rd_imwr_data = GET_RW_32(dw, EDMA_DIR_READ, ch01_imwr_data);

	dx_pci_read_msi_msg(pdev, &pci_msi);

	pr_debug("[pcie_reset] done: wr_eng=0x%x rd_eng=0x%x "
		"wr_imwr_data=0x%x rd_imwr_data=0x%x "
		"PCI_MSI_data=0x%x int_mask=0x%x/0x%x\n",
		GET_32(dw, wr_engine_en), GET_32(dw, rd_engine_en),
		wr_imwr_data, rd_imwr_data,
		pci_msi.data,
		GET_RW_32(dw, EDMA_DIR_WRITE, int_mask),
		GET_RW_32(dw, EDMA_DIR_READ, int_mask));
}

/*
 * dx_edma_flr_channels_stuck - After an FLR, return true if any channel is
 * still latched in CS=1/2.  FLR keeps the EP firmware alive, so a
 * firmware-latched channel error may survive it; only a full endpoint reset
 * (SBR / PERST# equivalent) restarts the firmware and clears the latch.
 */
static bool dx_edma_flr_channels_stuck(struct dw_edma *dw)
{
	int i;

	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		u32 cs = dw_edma_v0_core_ch_status_raw(&dw->chan[i]);

		if (cs == 1 || cs == 2)
			return true;
	}
	return false;
}

int dw_edma_v0_core_pcie_reset(struct dw_edma *dw)
{
	struct pci_dev *pdev;
	struct pci_dev *bridge;
	struct msi_msg saved_msi = {0};
	bool use_sbr;
	int ret;

	pdev = dw->pdev;
	if (!pdev) {
		pr_err("[pcie_reset] no pci_dev\n");
		return -EIO;
	}

	bridge = pdev->bus ? pdev->bus->self : NULL;

	ret = dx_edma_select_reset_method(pdev, &use_sbr);
	if (ret)
		return ret;

	pr_info("[pcie_reset] performing %s\n",
		use_sbr ? "PCIe Secondary Bus Reset (device has no FLR)"
			: "Function Level Reset");

	/* Snapshot MSI + PCI config before the reset clears them. */
	dx_edma_save_msi_before_reset(dw, &saved_msi);
	pci_save_state(pdev);

reset_again:
	if (use_sbr) {
		dx_edma_do_sbr(pdev, bridge);
	} else {
		/*
		 * Function Level Reset — resets the endpoint function without
		 * bringing the PCIe link down.  pcie_flr() waits for the
		 * function to become ready before returning.
		 */
		int flr_ret = pcie_flr(pdev);

		if (flr_ret) {
			pr_err("[pcie_reset] FLR failed (%d)\n", flr_ret);
			/*
			 * Fall back to SBR when the host can perform one.  Keep
			 * the saved PCI state intact (do NOT restore here) so
			 * the post-SBR pci_restore_state() below can reprogram
			 * config space.
			 */
			if (dx_edma_sbr_viable(pdev)) {
				pr_warn("[pcie_reset] FLR failed; falling back to Secondary Bus Reset\n");
				use_sbr = true;
				goto reset_again;
			}
			pr_err("[pcie_reset] FLR failed and no safe SBR available; cannot reset endpoint\n");
			pci_restore_state(pdev);
			return flr_ret;
		}
	}

	/* Restore PCI state + re-enable bus mastering. */
	pci_restore_state(pdev);
	pci_set_master(pdev);

	/* Reprogram a validated MSI message (SBR zeroed all eDMA registers). */
	ret = dx_edma_restore_msi_after_reset(dw, &saved_msi);
	if (ret)
		return ret;

	/* Reprogram all eDMA registers (int_mask, ch_pwr, engine, IMWR, iATU) */
	edma_reprogram_hw(dw);

	dx_edma_pcie_reset_diag(dw, pdev);

	/*
	 * Re-save PCI state for the NEXT recovery.  pci_restore_state() above
	 * consumed the saved state — without this re-save, the next SBR or
	 * link-flap recovery finds no saved state and leaves BARs unprogrammed.
	 */
	pci_save_state(pdev);

	/*
	 * FLR keeps the EP firmware alive and may not clear a latched channel
	 * error (CS=1/2).  If FLR left a channel stuck and the host can perform
	 * a bus reset, escalate to SBR; the post-reset re-init is then
	 * completed asynchronously by the link-health worker once the link
	 * re-trains.
	 */
	if (!use_sbr && dx_edma_flr_channels_stuck(dw)) {
		if (dx_edma_sbr_viable(pdev)) {
			pr_warn("[pcie_reset] FLR did not clear stuck DMA channel(s); escalating to Secondary Bus Reset\n");
			use_sbr = true;
			goto reset_again;
		}
		pr_warn("[pcie_reset] FLR did not clear stuck DMA channel(s); no safe SBR on this host — a firmware-latched error may require a reboot to clear\n");
		return -EIO;
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
	bool atomic_wait;
	unsigned int atomic_retry;
	const unsigned int atomic_timeout_us = 5000;
	const unsigned int atomic_step_us = 10;

	if (!dx_dma_mmio_poll_allowed(dw))
		return -ENODEV;

	ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);
	if (ch_control == U32_MAX)
		return -ENODEV;
	atomic_wait = in_atomic() || irqs_disabled();

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

		if (atomic_wait) {
			for (atomic_retry = 0;
			     atomic_retry < (atomic_timeout_us / atomic_step_us);
			     atomic_retry++) {
				u32 new_cs;

				if (!dx_dma_mmio_poll_allowed(dw))
					return -ENODEV;
				ch_control = GET_CH_32(dw, chan->dir, chan->id,
						      ch_control1);
				if (ch_control == U32_MAX)
					return -ENODEV;
				new_cs = (ch_control >> 5) & 0x3;
				if (new_cs != 1) {
					if (new_cs == 2) {
						dw_edma_v0_core_ch_recover_abort(chan);
						return -EIO;
					}
					return 0;
				}
				udelay(atomic_step_us);
			}

			pr_warn("CH%d: busy after atomic wait (%u us, CS=%u)\n",
				chan->id, atomic_timeout_us,
				(ch_control >> 5) & 0x3);
			return -EBUSY;
		}

		start_jiffies = jiffies;
		timeout_jiffies = start_jiffies + usecs_to_jiffies(5000);
		retry_count = 0;
		status_changed = false;

		while (time_before(jiffies, timeout_jiffies)) {
			retry_count++;
			if (!dx_dma_mmio_poll_allowed(dw))
				return -ENODEV;
			ch_control = GET_CH_32(dw, chan->dir, chan->id, ch_control1);
			if (ch_control == U32_MAX)
				return -ENODEV;

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

			if (new_cs == 2) {
				dw_edma_v0_core_ch_recover_abort(chan);
				return -EIO;
			}
		} else {
			pr_warn("CH%d status still busy after %u us (%d retries)\n",
				chan->id, elapsed_us, retry_count);
			return -EBUSY;
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
		if (!dx_dma_mmio_poll_allowed(dw))
			return -ENODEV;
		ctrl1.U = GET_CH_32(dw, dir, channel, ch_control1);
		if (ctrl1.U == U32_MAX)
			return -ENODEV;
		
		/* CS (Channel Status): CS=0 and DMA_STOP are idle.  DMA_ERR means
		 * the helper channel is halted and ringing a doorbell would be ignored. */
		if (ctrl1.CS == 0 || ctrl1.CS == DMA_STOP)
			return 0;
		if (ctrl1.CS == DMA_ERR)
			return -EIO;

		elapsed++;
	}

	pr_err("Channel %d (%s) not idle after %dms (CS=%d)\n",
	       channel, is_write ? "R" : "W", DMA_POLL_TIMEOUT_US, ctrl1.CS);
	return -ETIMEDOUT;
}

static int dw_edma_v0_core_write_chunk(struct dw_edma_chunk *chunk, int dev_n, int dma_n, int ch_n)
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
		return -EINVAL;
	}
	if (unlikely(!chunk->burst || list_empty(&chunk->burst->list))) {
		pr_err("[edma] ch%d dev%d dma%d empty burst list (burst=%p alloc=%d)\n",
		       chunk->chan->id, dev_n, dma_n, chunk->burst, chunk->bursts_alloc);
		return -EINVAL;
	}

	dx_pcie_start_profile(PCIE_DESC_GEN_T, 0, dev_n, dma_n, ch_n);

	orig_bursts = chunk->bursts_alloc;

	/* Merge contiguous bursts to reduce LLI count. */
	ptr = list_first_entry(&chunk->burst->list,
			       struct dw_edma_burst, list);
	list_for_each_entry_safe(curr, next,
				&chunk->burst->list, list) {
		bool contd = false;

		if (ptr == curr)
			continue;

		if (ch_n == EDMA_DIR_WRITE &&
		    curr->dar == (ptr->dar + ptr->sz))
			contd = true;
		else if (ch_n == EDMA_DIR_READ &&
			 curr->sar == (ptr->sar + ptr->sz))
			contd = true;

		if (contd) {
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
	return 0;
}

static void dw_edma_v0_xfer_llm(struct dw_edma_chunk *chunk)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	u32 ch_control1;

	/*
	 * LLI descriptor-copy DMA can use the same READ channel as H2C data.
	 * Its Done status/MSI must not be observed as data-transfer Done after
	 * xfer_started is set below, otherwise the next chunk can be launched
	 * while the real data transfer is still CS=RUN.
	 */
	dw_edma_v0_core_clear_done_int(chan);
	dw_edma_v0_core_unmask_done_abort_int(chan);
	
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
	WRITE_ONCE(chan->last_xfer_start_jiffies, jiffies);
	WRITE_ONCE(chan->xfer_started, true);
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
		if (!dx_dma_mmio_poll_allowed(dw))
			return -ENODEV;

		ctrl1.U = GET_CH_32(dw, dir, channel, ch_control1);
		if (ctrl1.U == U32_MAX)
			return -ENODEV;
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

		if (!(in_atomic() || irqs_disabled()) && (elapsed % 1024) == 0)
			cond_resched();

		elapsed++;
	}

	pr_err("[%s][%d] CS=%d size=%u Timeout (%d ms) \n",
		(dir == EDMA_DIR_WRITE) ? "W" : "R", channel, ctrl1.CS, xfer_size, max_wait);
	return -ETIMEDOUT;
}

static bool dx_dma_try_acquire_helper_channel(struct dma_chan_lock *target_lock,
					      unsigned long *flags)
{
	return dx_dma_try_acquire_chan_ownership(target_lock, flags);
}

static void dx_dma_release_helper_channel(struct dma_chan_lock *target_lock,
					 unsigned long *flags)
{
	dx_dma_release_chan_ownership(target_lock, flags);
}

static bool dx_dma_mmio_poll_allowed(struct dw_edma *dw)
{
	return atomic_read(&dw->dev_state) == DX_DEV_LIVE &&
		atomic_read(&dw->link_state) == DX_LINK_UP &&
		!atomic_read(&dw->background_recovery_paused);
}

static bool dx_dma_shadow_precopy_allowed(struct dw_edma *dw)
{
	/* Shadow pre-copy uses READ helper channels 2/3.  Public H2C data is
	 * limited to READ channels 0/1, and normal driver DMA marks the same
	 * per-channel ownership locks while a transfer is active. */
	return dx_dma_mmio_poll_allowed(dw) &&
		dw->rd_ch_cnt > DX_READ_HELPER_CH_LAST;
}

static bool dx_dma_desc_helper_allowed(struct dw_edma *dw)
{
	/*
	 * Descriptor-copy DMA is always a READ-channel host->device copy into
	 * device LL SRAM.  Do not reuse public READ channels 0/1: H2C data uses
	 * those channels, and a descriptor-copy Done on the data channel is
	 * indistinguishable from the real data Done.  Use reserved READ helpers
	 * 2/3 whenever HW exposes them and MMIO is known-live.
	 */
	return dx_dma_mmio_poll_allowed(dw) &&
		dw->rd_ch_cnt > DX_READ_HELPER_CH_LAST;
}

bool dw_edma_v0_core_shadow_precopy_available(struct dw_edma *dw)
{
	return dx_dma_shadow_precopy_allowed(dw);
}

static int dw_edma_v0_core_xfer_llm_desc(struct dw_edma_chunk *chunk)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	DMA_CH_CONTROL1_OFF_t ctrl1;
	int ret = 0;
	int channel = -1;
	int retry;
	unsigned long flags;
	bool use_dedicated = false;
	struct dma_chan_lock *target_lock = NULL;
	ktime_t llm_start;
	bool perf_enabled = READ_ONCE(g_perf_enabled);
	bool atomic_context = in_atomic() || irqs_disabled();
	u32 acquire_timeout_us = atomic_context ?
		DX_DMA_HELPER_ACQUIRE_ATOMIC_TIMEOUT_US :
		DX_DMA_HELPER_ACQUIRE_TIMEOUT_US;
	/* Round-Robin Toggle for Load Balancing (per-device) */
	int first_ch, second_ch;

	if (dx_dma_desc_helper_allowed(dw)) {
		/* Toggle priority: Odd->(3,2), Even->(2,3) */
		if (atomic_inc_return(&dw->rr_toggle) & 1) {
			first_ch = EDMA_CH_ID_3;
			second_ch = EDMA_CH_ID_2;
		} else {
			first_ch = EDMA_CH_ID_2;
			second_ch = EDMA_CH_ID_3;
		}

		for (retry = 0;
		     retry < (acquire_timeout_us /
			      DX_DMA_HELPER_ACQUIRE_STEP_US) && !use_dedicated;
		     retry++) {
			if (!dx_dma_desc_helper_allowed(dw))
				return -ENODEV;
			if (READ_ONCE(chan->aborted) || READ_ONCE(chan->hw_err))
				return -EIO;

			target_lock = &dw->rd_dma_chan_locks[first_ch];
			if (dx_dma_try_acquire_helper_channel(target_lock, &flags)) {
				use_dedicated = true;
				channel = first_ch;
				break;
			}
			target_lock = &dw->rd_dma_chan_locks[second_ch];
			if (dx_dma_try_acquire_helper_channel(target_lock, &flags)) {
				use_dedicated = true;
				channel = second_ch;
				break;
			}

			if (atomic_context)
				udelay(DX_DMA_HELPER_ACQUIRE_STEP_US);
			else
				usleep_range(DX_DMA_HELPER_ACQUIRE_STEP_US,
					     DX_DMA_HELPER_ACQUIRE_STEP_US * 2);
		}
		if (perf_enabled)
			dx_pcie_perf_record_helper_acquire(dw->idx,
				use_dedicated ? retry + 1 : retry,
				channel, use_dedicated);

		if (!use_dedicated) {
			pr_err_ratelimited("[R] helper channels busy for LLI copy\n");
			return -EBUSY;
		}
	} else if (chan->dir == EDMA_DIR_WRITE) {
		pr_err("[R] no safe helper channel for C2H LLI copy\n");
		return -EBUSY;
	}

	if (!use_dedicated) {
		ret = wait_for_dma_channel_idle(dw, chan->id, EDMA_DIR_READ);
		if (ret != 0) {
			pr_err("[R][%d] Channel not idle\n", chan->id);
			return ret;
		}
		channel = chan->id;
	} else {
		ret = wait_for_dma_channel_idle(dw, channel, EDMA_DIR_READ);
		if (ret != 0) {
			pr_err("[R][%d] helper channel not idle\n", channel);
			dx_dma_release_helper_channel(target_lock, &flags);
			return ret;
		}
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

	if (perf_enabled)
		get_start_time(&llm_start);

	SET_RW_32(dw, EDMA_DIR_READ, doorbell,
		FIELD_PREP(EDMA_V0_DOORBELL_CH_MASK, channel));

	ret = dx_dma_polling_wait(dw, channel, EDMA_DIR_READ);
	if (chan->dir == EDMA_DIR_WRITE && perf_enabled)
		dx_pcie_perf_record_helper_llm_copy(dw->idx,
			get_elapsed_time_ns(llm_start), ret);
	if (ret) {
		pr_err("[R][%d] LLM desc xfer fail\n", channel);
	}

	if (use_dedicated) {
		target_lock = &dw->rd_dma_chan_locks[channel];
		dx_dma_release_helper_channel(target_lock, &flags);
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
	WRITE_ONCE(chan->last_xfer_start_jiffies, jiffies);
	WRITE_ONCE(chan->xfer_started, true);
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

static void dw_edma_v0_core_signal_hw_err(struct dw_edma_chan *chan)
{
	wait_queue_head_t *wq;

	WRITE_ONCE(chan->hw_err, true);
	wq = READ_ONCE(chan->transfer_wq);
	if (wq)
		wake_up(wq);
}

void dw_edma_v0_core_cancel_prepared(struct dw_edma_chunk *chunk, bool is_llm)
{
	struct dw_edma_chan *chan = chunk->chan;

	if (!is_llm)
		return;

	/* A prepared LLM chunk masked data Done while helper-copying the LLI.
	 * If the caller cancels before the data doorbell, clear any descriptor-copy
	 * Done residue and restore the data channel interrupt mask for future use. */
	dw_edma_v0_core_clear_done_int(chan);
	dw_edma_v0_core_unmask_done_abort_int(chan);
}

int dw_edma_v0_core_prepare_start(struct dw_edma_chunk *chunk, bool first,
					  bool is_llm)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	int ret;

	if (first && !is_llm)
		dw_edma_v0_core_unmask_done_abort_int(chan);

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
		dw_edma_v0_core_signal_hw_err(chan);
		return ret;
	}
	if (is_llm) {
		/* Suppress descriptor-copy Done IRQs; data Done is armed in
		 * dw_edma_v0_xfer_llm() immediately before the data doorbell. */
		dw_edma_v0_core_mask_done_int(chan);
		dw_edma_v0_core_clear_done_int(chan);

		ret = dw_edma_v0_core_write_chunk(chunk, dw->idx, chan->id, chan->dir);
		if (ret) {
			pr_err("CH%d: write_chunk failed (%d), skipping doorbell\n",
			       chan->id, ret);
			dw_edma_v0_core_signal_hw_err(chan);
			dw_edma_v0_core_cancel_prepared(chunk, is_llm);
			return ret;
		}
		
		/* Sync for device if using Buddy Allocator (Streaming DMA).
		 * Only sync the actual descriptor data size, not the entire
		 * 1MB host_region, to avoid excessive cache flush overhead. */
		if (chan->desc_buf_is_buddy) {
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
		if (ret) {
			/*
			 * -EBUSY = helper channels transiently exhausted.
			 * NOT a HW error — caller (lazy refill / start_transfer)
			 * must decide whether to retry or escalate.  Don't
			 * signal hw_err here so that we don't poison the
			 * channel state for a recoverable resource conflict.
			 *
			 * Other return codes (e.g. -ETIMEDOUT from polling_wait)
			 * indicate the helper actually started but did not
			 * complete — that IS a HW-level fault and must escalate.
			 */
			if (ret != -EBUSY) {
				pr_err(">> LLM descriptor transfer failed, aborting (err=%d)\n",
				       ret);
				dw_edma_v0_core_signal_hw_err(chan);
			}
			dw_edma_v0_core_cancel_prepared(chunk, is_llm);
			return ret;
		}
	}

	return 0;
}

void dw_edma_v0_core_launch_prepared(struct dw_edma_chunk *chunk, bool is_llm)
{
	if (is_llm)
		dw_edma_v0_xfer_llm(chunk);
	else
		dw_edma_v0_xfer_llm_not(chunk);
}

/*
 * Pre-build a chunk's LLI in host memory (write_chunk + cache flush).
 * Called from workqueue context. Does NOT touch BAR0 or doorbell.
 * Returns 0 on success, negative errno on failure.
 */
int dw_edma_v0_core_prebuild_chunk(struct dw_edma_chunk *chunk)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	size_t desc_sz;
	int ret;

	ret = dw_edma_v0_core_write_chunk(chunk, dw->idx, chan->id, chan->dir);
	if (ret)
		return ret;

	/* Sync the LLI data for device if using Buddy Allocator */
	if (chan->desc_buf_is_buddy) {
		desc_sz = (chunk->bursts_alloc + 1) * EDMA_LL_SZ;
		desc_sz += sizeof(struct dw_edma_v0_llp);
		if (desc_sz > chunk->host_region.sz)
			desc_sz = chunk->host_region.sz;
		dma_sync_single_for_device(chan->chip->dev,
					   chunk->host_region.paddr,
					   desc_sz,
					   DMA_TO_DEVICE);
	}
	return 0;
}

/*
 * Pre-copy LLI from host memory to device LL SRAM using dedicated READ
 * channels 2/3.  Called from workqueue context (process context, safe to
 * poll) OUTSIDE any spinlock.  This lets the ISR later launch the chunk
 * with just a doorbell write (no expensive DMA copy under lock).
 */
int dw_edma_v0_core_precopy_lli(struct dw_edma_chunk *chunk,
				       struct dw_edma_desc *desc)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	DMA_CH_CONTROL1_OFF_t ctrl1;
	int ret = 0;
	int channel = -1;
	unsigned long flags;
	struct dma_chan_lock *target_lock = NULL;
	int first_ch, second_ch;
	int retry;

	/* Dedicated helper channels are reserved only for WRITE data paths.
	 * READ data channels 2/3 are hidden from normal clients and used only
	 * for descriptor-copy helper DMA. */
	if (chan->dir != EDMA_DIR_WRITE)
		return -EOPNOTSUPP;

	if (!dx_dma_shadow_precopy_allowed(dw))
		return -ENODEV;

	/* Always use dedicated READ channels 2/3 for LLI copy,
	 * regardless of data channel direction.  This avoids
	 * conflicting with a running data DMA on the same channel. */
	if (atomic_inc_return(&dw->rr_toggle) & 1) {
		first_ch = EDMA_CH_ID_3;
		second_ch = EDMA_CH_ID_2;
	} else {
		first_ch = EDMA_CH_ID_2;
		second_ch = EDMA_CH_ID_3;
	}

	for (retry = 0;
	     retry < (DX_DMA_HELPER_ACQUIRE_TIMEOUT_US /
		      DX_DMA_HELPER_ACQUIRE_STEP_US);
	     retry++) {
		if (!dx_dma_shadow_precopy_allowed(dw))
			return -ENODEV;
		if (desc && READ_ONCE(desc->shadow_state) == SHADOW_CANCELLED)
			return -ECANCELED;
		if (READ_ONCE(chan->aborted) || READ_ONCE(chan->hw_err))
			return -EIO;

		target_lock = &dw->rd_dma_chan_locks[first_ch];
		if (dx_dma_try_acquire_helper_channel(target_lock, &flags)) {
			channel = first_ch;
			break;
		}
		target_lock = &dw->rd_dma_chan_locks[second_ch];
		if (dx_dma_try_acquire_helper_channel(target_lock, &flags)) {
			channel = second_ch;
			break;
		}
		usleep_range(DX_DMA_HELPER_ACQUIRE_STEP_US,
			     DX_DMA_HELPER_ACQUIRE_STEP_US * 2);
	}
	if (channel < 0)
		return -EBUSY;

	if (!dx_dma_shadow_precopy_allowed(dw) ||
	    (desc && READ_ONCE(desc->shadow_state) == SHADOW_CANCELLED) ||
	    READ_ONCE(chan->aborted) || READ_ONCE(chan->hw_err)) {
		target_lock = &dw->rd_dma_chan_locks[channel];
		dx_dma_release_helper_channel(target_lock, &flags);
		return -ECANCELED;
	}

	ret = wait_for_dma_channel_idle(dw, channel, EDMA_DIR_READ);
	if (ret) {
		target_lock = &dw->rd_dma_chan_locks[channel];
		dx_dma_release_helper_channel(target_lock, &flags);
		return ret;
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

	dbg_tfr(">> [R][%d] precopy LLI for [%s][%d] sz:0x%x\n",
		channel,
		(chan->dir == EDMA_DIR_WRITE) ? "W" : "R", chan->id,
		(chunk->bursts_alloc + 1) * EDMA_LL_SZ);

	SET_RW_32(dw, EDMA_DIR_READ, doorbell,
		  FIELD_PREP(EDMA_V0_DOORBELL_CH_MASK, channel));

	ret = dx_dma_polling_wait(dw, channel, EDMA_DIR_READ);
	if (ret)
		pr_err("[R][%d] LLI precopy xfer fail\n", channel);

	target_lock = &dw->rd_dma_chan_locks[channel];
	dx_dma_release_helper_channel(target_lock, &flags);

	return ret;
}

/*
 * Lightweight launch for a chunk whose LLI was already pre-copied to
 * device LL SRAM by dw_edma_v0_core_precopy_lli().  Only programs the
 * LLP register and rings the doorbell — no DMA copy, no polling.
 *
 * Do NOT poll channel CS here.  This function is used only by the shadow
 * continuation path after the current chunk's Done ISR has already run:
 * the WQ launches only after the ISR has set SHADOW_ISR_PENDING.
 *
 * At that point the software state machine, not the CS bits, is the source
 * of truth that the previous chunk is complete and the device LL SRAM can be
 * reused.  On this eDMA block CS can transiently report the LL-fetch/error
 * encoding at chunk boundaries; treating that as a hard failure causes the
 * 1GB multi-channel shadow test to abort with "skipping doorbell" even
 * though the descriptor was built and the previous chunk completed.
 *
 * Safe to call under spinlock (all MMIO, non-sleeping, ~5 BAR0 writes).
 */
int dw_edma_v0_core_launch_precopied(struct dw_edma_chunk *chunk)
{
	dw_edma_v0_xfer_llm(chunk);
	return 0;
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
