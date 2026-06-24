// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2026 DeepX, Inc. and/or its affiliates.
 * PCIe link health monitoring and auto-recovery.
 *
 * Two-layer detection:
 *   Layer 1 (passive): ISR sentinel / polling_ack 0xFFFFFFFF check
 *   Layer 2 (active):  Periodic health worker polling PCI config space
 *
 * Multi-signal link health check (not single sentinel):
 *   1. PCI config read return code
 *   2. Vendor ID != 0xFFFF
 *   3. DLLLA (Data Link Layer Link Active) bit
 *   4. Negotiated speed/width sanity
 *
 * Recovery: worker enqueues, never executes inline.
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#include "dw-edma-core.h"
#include "dw-edma-v0-core.h"
#include "dx_link_health.h"
#include "dx_util.h"
#include "dx_pcie_api.h"
#include "dx_message.h"

/* Default for newly-probed devices.  Runtime sysfs toggles update this and
 * every currently-bound device.  Keep separate from background_recovery_paused:
 * that flag is reserved for AER/sysfs-reset/remove lifecycle ownership. */
static atomic_t g_link_health_default_enabled = ATOMIC_INIT(1);

/* ------------------------------------------------------------------ */
/* Link health detection                                              */
/* ------------------------------------------------------------------ */

/*
 * dx_pcie_check_link_health - Multi-signal link health check
 *
 * Uses PCI config read return code + Vendor ID + DLLLA to determine
 * link state.  Does NOT use a single 0xFFFFFFFF sentinel alone.
 *
 * Returns:  DX_LINK_UP, DX_LINK_DOWN, or DX_LINK_DEGRADED.
 * Called from: health worker, ISR (via schedule), recovery paths.
 */
enum dx_link_state dx_pcie_check_link_health(struct dw_edma *dw)
{
	struct pci_dev *pdev = dw->pdev;
	u16 vendor, linkstat;
	int ret;

	/* Signal 1: PCI config read — returns error if link is down */
	ret = pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor);
	if (ret != PCIBIOS_SUCCESSFUL || vendor == (u16)~0)
		return DX_LINK_DOWN;

	/* Signal 2: Link status — speed and width */
	ret = pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &linkstat);
	if (ret != PCIBIOS_SUCCESSFUL)
		return DX_LINK_DOWN;

	{
		u16 speed = linkstat & PCI_EXP_LNKSTA_CLS;
		u16 width = (linkstat & PCI_EXP_LNKSTA_NLW)
			    >> PCI_EXP_LNKSTA_NLW_SHIFT;

		/* speed=0 or width=0 means link is not trained */
		if (speed == 0 || width == 0)
			return DX_LINK_DOWN;
	}

	/* Signal 3: DLLLA (Data Link Layer Link Active).
	 * DLLLA is an optional PCIe capability — some host Root Ports or
	 * pre-silicon platforms report it as 0 even when the link is fully
	 * functional.  Treating DLLLA=0 as DEGRADED causes spurious warnings
	 * and, worse, makes dx_pcie_validate_link() return 1 after every
	 * AER/reset recovery — which in turn lets dx_dma_full_reinit fire
	 * LINK_EV_UP with link_state=DEGRADED, clobbering dev->mem_addr
	 * in the RT module.  Since speed/width already passed, we trust
	 * those signals and ignore DLLLA. */
	(void)linkstat; /* DLLLA intentionally not consulted */

	return DX_LINK_UP;
}

/*
 * dx_pcie_validate_link - Post-reinit link validation
 *
 * Polls link health for up to DX_LINK_VALIDATION_TIMEOUT_MS.
 * Returns 0 on LINK_UP, -ETIMEDOUT if link doesn't come up,
 * 1 if link is degraded (functional but warn).
 */
int dx_pcie_validate_link(struct dw_edma *dw)
{
	unsigned long timeout = jiffies +
		msecs_to_jiffies(DX_LINK_VALIDATION_TIMEOUT_MS);
	enum dx_link_state ls;

	do {
		ls = dx_pcie_check_link_health(dw);
		if (ls == DX_LINK_UP)
			return 0;
		if (ls == DX_LINK_DEGRADED)
			return 1;
		msleep(20);
	} while (time_before(jiffies, timeout));

	return -ETIMEDOUT;
}

/* ------------------------------------------------------------------ */
/* Quiesce / Recovery                                                 */
/* ------------------------------------------------------------------ */

