// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/wait.h>

#include "dw-edma-core.h"
#include "dw-edma-v0-core.h"

#include "dw-edma-thread.h"
#include "dx_util.h"
#include "dx_sgdma_cdev.h"
#include "dx_lib.h"
#include "dx_message.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/task.h>
#endif

#define EDMA_CHANNEL_NAME			"dma%dchan%d"

static u32 timeout = 3000;
module_param(timeout, uint, 0644);
MODULE_PARM_DESC(timeout, "Transfer timeout in msec");

/*
 * Post-signal in-flight drain bound.
 *
 * After a signal (e.g. SIGKILL from a dying client's exit_group) interrupts
 * an in-flight DMA, we deliberately keep draining so a late Done MSI does not
 * dereference the caller's stack-owned callback context.  Each drain wait uses
 * no-progress (deadline-extends-on-progress) semantics, so a return of 0 means
 * ZERO forward progress for a full `timeout` window.  If the device is wedged
 * (silently stalled after repeated SBR recoveries, with no abort IRQ / link-down
 * to set hw_err or bump recovery_epoch) the drain must NOT wait uninterruptibly
 * forever — that leaves an unkillable D-state zombie.  Abort after this many
 * consecutive zero-progress windows so the thread can exit and the error
 * propagates into the normal recovery path.
 */
#define DX_POST_SIGNAL_DRAIN_MAX_STALLS		2

static struct dw_edma_info test_info[MAX_DEV_NUM][EDMA_MAX_WR_CH][EDMA_CH_END];

/* Global mutex to prevent concurrent channel allocation */
static DEFINE_MUTEX(dma_allocation_mutex);

static unsigned long dx_jiffies_age_ms(unsigned long then)
{
	if (!then)
		return ~0UL;
	return jiffies_to_msecs(jiffies - then);
}

static void dw_edma_log_completion_snapshot(struct dw_edma_info *info,
						    struct dma_chan *chan,
						    dma_cookie_t cookie,
						    u64 start_status_seen,
						    u64 start_done_isr,
						    u64 start_done_replay)
{
	struct dw_edma_chan *dw_chan = dchan2dw_edma_chan(chan);
	struct dw_edma *dw = dw_chan->chip->dw;
	struct dw_edma_done *done = &info->dma_done;
	struct device *dev = chan->device->dev;
	struct virt_dma_desc *vd;
	struct dw_edma_desc *desc;
	struct dw_edma_chunk *child;
	enum dma_status cookie_status;
	u64 status_seen, done_isr, done_replay;
	unsigned long last_status, last_isr, last_replay;
	unsigned long flags;
	u32 cs, done_int, abort_int;
	bool lazy = false, shadow_more = false;
	int shadow_state = -1, chunks_alloc = -1, bursts = -1;
	u32 sg_remaining = 0, shadow_remaining = 0, chunk_sz = 0;

	cookie_status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
	cs = dw_edma_v0_core_ch_status_raw(dw_chan);
	done_int = dw_edma_v0_core_status_done_int(dw, dw_chan->dir);
	abort_int = dw_edma_v0_core_status_abort_int(dw, dw_chan->dir);
	status_seen = atomic64_read(&dw_chan->done_status_seen_cnt);
	done_isr = atomic64_read(&dw_chan->done_isr_cnt);
	done_replay = atomic64_read(&dw_chan->done_replay_cnt);
	last_status = READ_ONCE(dw_chan->last_done_status_jiffies);
	last_isr = READ_ONCE(dw_chan->last_done_isr_jiffies);
	last_replay = READ_ONCE(dw_chan->last_done_replay_jiffies);

	spin_lock_irqsave(&dw_chan->vc.lock, flags);
	vd = vchan_next_desc(&dw_chan->vc);
	if (vd) {
		desc = vd2dw_edma_desc(vd);
		lazy = READ_ONCE(desc->lazy_mode);
		shadow_state = desc->shadow_state;
		shadow_more = desc->shadow_has_more;
		sg_remaining = desc->sg_remaining;
		shadow_remaining = desc->shadow_next_remaining;
		chunks_alloc = desc->chunks_alloc;
		child = list_first_entry_or_null(&desc->chunk->list,
						       struct dw_edma_chunk, list);
		if (child) {
			chunk_sz = child->ll_region.sz;
			bursts = child->bursts_alloc;
		}
	}
	spin_unlock_irqrestore(&dw_chan->vc.lock, flags);

	dev_warn_ratelimited(dev,
		"%s: completion snapshot done=%d hw_err=%d cookie=%d cs=%u done_int=0x%x abort_int=0x%x xfer_started=%d shadow_active=%d lazy=%d shadow_st=%d shadow_more=%d sg_rem=%u shadow_rem=%u chunks=%d chunk_sz=%u bursts=%d irq_seen=%llu(+%llu) done_isr=%llu(+%llu) replay=%llu(+%llu) age_ms(status/isr/replay)=%lu/%lu/%lu\n",
		dma_chan_name(chan), READ_ONCE(done->done),
		READ_ONCE(dw_chan->hw_err), cookie_status, cs, done_int,
		abort_int, READ_ONCE(dw_chan->xfer_started),
		dw_edma_shadow_transfer_active(dw_chan), lazy, shadow_state,
		shadow_more, sg_remaining, shadow_remaining, chunks_alloc,
		chunk_sz, bursts, status_seen,
		status_seen - start_status_seen, done_isr,
		done_isr - start_done_isr, done_replay,
		done_replay - start_done_replay,
		dx_jiffies_age_ms(last_status), dx_jiffies_age_ms(last_isr),
		dx_jiffies_age_ms(last_replay));
}

static u64 dx_dma_addr_end(u64 start, size_t len)
{
	if (!len)
		return start;
	if (start > U64_MAX - (u64)(len - 1))
		return U64_MAX;
	return start + (u64)len - 1;
}

static u64 dx_dma_addr_add(u64 start, u64 off)
{
	if (start > U64_MAX - off)
		return U64_MAX;
	return start + off;
}