/*
 * dx_dma_quiesce_for_link_down - Lightweight quiesce without MMIO writes
 *
 * Unlike dx_dma_quiesce_channels() which calls dw_edma_v0_core_off()
 * (6x writel to BAR), this version skips MMIO writes because the link
 * is already down — writel would either silently fail or cause a
 * platform-specific fault (ARM64 synchronous external abort).
 */
void dx_dma_quiesce_for_link_down(struct dw_edma *dw)
{
	int i;

	/* Block new submissions */
	atomic_set(&dw->dev_state, DX_DEV_AER_RESET);
	atomic_set(&dw->link_state, DX_LINK_DOWN);

	/* Bump epoch so in-flight threads detect stale context */
	atomic_inc(&dw->recovery_epoch);

	/* Full barrier: epoch + state visible before hw_err / wq reads */
	smp_mb();

	/* Drain shadow prebuild work after publishing link-down so any queued
	 * worker sees the new state and exits before helper-channel MMIO. */
	if (dw->shadow_wq)
		flush_workqueue(dw->shadow_wq);

	/* Do NOT call dw_edma_v0_core_off() — link is down, writel unsafe */

	/* Wake all sleeping transfer threads via hw_err */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		struct dw_edma_chan *chan = &dw->chan[i];
		wait_queue_head_t *wq;

		WRITE_ONCE(chan->hw_err, true);
		wq = READ_ONCE(chan->transfer_wq);
		if (wq)
			wake_up(wq);
	}

	pci_err(dw->pdev, "link-down quiesce: all channels marked hw_err\n");

	/* Notify RT module so it can set dev->recovering, wake message
	 * waiters with -ENODATA, and clear stale response queues.
	 * No AER/reset callback fires for EP-initiated link flaps, so
	 * this is the only signal the RT module gets. */
	dx_pcie_notify_link_event(dw->idx, DX_PCIE_LINK_EV_DOWN);
}

/*
 * dx_dma_recovery_rate_ok - Check if recovery is within rate limits
 *
 * Prevents recovery storm: max DX_RECOVERY_MAX_IN_WINDOW recoveries
 * within DX_RECOVERY_WINDOW_SECS.
 */
static bool dx_dma_recovery_rate_ok(struct dw_edma *dw)
{
	unsigned long now = jiffies;
	unsigned long window = DX_RECOVERY_WINDOW_SECS * HZ;

	/* Reset window if expired */
	if (time_after(now, dw->recovery_window_start + window)) {
		dw->recovery_window_start = now;
		dw->recovery_count_in_window = 0;
	}

	if (dw->recovery_count_in_window >= DX_RECOVERY_MAX_IN_WINDOW) {
		pci_warn(dw->pdev,
			 "recovery rate limit: %d recoveries in %ds window\n",
			 dw->recovery_count_in_window,
			 DX_RECOVERY_WINDOW_SECS);
		return false;
	}

	return true;
}

static void dx_dma_recovery_rate_record_attempt(struct dw_edma *dw)
{
	dw->last_recovery_jiffies = jiffies;
	dw->recovery_count_in_window++;
}

/*
 * dx_dma_full_reinit - Full HW reinit after link recovery
 *
 * Mirrors dw_edma_v0_core_pcie_reset() sequence:
 * PCI config restore, MSI, ch_pwr, engine enable, IMWR, iATU, unmask.
 * Then validates link status before transitioning to LIVE.
 */
int dx_dma_full_reinit(struct dw_edma *dw)
{
	struct pci_dev *pdev = dw->pdev;
	int ret, i;

	pci_info(pdev, "full reinit: restoring PCI + eDMA state\n");

	atomic_set(&dw->link_state, DX_LINK_RECOVERING);

	/* Use the no-SBR restore path.  The link is already up — issuing
	 * another SBR would reset the EP again, causing a recovery loop.
	 * Just restore PCI config + reprogram eDMA registers. */
	ret = dw_edma_v0_core_edma_restore(dw);
	if (ret) {
		pci_err(pdev, "full reinit: edma_restore failed (%d)\n", ret);
		atomic_set(&dw->link_state, DX_LINK_DOWN);
		return ret;
	}

	/* Trigger dxrt module re-init on next ioctl */
	dx_pcie_set_init_completed(dw->idx);

	/* Post-link validation */
	ret = dx_pcie_validate_link(dw);
	if (ret < 0) {
		pci_err(pdev, "full reinit: link validation failed\n");
		atomic_set(&dw->link_state, DX_LINK_DOWN);
		return -ETIMEDOUT;
	}
	if (ret == 1) {
		pci_warn(pdev, "full reinit: link degraded (speed/width below expected)\n");
		atomic_set(&dw->link_state, DX_LINK_DEGRADED);
	} else {
		atomic_set(&dw->link_state, DX_LINK_UP);
	}

	/* Clear hw_err on all channels */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		dw->chan[i].hw_err = false;
		dw->chan[i].aborted = false;
		/*
		 * Reset SW channel state to IDLE.  A channel whose in-flight
		 * transfer was aborted by the reset is still marked
		 * EDMA_ST_BUSY; the eDMA engine has been restored but this SW
		 * field is not.  Without this, dw_edma_alloc_chan_resources()
		 * rejects the channel with -EBUSY, so dma_request_channel()
		 * returns NULL and every reopen fails ("dmaNchanM not found").
		 */
		dw->chan[i].status = EDMA_ST_IDLE;
	}

	/* Transition to LIVE */
	atomic_set(&dw->dev_state, DX_DEV_LIVE);

	pci_info(pdev, "full reinit: done (link_state=%d)\n",
		 atomic_read(&dw->link_state));

	/* Notify RT module so it can clear dev->recovering, clear stale
	 * response queues, reset response_pending, and wake any waiters
	 * still blocked on response_wq so they can see recovering==0. */
	dx_pcie_notify_link_event(dw->idx, DX_PCIE_LINK_EV_UP);

	return 0;
}

static bool dx_dma_background_recovery_blocked(struct dw_edma *dw)
{
	return atomic_read(&dw->dev_state) == DX_DEV_REMOVING ||
		!atomic_read(&dw->link_health_enabled) ||
		atomic_read(&dw->background_recovery_paused);
}

/*
 * dx_pcie_ep_config_was_reset - Detect an endpoint that silently reset itself.
 *
 * After an EP-side power cycle (or any reset that re-trains the link without
 * the host observing a DOWN edge) the PCIe link comes back UP on its own
 * — config space is readable again — but the endpoint's config has reverted
 * to power-on defaults: BARs unassigned and Memory Space / Bus Master cleared
 * in the Command register.  In that state BAR MMIO reads return 0xFFFFFFFF
 * even though the link is perfectly fine; the host only needs to restore
 * config space and reprogram iATU/eDMA (no SBR/FLR required).
 *
 * At probe the kernel enables Memory Space via pci_enable_device(), so a
 * cleared PCI_COMMAND_MEMORY bit while the link is up is a reliable signal
 * that the endpoint reset underneath us.
 *
 * Return: true if the link is up but the EP lost its host-programmed config.
 */
static bool dx_pcie_ep_config_was_reset(struct dw_edma *dw)
{
	struct pci_dev *pdev = dw->pdev;
	u16 vendor, cmd;

	/* Link must be up (config space accessible) for this to apply.
	 * A down link is handled by the normal DOWN->UP recovery path. */
	if (pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor) != PCIBIOS_SUCCESSFUL ||
	    vendor == (u16)~0)
		return false;

	if (pci_read_config_word(pdev, PCI_COMMAND, &cmd) != PCIBIOS_SUCCESSFUL)
		return false;

	/* Memory Space disabled while link is up => config reset to defaults. */
	return !(cmd & PCI_COMMAND_MEMORY);
}

/* ------------------------------------------------------------------ */
/* Worker functions                                                   */
/* ------------------------------------------------------------------ */

/*
 * dx_dma_recovery_work_fn - Deferred recovery work item
 *
 * Enqueued by health worker or ISR sentinel when link-down is detected
 * and subsequently link-up is observed.  Worker performs the heavy
 * reinit; the health-check only enqueues, never executes inline.
 */