static void dw_edma_log_xfer_context(struct dw_edma_info *info,
					     struct dma_chan *chan,
					     dma_cookie_t cookie,
					     const char *reason,
					     int err)
{
	struct dx_dma_io_cb *cb = info ? info->cb : NULL;
	struct dw_edma_chan *dw_chan;
	struct device *dev;
	struct dma_tx_state txstate = {0};
	struct virt_dma_desc *vd;
	struct dw_edma_desc *desc;
	struct scatterlist *sg;
	dma_addr_t first_dma = 0;
	unsigned int first_dma_len = 0;
	unsigned long flags;
	enum dma_status status;
	u64 completed = 0;
	u64 described = 0;
	u64 inflight = 0;
	u64 remaining = 0;
	u64 next_ep;
	u32 cs;
	u32 sg_remaining = 0;
	u32 shadow_remaining = 0;
	int shadow_state = -1;
	bool lazy = false;

	if (!cb || !chan || !chan->device || !chan->device->dev)
		return;

	dw_chan = dchan2dw_edma_chan(chan);
	if (!dw_chan)
		return;

	dev = chan->device->dev;

	status = dmaengine_tx_status(chan, cookie, &txstate);
	cs = dw_edma_v0_core_ch_status_raw(dw_chan);

	spin_lock_irqsave(&dw_chan->vc.lock, flags);
	vd = vchan_find_desc(&dw_chan->vc, cookie);
	if (!vd)
		vd = vchan_next_desc(&dw_chan->vc);
	if (vd) {
		desc = vd2dw_edma_desc(vd);
		completed = desc->xfer_sz;
		described = (u64)desc->alloc_sz + desc->shadow_alloc_sz;
		lazy = READ_ONCE(desc->lazy_mode);
		shadow_state = desc->shadow_state;
		sg_remaining = desc->sg_remaining;
		shadow_remaining = desc->shadow_next_remaining;
	}
	spin_unlock_irqrestore(&dw_chan->vc.lock, flags);

	if (completed > cb->len)
		completed = cb->len;
	if (described > cb->len)
		described = cb->len;
	if (described > completed)
		inflight = described - completed;
	if (cb->len > completed)
		remaining = cb->len - completed;
	next_ep = dx_dma_addr_add(cb->ep_addr, completed);

	sg = cb->sgt.sgl;
	if (sg) {
		first_dma = sg_dma_address(sg);
		first_dma_len = sg_dma_len(sg);
	}

	dev_err(dev,
		"%s: DMA %s error (%s, err=%d): path=%s host_user=0x%llx first_dma=%pad first_dma_len=0x%x sg_nents=%u pages=%u ep=0x%llx..0x%llx total=0x%zx completed_at_least=0x%llx next_ep=0x%llx remaining_at_least=0x%llx described=0x%llx inflight_unknown=0x%llx residue=0x%x status=%d cs=%u lazy=%d sg_rem=%u shadow_st=%d shadow_rem=%u\n",
		dma_chan_name(chan), cb->write ? "write" : "read", reason,
		err, cb->write ? "Host->EP" : "EP->Host",
		(unsigned long long)(uintptr_t)cb->buf, &first_dma,
		first_dma_len, cb->sgt.nents, cb->pages_nr, cb->ep_addr,
		dx_dma_addr_end(cb->ep_addr, cb->len), cb->len,
		completed, next_ep, remaining, described, inflight,
		txstate.residue, status, cs, lazy, sg_remaining,
		shadow_state, shadow_remaining);
}

enum dw_edma_fail_action {
	DW_EDMA_FAIL_NO_HW_TOUCH = 0,
	DW_EDMA_FAIL_TERMINATE_ONCE,
	DW_EDMA_FAIL_TERMINATE_TWICE,
};

static void dw_edma_finalize_failed_transfer(struct dw_edma_info *info,
					  struct dma_chan *chan,
					  dma_cookie_t cookie,
					  const char *reason,
					  int err,
					  enum dw_edma_fail_action action)
{
	struct dx_dma_io_cb *cb = info->cb;

	if (!err)
		err = -EIO;

	/* Process-context only: callers are dw_edma_sg_process() error exits. */
	dw_edma_log_xfer_context(info, chan, cookie, reason, err);

	switch (action) {
	case DW_EDMA_FAIL_TERMINATE_TWICE:
		dmaengine_terminate_all(chan);
		dmaengine_terminate_all(chan);
		break;
	case DW_EDMA_FAIL_TERMINATE_ONCE:
		dmaengine_terminate_all(chan);
		break;
	case DW_EDMA_FAIL_NO_HW_TOUCH:
	default:
		break;
	}

	cb->result = err;
}

static struct dma_chan_lock *dw_edma_run_chan_lock(struct dw_edma_chan *dw_chan)
{
	struct dw_edma *dw = dw_chan->chip->dw;

	if (dw_chan->dir == EDMA_DIR_READ) {
		if (dw_chan->id >= EDMA_MAX_RD_CH)
			return NULL;
		return &dw->rd_dma_chan_locks[dw_chan->id];
	}

	if (dw_chan->id >= EDMA_MAX_WR_CH)
		return NULL;
	return &dw->wr_dma_chan_locks[dw_chan->id];
}

/*
 * Wait for done->done with per-chunk progress-based deadline extension.
 *
 * Multi-chunk shadow lazy transfers can legitimately exceed a single
 * fixed timeout, so each observed forward-progress event (Done ISR,
 * shadow-WQ replay, or status counter increment from any other code
 * path) refreshes the deadline ("no-progress timeout" semantics).
 *
 * This loop is pure observation: it never inspects CS and never pokes
 * HW.  CS classification and any recovery actions are deferred to
 * dw_edma_handle_wait_timeout() once this returns 0 (no progress
 * within the deadline).
 */