static void dx_dma_recovery_work_fn(struct work_struct *work)
{
	struct dw_edma *dw = container_of(work, struct dw_edma, recovery_work);
	enum dx_link_state ls;
	int ret;

	/* Bail if teardown/reset already owns recovery sequencing. */
	if (dx_dma_background_recovery_blocked(dw))
		return;

	/* Verify link is actually up before attempting reinit */
	ls = dx_pcie_check_link_health(dw);
	if (ls == DX_LINK_DOWN) {
		pci_info(dw->pdev,
			 "recovery_work: link still down, skipping reinit\n");
		return;
	}

	/* Rate-limit check */
	if (!dx_dma_recovery_rate_ok(dw)) {
		/* Enter cooldown: silence further scheduling for a while.
		 * The health worker consults rate_limit_cooldown_until
		 * before re-enqueuing this work, so we stop the log storm. */
		dw->rate_limit_cooldown_until =
			jiffies + DX_RECOVERY_COOLDOWN_SECS * HZ;
		pci_err(dw->pdev,
			"recovery_work: rate limited, cooling down for %ds — "
			"manual remove/rescan may be needed\n",
			DX_RECOVERY_COOLDOWN_SECS);
		return;
	}

	/* AER/reset may have taken ownership after the worker started. */
	if (dx_dma_background_recovery_blocked(dw))
		return;

	/* Count attempts, not just successes, so repeated failures also cool down. */
	dx_dma_recovery_rate_record_attempt(dw);

	if (dx_dma_background_recovery_blocked(dw))
		return;

	ret = dx_dma_full_reinit(dw);
	if (ret) {
		pci_err(dw->pdev,
			"recovery_work: reinit failed (%d), will retry\n",
			ret);
		/* link_state already set to DOWN by full_reinit,
		 * health worker will re-schedule recovery on next check */
	}
}

/*
 * dx_link_health_work_fn - Periodic link health monitor
 *
 * Runs every DX_HEALTH_INTERVAL_MS.  Checks multi-signal link health.
 * On DX_HEALTH_FAIL_THRESHOLD consecutive failures → quiesce + mark LINK_DOWN.
 * On link-up after previous LINK_DOWN → enqueue recovery work.
 */
static void dx_link_health_work_fn(struct work_struct *work)
{
	struct dw_edma *dw = container_of(to_delayed_work(work),
					  struct dw_edma, health_work);
	enum dx_link_state ls;
	int prev_link;

	/* Early exit if teardown/remove already paused background recovery. */
	if (dx_dma_background_recovery_blocked(dw))
		return;

	prev_link = atomic_read(&dw->link_state);
	ls = dx_pcie_check_link_health(dw);

	switch (ls) {
	case DX_LINK_DOWN:
		dw->health_fail_count++;
		if (dw->health_fail_count >= DX_HEALTH_FAIL_THRESHOLD &&
		    prev_link != DX_LINK_DOWN) {
			pci_err(dw->pdev,
				"health: link down detected (%d consecutive fails)\n",
				dw->health_fail_count);
			dx_dma_quiesce_for_link_down(dw);
		}
		break;

	case DX_LINK_UP:
	case DX_LINK_DEGRADED:
		if (prev_link == DX_LINK_DOWN) {
			/* Link came back — enqueue recovery, don't do it inline.
			 *
			 * Two guards:
			 *   1. cooldown — silence after rate-limit hit, so we
			 *      don't log-storm when EP keeps flapping.
			 *   2. work_busy — avoid double-enqueue of work that
			 *      hasn't finished yet. */
			dw->health_fail_count = 0;

			if (time_before(jiffies,
					dw->rate_limit_cooldown_until)) {
				/* Silent during cooldown; user already saw
				 * the rate-limit error once. */
				break;
			}

			if (work_busy(&dw->recovery_work)) {
				/* recovery_work already queued or running */
				break;
			}

			pci_info(dw->pdev,
				 "health: link restored, scheduling recovery\n");
			schedule_work(&dw->recovery_work);
		} else {
			dw->health_fail_count = 0;
			if (ls == DX_LINK_DEGRADED &&
			    prev_link != DX_LINK_DEGRADED) {
				pci_warn(dw->pdev,
					 "health: link degraded (speed/width reduced)\n");
				atomic_set(&dw->link_state, DX_LINK_DEGRADED);
			} else if (dx_pcie_ep_config_was_reset(dw) &&
				   !time_before(jiffies,
						dw->rate_limit_cooldown_until) &&
				   !work_busy(&dw->recovery_work)) {
				/* Link is up but the endpoint reset itself
				 * (e.g. power cycle) without us catching the
				 * DOWN edge.  Its config reverted to power-on
				 * defaults (BARs unassigned, Memory Space off),
				 * so BAR MMIO reads 0xFFFFFFFF.  Restore config +
				 * reprogram iATU/eDMA — no SBR/FLR needed.
				 *
				 * Quiesce first (bump recovery_epoch, set hw_err,
				 * wake D-state DMA threads, notify RT link-down)
				 * before scheduling reinit.  Without this, any
				 * in-flight DMA thread blocked in
				 * dw_edma_wait_done(false) never wakes because
				 * neither recovery_epoch nor hw_err changed —
				 * full_reinit only clears hw_err, it never sets
				 * it — leaving those threads in uninterruptible
				 * D-state even after recovery completes. */
				pci_info(dw->pdev,
					 "health: endpoint config reset detected (link up), scheduling reinit\n");
				dx_dma_quiesce_for_link_down(dw);
				schedule_work(&dw->recovery_work);
			}
		}
		break;

	default:
		break;
	}

	/* Reschedule unless teardown/reset or sysfs disable blocked work. */
	if (!dx_dma_background_recovery_blocked(dw))
		schedule_delayed_work(&dw->health_work,
				      msecs_to_jiffies(DX_HEALTH_INTERVAL_MS));
}