static long dw_edma_wait_done(struct dw_edma_info *info,
			      struct dma_chan *chan,
			      int start_epoch,
			      bool interruptible)
{
	struct dw_edma_chan *dw_chan = dchan2dw_edma_chan(chan);
	struct dw_edma *dw = dw_chan->chip->dw;
	struct dw_edma_done *done = &info->dma_done;
	unsigned long total = msecs_to_jiffies(timeout);
	unsigned long slice = msecs_to_jiffies(50);
	unsigned long deadline;
	u64 last_done_isr;
	u64 last_done_replay;
	u64 last_status_seen;
	long ret;

	if (!total)
		total = 1;
	if (!slice)
		slice = 1;
	deadline = jiffies + total;
	last_done_isr = atomic64_read(&dw_chan->done_isr_cnt);
	last_done_replay = atomic64_read(&dw_chan->done_replay_cnt);
	last_status_seen = atomic64_read(&dw_chan->done_status_seen_cnt);

	for (;;) {
		u64 done_isr;
		u64 done_replay;
		u64 status_seen;
		unsigned long remaining;
		unsigned long wait;

		done_isr = atomic64_read(&dw_chan->done_isr_cnt);
		done_replay = atomic64_read(&dw_chan->done_replay_cnt);
		status_seen = atomic64_read(&dw_chan->done_status_seen_cnt);
		if (done_isr != last_done_isr ||
		    done_replay != last_done_replay ||
		    status_seen != last_status_seen) {
			last_done_isr = done_isr;
			last_done_replay = done_replay;
			last_status_seen = status_seen;
			deadline = jiffies + total;
		}

		if (time_after_eq(jiffies, deadline))
			return 0;

		remaining = deadline - jiffies;
		wait = min(slice, remaining ? remaining : 1);
		if (interruptible) {
			ret = wait_event_interruptible_timeout(info->done_wait,
				READ_ONCE(done->done) ||
				READ_ONCE(dw_chan->hw_err),
				wait);
			if (ret == -ERESTARTSYS)
				return ret;
		} else {
			ret = wait_event_timeout(info->done_wait,
				READ_ONCE(done->done) ||
				READ_ONCE(dw_chan->hw_err),
				wait);
		}

		if (READ_ONCE(done->done) || READ_ONCE(dw_chan->hw_err))
			return ret ? ret : 1;
		if (atomic_read(&dw->recovery_epoch) != start_epoch)
			return 0;
	}
}

static void dw_edma_callback(void *arg)
{
	struct dw_edma_info *info = arg;
	dbg_tfr("[%s]\n", __func__);
	dx_pcie_end_profile(PCIE_ISR_EXEC_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

	dx_pcie_start_profile(PCIE_WAKEUP_LATENCY_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	WRITE_ONCE(info->dma_done.done, true);
	wake_up(info->dma_done.wait);
}

/*
 * Shared primitives for dw_edma_handle_wait_timeout stages.
 *
 * Return convention used by all stage / re-wait helpers:
 *    0    — done->done observed; caller may treat as completed
 *   -EIO  — recovery_epoch flipped or hw_err set; abort
 *    1    — neither; fall through to next stage / next action
 */

/*
 * Snapshot epoch/hw_err/done after a wait or external sync point.
 * Returns 0 if done observed, -EIO if epoch flipped or hw_err set,
 * 1 otherwise.
 */
static int dw_edma_check_done(struct dw_edma_info *info,
			      struct dma_chan *chan, int start_epoch)
{
	struct dw_edma_chan *dw_chan = dchan2dw_edma_chan(chan);
	struct dw_edma *dw = dw_chan->chip->dw;

	if (atomic_read(&dw->recovery_epoch) != start_epoch ||
	    READ_ONCE(dw_chan->hw_err))
		return -EIO;
	return READ_ONCE(info->dma_done.done) ? 0 : 1;
}

/*
 * Re-wait for done->done with epoch/hw_err short-circuit.  @interruptible
 * controls whether a signal can interrupt the wait (matches the original
 * mix of wait_event_timeout vs wait_event_interruptible_timeout).
 */
static int dw_edma_rewait_done(struct dw_edma_info *info,
			       struct dma_chan *chan,
			       int start_epoch,
			       unsigned int ms,
			       bool interruptible)
{
	struct dw_edma_chan *dw_chan = dchan2dw_edma_chan(chan);
	struct dw_edma_done *done = &info->dma_done;
	long jt = msecs_to_jiffies(ms);

	if (interruptible)
		wait_event_interruptible_timeout(info->done_wait,
			READ_ONCE(done->done) || READ_ONCE(dw_chan->hw_err),
			jt);
	else
		wait_event_timeout(info->done_wait,
			READ_ONCE(done->done) || READ_ONCE(dw_chan->hw_err),
			jt);

	return dw_edma_check_done(info, chan, start_epoch);
}

/*
 * Stage 1 — Late callback delivery.
 *
 * vchan_cookie_complete() updates the cookie state synchronously under
 * vc.lock, but the user-visible callback (and done->done flip) is only
 * delivered when the vchan tasklet runs.  On a busy box where softirq
 * scheduling slips past our timeout, HW is done but done->done is still
 * 0.  Boost the tasklet, brief re-wait, then synchronously flush via
 * tasklet_kill so no late tasklet can dereference info->cb (caller's
 * stack-allocated dx_dma_io_cb) after we return.
 *
 * IMPORTANT: once cookie == DMA_COMPLETE we must NOT fall through to
 * CS-based stages — the stuck branch would terminate a descriptor the
 * vchan tasklet is about to free (use-after-free / double-complete).
 */
static int dw_edma_try_late_callback_recovery(struct dw_edma_info *info,
					      struct dma_chan *chan,
					      dma_cookie_t cookie,
					      int start_epoch)
{
	struct dw_edma_chan *dw_chan = dchan2dw_edma_chan(chan);
	struct device *dev = chan->device->dev;
	int rc;

	if (dma_async_is_tx_complete(chan, cookie, NULL, NULL) != DMA_COMPLETE)
		return 1;

	tasklet_hi_schedule(&dw_chan->vc.task);
	rc = dw_edma_rewait_done(info, chan, start_epoch, 100, false);
	if (rc <= 0) {
		if (rc == 0)
			dev_warn_ratelimited(dev,
				"%s: late callback delivery (cookie complete, tasklet was delayed)\n",
				dma_chan_name(chan));
		return rc;
	}

	/* 100ms wasn't enough — synchronously flush the tasklet. */
	dev_warn_ratelimited(dev,
		"%s: cookie complete, tasklet stalled; forcing flush\n",
		dma_chan_name(chan));
	tasklet_kill(&dw_chan->vc.task);
	rc = dw_edma_check_done(info, chan, start_epoch);
	if (rc <= 0)
		return rc;

	/* Cookie complete, tasklet flushed, callback never fired — extremely
	 * rare (recovery path drained without epoch flip).  HW + dmaengine
	 * are still done; report error without touching the channel.
	 */
	dev_err(dev, "%s: cookie complete but callback never fired after flush\n",
		dma_chan_name(chan));
	return -EIO;
}

/*
 * Stage 2 — Unconditional grace re-wait.
 *
 * Give the ISR / shadow WQ one more timeout window to deliver progress.
 * CS is intentionally NOT consulted here — CS=3 is the normal stop
 * state and CS=1 means HW is still running; both warrant the same
 * "wait a bit more" response.  Re-launching the next chunk and waking
 * this thread are entirely the responsibility of the Done ISR (real or
 * shadow-WQ replayed); this stage MUST NOT poke HW.
 *
 * If still nothing, fall through to Stage 3/4 which classify by CS and
 * apply proper graceful-stop semantics for truly stuck channels.
 *
 * @cs is refreshed before fall-through so Stage 3 sees the latest value.
 */
static int dw_edma_grace_rewait(struct dw_edma_info *info,
				struct dma_chan *chan,
				int start_epoch, u32 *cs)
{
	struct dw_edma_chan *dw_chan = dchan2dw_edma_chan(chan);
	int rc;

	dev_dbg(chan->device->dev,
		"%s: watchdog expired (CS=%u); extending wait once\n",
		dma_chan_name(chan), *cs);
	rc = dw_edma_rewait_done(info, chan, start_epoch, timeout, true);
	if (rc <= 0) {
		if (rc == 0)
			dev_info_ratelimited(chan->device->dev,
				"%s: completion recovered after grace re-wait\n",
				dma_chan_name(chan));
		return rc;
	}

	*cs = dw_edma_v0_core_ch_status_raw(dw_chan);
	return 1;
}

/*
 * Stage 3 — CS=2 PCIe error.  Abort MSI never arrived (HW limitation).
 * Two terminate_all calls perform engine_en cycle to clear CS=2.  This
 * resets ALL channels on this direction — CS[6:5] is read-only HW
 * status with no per-channel recovery path.  If soft reset fails,
 * report a channel-fail event upstream.
 */
static int dw_edma_recover_cs2_error(struct dw_edma_info *info,
				     struct dma_chan *chan, u32 cs)
{
	struct dw_edma_chan *dw_chan = dchan2dw_edma_chan(chan);
	struct device *dev = chan->device->dev;

	if (cs != 2)
		return 1;

	dev_warn(dev, "%s: timeout (CS=2), attempting soft reset\n",
		 dma_chan_name(chan));
	dmaengine_terminate_all(chan);  /* 1st: EDMA_REQ_STOP */
	dmaengine_terminate_all(chan);  /* 2nd: ch_soft_reset */

	cs = dw_edma_v0_core_ch_status_raw(dw_chan);
	if (cs == 0 || cs == 3) {
		dev_info(dev, "%s: channel recovered (CS=%u)\n",
			 dma_chan_name(chan), cs);
		return -EIO;
	}

	dev_err(dev, "%s: soft reset FAILED (CS=%u), user-space recovery needed\n",
		dma_chan_name(chan), cs);
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
	dx_pcie_enqueue_event_response(dw_chan->chip->dw->idx,
		ERR_PCIE_DMA_CH_FAIL(dw_chan->id));
#endif
	return -EIO;
}

/*
 * Stage 4 — Stuck / unknown CS.  Mirror the SIGINT graceful-stop path:
 * 1st terminate_all only sets EDMA_REQ_STOP (no HW touch); the Done ISR
 * for the in-flight chunk lets us drain cleanly.  Only if that fails do
 * we force engine_cycle via a 2nd terminate_all.  Forcibly resetting an
 * in-flight transfer is dangerous (partial PCIe writes may leave HW in
 * CS=2 or worse).
 */
static int dw_edma_stuck_graceful_then_force(struct dw_edma_info *info,
					     struct dma_chan *chan,
					     int start_epoch, u32 cs)
{
	struct dw_edma_chan *dw_chan = dchan2dw_edma_chan(chan);
	struct dw_edma *dw = dw_chan->chip->dw;
	struct dw_edma_done *done = &info->dma_done;
	struct device *dev = chan->device->dev;
	int rc;

	dev_warn(dev, "%s: timeout (CS=%u), requesting graceful stop\n",
		 dma_chan_name(chan), cs);
	dmaengine_terminate_all(chan);  /* 1st: EDMA_REQ_STOP — no HW touch */

	rc = dw_edma_rewait_done(info, chan, start_epoch, timeout, false);
	if (rc <= 0) {
		if (rc == 0) {
			dev_info(dev,
				"%s: in-flight chunk completed after graceful stop\n",
				dma_chan_name(chan));
			return -EIO;  /* Channel safe; transfer was aborted */
		}
		return rc;
	}

	/* Graceful stop did not land — force engine cycle. */
	cs = dw_edma_v0_core_ch_status_raw(dw_chan);
	dev_err(dev,
		"%s: stuck after graceful stop (CS=%u) done=%d done_int=0x%x abort_int=0x%x; forcing reset\n",
		dma_chan_name(chan), cs, READ_ONCE(done->done),
		dw_edma_v0_core_status_done_int(dw, dw_chan->dir),
		dw_edma_v0_core_status_abort_int(dw, dw_chan->dir));
	dmaengine_terminate_all(chan);  /* 2nd: engine_cycle force reset */
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
	dx_pcie_enqueue_event_response(dw->idx,
		ERR_PCIE_DMA_CH_FAIL(dw_chan->id));
#endif
	return -ETIMEDOUT;
}

/*
 * dw_edma_handle_wait_timeout - Reclassify a wait_event timeout into action.
 *
 * Called when the primary wait returned without done->done, without
 * -ERESTARTSYS, and without hw_err.  Walks four stages, each either
 * resolves the timeout (0 / -errno) or returns 1 to fall through:
 *
 *   1. Late callback delivery       (cookie == DMA_COMPLETE)
 *   2. Grace re-wait                (one extra timeout window, CS-blind)
 *   3. CS=2 PCIe error              (engine_en cycle)
 *   4. Stuck / unknown CS           (graceful stop, then force)
 *
 * Stage 4 always resolves and is the terminal fallback.
 */
static int dw_edma_handle_wait_timeout(struct dw_edma_info *info,
				       struct dma_chan *chan,
				       dma_cookie_t cookie,
				       int start_epoch)
{
	struct dw_edma_chan *dw_chan = dchan2dw_edma_chan(chan);
	u32 cs;
	int rc;

	rc = dw_edma_try_late_callback_recovery(info, chan, cookie, start_epoch);
	if (rc != 1)
		return rc;

	cs = dw_edma_v0_core_ch_status_raw(dw_chan);

	rc = dw_edma_grace_rewait(info, chan, start_epoch, &cs);
	if (rc != 1)
		return rc;

	rc = dw_edma_recover_cs2_error(info, chan, cs);
	if (rc != 1)
		return rc;

	return dw_edma_stuck_graceful_then_force(info, chan, start_epoch, cs);
}

static int dw_edma_sg_process(struct dw_edma_info *info,
				    struct dma_chan *chan)
{
	struct dma_async_tx_descriptor *txdesc;
	struct dw_edma_done *done;
	struct dma_slave_config	sconf;
	enum dma_status status;
	struct scatterlist *sg;
	struct sg_table	*sgt;
	dma_cookie_t cookie;
	struct device *dev;
	u32 f_prp_cnt = 0;
	u32 f_sbt_cnt = 0;
	u32 f_cpl_err = 0;
	u32 f_cpl_bsy = 0;
	u32 f_tm_cnt = 0;
	struct dx_dma_io_cb *cb;
	struct dw_edma_chan *dw_chan;
	struct dma_chan_lock *chan_lock = NULL;
	unsigned long chan_lock_flags;
	bool chan_lock_owned = false;
	int orig_nents = 0;
	enum dma_transfer_direction	direction = !info->cb->write ? DMA_DEV_TO_MEM : DMA_MEM_TO_DEV;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	int nice = task_nice(current);
	int policy = current->policy;
#endif
#ifdef DMA_PERF_MEASURE
	ktime_t dma_trans_t;
#endif
	long ret;
	int start_epoch;
	u64 start_status_seen;
	u64 start_done_isr;
	u64 start_done_replay;

	dbg_tfr("[%s] Start!!\n", __func__);

#ifdef DMA_PERF_MEASURE
	if (g_perf_enabled)
		get_start_time(&dma_trans_t);
#endif

	if (chan == NULL) {
		pr_err("[%s] DMA channel Null point error! (dev_n=%d, npu_id=%d, write=%d)\n",
			__func__, info->dev_n,
			info->cb ? info->cb->npu_id : -1,
			info->cb ? info->cb->write : -1);
		return -1;
	}

	info->done = false;
	info->dma_done.wait = &info->done_wait;
	init_waitqueue_head(&info->done_wait);

	done = &info->dma_done;
	dev = chan->device->dev;
	cb = info->cb;
	dw_chan = dchan2dw_edma_chan(chan);

	if (info->cb->npu_run) {
		dw_chan->en_lie = true;
	} else {
		dw_chan->en_lie = false;
	}
	dbg_tfr("%s: local interrupt is %s\n",
			dma_chan_name(chan), 
			dw_chan->en_lie ? "enable" : "disable");

	/* Set SG Table */
	sgt = &(cb->sgt);
	sg = &cb->sgt.sgl[0];

	chan_lock = dw_edma_run_chan_lock(dw_chan);
	if (!chan_lock ||
	    !dx_dma_try_acquire_chan_ownership(chan_lock, &chan_lock_flags)) {
		dev_err(dev, "%s: DMA channel ownership busy\n",
			dma_chan_name(chan));
		cb->result = -EBUSY;
		goto err_alloc_descs;
	}
	chan_lock_owned = true;

	/*
	 * Configures DMA channel according to the direction
	 *  - flags
	 *  - source and destination addresses
	 */
	dx_pcie_start_profile(PCIE_DMA_MAP_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	/* Save original nents before dma_map_sg may coalesce entries.
	 * dma_unmap_sg must receive the same nents passed to dma_map_sg
	 * (i.e., the original count), otherwise cache maintenance
	 * (invalidation for DMA_FROM_DEVICE) will be incomplete,
	 * causing stale L1 cache lines (64-byte corruption on ARM). */
	orig_nents = sgt->nents;
	if (cb->pre_mapped) {
		/* Buffer already mapped (e.g. dma_alloc_coherent kernel buf).
		 * sg_dma_address/sg_dma_len are already set - skip dma_map_sg
		 * to avoid double-mapping and address overwrite. */
		dbg_tfr("%s: pre_mapped buffer, skip dma_map_sg\n",
			dma_chan_name(chan));
		if (direction == DMA_DEV_TO_MEM) {
			sconf.src_addr = cb->ep_addr;
			sconf.dst_addr = sg_dma_address(sg);
		} else {
			sconf.src_addr = sg_dma_address(sg);
			sconf.dst_addr = cb->ep_addr;
		}
	} else if (direction == DMA_DEV_TO_MEM) {
		/* DMA_DEV_TO_MEM - WRITE - DMA_FROM_DEVICE */
		dbg_tfr("%s: DMA_DEV_TO_MEM - WRITE - DMA_FROM_DEVICE\n",
			dma_chan_name(chan));
		sgt->nents = dma_map_sg(dev, sgt->sgl, orig_nents, DMA_FROM_DEVICE);
		if (!sgt->nents) {
			pr_err("sg table mapped fail(DMA_FROM_DEVICE)\n");
			goto err_alloc_descs;
		}
		/* Endpoint memory */
		sconf.src_addr = cb->ep_addr;
		/* CPU memory */
		sconf.dst_addr = sg_dma_address(sg);
	} else {
		/* DMA_MEM_TO_DEV - READ - DMA_TO_DEVICE */
		dbg_tfr("%s: DMA_MEM_TO_DEV - READ - DMA_TO_DEVICE\n",
			dma_chan_name(chan));
		sgt->nents = dma_map_sg(dev, sgt->sgl, orig_nents, DMA_TO_DEVICE);
		if (!sgt->nents) {
			pr_err("sg table mapped fail(DMA_TO_DEVICE)\n");
			goto err_alloc_descs;
		}

		/* CPU memory */
		sconf.src_addr = sg_dma_address(sg);
		/* Endpoint memory */
		sconf.dst_addr = cb->ep_addr;
	}
	dx_pcie_end_profile(PCIE_DMA_MAP_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	if (sgt->nents == 1) {
		cb->is_llm = false;
	}
	dw_chan->is_llm = cb->is_llm;

	sconf.direction = DMA_TRANS_NONE; /* remote DMA (Device <-> Host Memory) */

	dmaengine_slave_config(chan, &sconf);
	dbg_tfr("%s: addr(physical) src=%pa, dst=%pa\n",
		dma_chan_name(chan), &sconf.src_addr, &sconf.dst_addr);

	/*
	 * Admission gate — checked BEFORE prep/submit so that we never
	 * leave a descriptor with callback=dw_edma_callback dangling in
	 * any vchan list (desc_allocated / desc_submitted).
	 *
	 * Why BEFORE prep:
	 *   dw_edma_device_terminate_all() only drains descriptor lists
	 *   under the (chan->request == EDMA_REQ_STOP && ST_BUSY) branch.
	 *   Between submit and issue_pending the channel is still ST_IDLE
	 *   with configured=false, so terminate_all hits the "Do nothing"
	 *   branch and leaves the descriptor queued.  A later tasklet
	 *   (triggered by recovery, vchan flush, or ISR on adjacent
	 *   channel) would then fire dw_edma_callback() with info->cb
	 *   pointing at the caller's freed on-stack dx_dma_io_cb —
	 *   "BUG: Unable to handle page fault" on info->cb->len.
	 *
	 * Capture start_epoch here too so the post-wait guard below can
	 * detect a recovery that happened while we were waiting.
	 */
	start_epoch = atomic_read(&dw_chan->chip->dw->recovery_epoch);
	smp_rmb(); /* pairs with smp_mb() in dx_pcie_reset_dma_channels */
	{
		int state = atomic_read(&dw_chan->chip->dw->dev_state);
		if (state != DX_DEV_LIVE) {
			dev_warn(dev, "%s: device not live (state=%d), rejecting transfer\n",
				 dma_chan_name(chan), state);
			cb->result = -EIO;
			goto err_stats;
		}
	}

	/*
	 * Prepare the DMA channel for the transfer
	 *  - provide scatter-gather list
	 *  - configure to trigger an interrupt after the transfer
	 */
	dx_pcie_start_profile(PCIE_DMA_PREP_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	txdesc = dmaengine_prep_slave_sg(chan, sgt->sgl, sgt->nents,
					 direction,
					 DMA_PREP_INTERRUPT);
	if (!txdesc) {
		dev_err(dev, "%s: dmaengine_prep_slave_sg\n",
			dma_chan_name(chan));
		f_prp_cnt++;
		goto err_stats;
	}

	WRITE_ONCE(done->done, false);
	WRITE_ONCE(dw_chan->hw_err, false);
	dw_chan->aborted = false;

	txdesc->callback = dw_edma_callback;
	txdesc->callback_param = info;
	cookie = dmaengine_submit(txdesc);
	if (dma_submit_error(cookie)) {
		dev_err(dev, "%s: dma_submit_error\n", dma_chan_name(chan));
		f_sbt_cnt++;
		goto err_stats;
	}
	dx_pcie_end_profile(PCIE_DMA_PREP_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

#ifdef DMA_PERF_MEASURE
	/* send pointer to measure a performace */
	if (g_perf_enabled)
		dw_chan->chip->dw->irq[0].data[info->cb->npu_id][info->cb->write] = info;
#endif

	/* Publish waitqueue so recovery (dx_pcie_reset_dma_channels) can
	 * wake this thread immediately instead of waiting for timeout. */
	WRITE_ONCE(dw_chan->transfer_wq, &info->done_wait);
	start_status_seen = atomic64_read(&dw_chan->done_status_seen_cnt);
	start_done_isr = atomic64_read(&dw_chan->done_isr_cnt);
	start_done_replay = atomic64_read(&dw_chan->done_replay_cnt);

	/* Start DMA transfer - trigger a doorbell of dma.
	 *
	 * Any state flip that happens from here on is handled by:
	 *   - dx_pcie_reset_dma_channels (recovery thread calls
	 *     terminate_all while chan is ST_BUSY -> proper drain path)
	 *   - recovery_epoch re-check after wait_event returns
	 * Those paths safely complete the descriptor and wake us with
	 * cb->result = -EIO. */
	dx_pcie_start_profile(PCIE_DMA_XFER_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	dma_async_issue_pending(chan);


	/* Thread waits here for transfer completion or exists by timeout */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	if (policy == SCHED_NORMAL)
		sched_set_fifo(current);
#endif

	ret = dw_edma_wait_done(info, chan, start_epoch, true);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	if (policy == SCHED_NORMAL)
		sched_set_normal(current, nice);
#endif
	dx_pcie_end_profile(PCIE_WAKEUP_LATENCY_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

	/* Recovery guard: channel already cleaned up, exit immediately */
	if (atomic_read(&dw_chan->chip->dw->recovery_epoch) != start_epoch) {
		cb->result = -EIO;
		goto err_stats;
	}

	/* Check DMA transfer result and act upon it. */
	if (READ_ONCE(dw_chan->hw_err)) {
		dev_err(dev, "%s: channel signaled HW error (CS=%u), failing transfer\n",
			dma_chan_name(chan), dw_edma_v0_core_ch_status_raw(dw_chan));
		/*
		 * Two terminate_all calls needed: 1st sets EDMA_REQ_STOP,
		 * 2nd drains descriptors (prevents stale tasklet race).
		 */
		dw_edma_finalize_failed_transfer(info, chan, cookie, "hw_err", -EIO,
					      DW_EDMA_FAIL_TERMINATE_TWICE);
		/* No event here — hw_err is set by recovery path which
		 * already handles user-space coordination. */
	} else if (ret == -ERESTARTSYS) {
		unsigned int drain_stalls = 0;

		dev_info(dev, "%s: signal received, waiting for in-flight DMA completion\n",
			 dma_chan_name(chan));

		/*
		 * Ctrl+C only means the user task is leaving; it does not mean the
		 * already-doorbelled DMA should be stopped.  Keep the stack-owned
		 * callback context alive until the normal Done MSI/callback finishes.
		 *
		 * Bounded: dw_edma_wait_done(false) returns >0 on forward progress
		 * (Done ISR / status counter bump) and 0 only after a full no-progress
		 * `timeout` window.  A genuinely in-flight transfer keeps making
		 * progress; a wedged device makes none and, with no abort IRQ /
		 * link-down, never sets hw_err nor bumps recovery_epoch.  Abort after
		 * DX_POST_SIGNAL_DRAIN_MAX_STALLS zero-progress windows so this thread
		 * can exit instead of waiting uninterruptibly forever (which would
		 * leave an unkillable D-state zombie and wedge dxrtd).
		 */
		while (!READ_ONCE(done->done) && !READ_ONCE(dw_chan->hw_err) &&
		       atomic_read(&dw_chan->chip->dw->recovery_epoch) == start_epoch) {
			ret = dw_edma_wait_done(info, chan, start_epoch, false);
			if (READ_ONCE(done->done) || READ_ONCE(dw_chan->hw_err) ||
			    atomic_read(&dw_chan->chip->dw->recovery_epoch) != start_epoch)
				break;
			if (ret > 0) {
				/* forward progress observed — keep draining */
				drain_stalls = 0;
				continue;
			}
			/* ret == 0: a full no-progress window elapsed */
			if (++drain_stalls >= DX_POST_SIGNAL_DRAIN_MAX_STALLS) {
				dev_err(dev,
					"%s: no DMA progress %u ms after signal; aborting wedged transfer\n",
					dma_chan_name(chan),
					drain_stalls * timeout);
				dw_edma_finalize_failed_transfer(info, chan, cookie,
						"signal_drain_stall", -EIO,
						DW_EDMA_FAIL_TERMINATE_TWICE);
				cb->result = -EIO;
				goto err_stats;
			}
			dev_warn_ratelimited(dev,
				"%s: still waiting for DMA completion after signal (stall %u/%u)\n",
				dma_chan_name(chan), drain_stalls,
				DX_POST_SIGNAL_DRAIN_MAX_STALLS);
		}

		if (atomic_read(&dw_chan->chip->dw->recovery_epoch) != start_epoch ||
		    READ_ONCE(dw_chan->hw_err)) {
			cb->result = -EIO;
			goto err_stats;
		}

		dev_info(dev, "%s: DMA completed after signal\n",
			 dma_chan_name(chan));
		cb->result = 0;
	} else if (!READ_ONCE(done->done)) {
		int rc;

		dw_edma_log_completion_snapshot(info, chan, cookie,
			start_status_seen, start_done_isr, start_done_replay);
		rc = dw_edma_handle_wait_timeout(info, chan, cookie,
						 start_epoch);
		if (rc) {
			f_tm_cnt++;
			dw_edma_finalize_failed_transfer(info, chan, cookie,
					      "wait_timeout", rc,
					      DW_EDMA_FAIL_NO_HW_TOUCH);
		}
		/* rc == 0: late/shadow-replay recovery; done->done is set
		 * and we fall through to the standard completion check
		 * below for status / aborted handling. */
	}

	/*
	 * Final completion checks — reached normally or after timeout
	 * recovery.  Skip if a previous branch already set cb->result.
	 */
	if (atomic_read(&dw_chan->chip->dw->recovery_epoch) != start_epoch ||
	    READ_ONCE(dw_chan->hw_err)) {
		cb->result = -EIO;
		goto err_stats;
	}

	if (cb->result)
		goto err_stats;

	/* Re-read status after potential timeout-handler recovery. */
	status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);

	if (dw_chan->aborted) {
		/*
		 * Abort ISR completed the descriptor (done->done = true)
		 * but the transfer data is incomplete/corrupt.
		 * Abort ISR already sent enriched event to user-space
		 * (dx_pcie_enqueue_abort_event), so no duplicate here.
		 *
		 * Two terminate_all calls needed:
		 *   1st: sets EDMA_REQ_STOP
		 *   2nd: sees EDMA_REQ_STOP + CS=2, triggers ch_soft_reset
		 *         (engine_en cycle) to clear the HW error state.
		 * Without the 2nd call, channel stays in CS=2 permanently
		 * and all subsequent transfers fail with "skipping doorbell".
		 */
		dev_err(dev, "%s: transfer aborted by HW (CS=2)\n",
			dma_chan_name(chan));
		dw_chan->aborted = false;
		dw_edma_finalize_failed_transfer(info, chan, cookie, "abort", -EIO,
					      DW_EDMA_FAIL_TERMINATE_TWICE);
	} else if (status != DMA_COMPLETE) {
		if (status == DMA_ERROR) {
			f_cpl_err++;
		} else {
			f_cpl_bsy++;
		}
		dw_edma_finalize_failed_transfer(info, chan, cookie,
					      status == DMA_ERROR ? "completion_error" :
					      "completion_busy", -EIO,
					      DW_EDMA_FAIL_TERMINATE_ONCE);
	} else {
		cb->result = 0;
	}

err_stats:
	if (f_prp_cnt || f_sbt_cnt || f_tm_cnt || f_cpl_err || f_cpl_bsy) {
		dev_err(dev, "%s: failed - dmaengine_prep_slave_sg=%u, dma_submit_error=%u, timeout=%u, completion error status=%u, completion busy status=%u\n",
			dma_chan_name(chan), f_prp_cnt, f_sbt_cnt,
			f_tm_cnt, f_cpl_err, f_cpl_bsy);
		if (cb->result == 0) cb->result = -EIO;
	}

	/* Unmap scatter gather mapping.
	 * Use orig_nents (the count passed to dma_map_sg), not the
	 * coalesced sgt->nents returned by dma_map_sg, to ensure
	 * every original page receives proper cache maintenance.
	 * Skip unmap for pre-mapped buffers (dma_alloc_coherent). */
	dx_pcie_start_profile(PCIE_POST_PROCESS_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	if (!cb->pre_mapped && orig_nents > 0) {
		if (direction == DMA_DEV_TO_MEM) {
			dma_unmap_sg(dev, sgt->sgl, orig_nents, DMA_FROM_DEVICE);
		} else {
			dma_unmap_sg(dev, sgt->sgl, orig_nents, DMA_TO_DEVICE);
		}
	}
	dx_pcie_end_profile(PCIE_POST_PROCESS_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

err_alloc_descs:
	if (chan_lock_owned) {
		dx_dma_release_chan_ownership(chan_lock, &chan_lock_flags);
	}
	WRITE_ONCE(dw_chan->transfer_wq, NULL);	/* Unpublish waitqueue */
	sg_free_table(sgt);
	info->done = true; 

	return cb->result; // Return 0 on success, negative error code on failure
}

/* Optimized channel lookup with device ID pre-check */
static bool dw_edma_ch_filter_optimized(struct dma_chan *chan, void *filter_data)
{
	struct {
		int expected_dev_id;
		int expected_npu_id;
		char expected_name[32];
	} *filter_info = filter_data;
	const char *name;

	if (!chan || !filter_info || !chan->device || !chan->dev)
		return false;

	/* Quick device ID check first */
	if (chan->device->dev_id != filter_info->expected_dev_id) {
		return false;
	}

	name = dma_chan_name(chan);
	if (!name)
		return false;

	/* Then name comparison */
	if (strcmp(name, filter_info->expected_name) == 0) {
		dbg_tfr("DMA channel match found: %s (dev_id: %d)\n", 
			name, chan->device->dev_id);
		return true;
	}
	return false;
}

int dw_edma_dma_allocation(int dma_ch_id, int npu_id, struct dma_chan **_chan)
{
	dma_cap_mask_t mask;
	struct {
		int expected_dev_id;
		int expected_npu_id;
		char expected_name[32];
	} filter_info;
	int ret = 0;

#ifndef CONFIG_CMA_SIZE_MBYTES
	dbg_tfr("CMA not present/activated! Contiguous Memory may fail to be allocated\n");
#endif
	if (!_chan)
		return -EINVAL;

	/* Validate NPU ID range - only 0~3 are supported */
	if (npu_id < 0 || npu_id >= 4 || dma_ch_id < 0) {
		pr_err("Invalid NPU ID: %d (only 0~3 supported)\n", npu_id);
		return -EINVAL;
	}

	/* Protect concurrent allocation attempts with mutex */
	mutex_lock(&dma_allocation_mutex);

	/* Search dma channel */
	dbg_tfr("[DMA_ALLOC] dma%dchan%d allocation request\n", dma_ch_id, npu_id);

	if (!(*_chan)) {
		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);      /*Scatter Gather Mode*/
		dma_cap_set(DMA_CYCLIC, mask);     /*Cyclic Mode*/
		dma_cap_set(DMA_PRIVATE, mask);
		dma_cap_set(DMA_INTERLEAVE, mask);

		/* Try optimized filter first */
		filter_info.expected_dev_id = dma_ch_id;
		filter_info.expected_npu_id = npu_id;
		snprintf(filter_info.expected_name, sizeof(filter_info.expected_name),
			EDMA_CHANNEL_NAME, dma_ch_id, npu_id);

		(*_chan) = dma_request_channel(mask, dw_edma_ch_filter_optimized,
					&filter_info);
		if (*_chan) {
			dbg_tfr("[DMA_ALLOC] SUCCESS: dma%dchan%d allocated\n", dma_ch_id, npu_id);
		} else {
			pr_err("[DMA_ALLOC] FAILED: dma%dchan%d not found\n", dma_ch_id, npu_id);
			ret = -ENODEV;
		}
	}

	mutex_unlock(&dma_allocation_mutex);
	return ret;
}

void dw_edma_dma_deallocation(struct dma_chan **_chan)
{
	mutex_lock(&dma_allocation_mutex);

	if ((*_chan) != NULL && (*_chan)->client_count > 0) {
		dma_release_channel((*_chan));
		(*_chan) = NULL;
		dbg_tfr("DMA channel released\n");
	}

	mutex_unlock(&dma_allocation_mutex);
}

int dw_edma_run(struct dx_dma_io_cb * cb, struct dma_chan *dma_ch, int dev_n, int ch)
{
	struct dw_edma_info *info;
	int ret = 0;

	if (dev_n < 0 || dev_n >= MAX_DEV_NUM) {
		pr_err("[ERR] dw_edma_run: dev_n %d out of range\n", dev_n);
		return -EINVAL;
	}

	info = &test_info[dev_n][cb->npu_id][ch];

	mutex_lock(&info->lock);
	if (!info->done) {
		ret = -EBUSY;
		pr_err("DMA is running (BUSY:dev#%d, npu#%d, ch:%d) : %d\n", dev_n, cb->npu_id, ch, ret);
	} else {
		dbg_tfr("DMA is Ready to tranfser datas (dev#%d, npu#%d, ch:%d)\n", dev_n, cb->npu_id, ch);
		info->cb = cb;
		dx_pcie_start_profile(PCIE_KERNEL_DMA_TOTAL_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
		ret = dw_edma_sg_process(info, dma_ch);
		dx_pcie_end_profile(PCIE_KERNEL_DMA_TOTAL_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

	}
	mutex_unlock(&info->lock);

	return ret;
}

int dw_edma_thread_init(int dev_n)
{
	struct dw_edma_info *info;
	int i, j;

	if (dev_n < 0 || dev_n >= MAX_DEV_NUM)
		return -EINVAL;

	for (i = 0; i < EDMA_MAX_WR_CH; i++) {
		for (j = 0; j < EDMA_CH_END; j++) {
			info = &test_info[dev_n][i][j];
			dbg_init("Thread Init for dev#%d npu#%d [info:%p]\n", dev_n, i, info);
			info->init = true;
			info->dev_n = dev_n;
			info->done = true;
		}
	}
	clear_pcie_profile_info(0, 0, 0, 0, 0);

	return 0;
}

void dw_edma_thread_probe(void)
{
	int i, j, k;
	for (i = 0; i < MAX_DEV_NUM; i++) {
		for (j = 0; j < EDMA_MAX_WR_CH; j++) {
			for (k = 0; k < EDMA_CH_END; k++) {
				INIT_LIST_HEAD(&test_info[i][j][k].channels);
				mutex_init(&test_info[i][j][k].lock);
			}
		}
	}
}

void dw_edma_thread_exit(int dev_n)
{
	struct dw_edma_info *info;
	int i, j;

	if (dev_n < 0 || dev_n >= MAX_DEV_NUM)
		return;

	for (i = 0; i < EDMA_MAX_WR_CH; i++) {
		for (j = 0; j < EDMA_CH_END; j++) {
			info = &test_info[dev_n][i][j];
			mutex_lock(&info->lock);
			dbg_init("Thread Exit for dev#%d npu#%d [info:%p]\n", dev_n, i, info);
			info->init = false;
			mutex_unlock(&info->lock);
		}
	}
}