/* ------------------------------------------------------------------ */
/* Lifecycle (called from probe / remove)                             */
/* ------------------------------------------------------------------ */

/*
 * dx_link_health_init - Initialize link health fields and work structs
 *
 * Must be called during probe, before dx_link_health_start().
 */
void dx_link_health_init(struct dw_edma *dw)
{
	atomic_set(&dw->link_state, DX_LINK_UNKNOWN);
	atomic_set(&dw->link_health_enabled,
		   atomic_read(&g_link_health_default_enabled));
	atomic_set(&dw->background_recovery_paused, 0);
	dw->health_fail_count = 0;
	dw->last_recovery_jiffies = 0;
	dw->recovery_count_in_window = 0;
	dw->recovery_window_start = jiffies;
	dw->rate_limit_cooldown_until = 0;
	INIT_DELAYED_WORK(&dw->health_work, dx_link_health_work_fn);
	INIT_WORK(&dw->recovery_work, dx_dma_recovery_work_fn);
}

/*
 * dx_link_health_start - Start periodic health monitoring
 *
 * Called after probe completes and device is LIVE.
 */
void dx_link_health_start(struct dw_edma *dw)
{
	atomic_set(&dw->link_state, DX_LINK_UP);
	atomic_set(&dw->background_recovery_paused, 0);
	if (!atomic_read(&dw->link_health_enabled)) {
		pci_info(dw->pdev, "link health monitor disabled by sysfs\n");
		return;
	}
	schedule_delayed_work(&dw->health_work,
			      msecs_to_jiffies(DX_HEALTH_INTERVAL_MS));
	pci_info(dw->pdev, "link health monitor started (interval=%dms)\n",
		 DX_HEALTH_INTERVAL_MS);
}

/*
 * dx_link_health_stop - Stop health monitoring synchronously
 *
 * Must be called during remove BEFORE resource teardown.
 * cancel_delayed_work_sync ensures the worker has fully completed
 * before returning — no race with teardown.
 */
void dx_link_health_stop(struct dw_edma *dw)
{
	atomic_set(&dw->background_recovery_paused, 1);
	cancel_delayed_work_sync(&dw->health_work);
	cancel_work_sync(&dw->recovery_work);
}

void dx_link_health_set_enabled(struct dw_edma *dw, bool enable)
{
	if (enable) {
		atomic_set(&dw->link_health_enabled, 1);
		dw->health_fail_count = 0;
		if (atomic_read(&dw->dev_state) != DX_DEV_REMOVING &&
		    !atomic_read(&dw->background_recovery_paused)) {
			schedule_delayed_work(&dw->health_work,
					      msecs_to_jiffies(DX_HEALTH_INTERVAL_MS));
		}
		pci_info(dw->pdev, "link health monitor enabled\n");
	} else {
		atomic_set(&dw->link_health_enabled, 0);
		cancel_delayed_work_sync(&dw->health_work);
		cancel_work_sync(&dw->recovery_work);
		pci_info(dw->pdev, "link health monitor disabled\n");
	}
}

bool dx_link_health_is_enabled(struct dw_edma *dw)
{
	return atomic_read(&dw->link_health_enabled) != 0;
}

void dx_link_health_set_default_enabled(bool enable)
{
	atomic_set(&g_link_health_default_enabled, enable ? 1 : 0);
}

bool dx_link_health_default_enabled(void)
{
	return atomic_read(&g_link_health_default_enabled) != 0;
}
