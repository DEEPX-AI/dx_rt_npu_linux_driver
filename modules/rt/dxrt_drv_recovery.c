// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver — Recovery state machine
 *
 * This file owns everything related to post-recovery coordination:
 *   - per-dxdev state fields (dxrt_dev_state_t) and their transitions
 *   - PCIe link-event callback  (bridge from dx_dma link-health worker)
 *   - recovery readiness workqueue  (probes FW after link-up)
 *   - PERM_FAIL sliding-window guard
 *   - /sys/class/dxrt/dxrtN/recovery/ attributes
 *   - module_param tunables (include/dxrt_recovery_cfg.h for defaults)
 *
 * See: docs/RECOVERY_OVERVIEW.md
 *      docs/recovery/RECOVERY_IMPLEMENTATION_PLAN.md
 */
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/atomic.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

#include "dxrt_drv.h"
#include "dxrt_recovery_cfg.h"

#if IS_ACCELERATOR

#define DXRT_FW_REBOOT_ACK_WINDOW_MS 2000u

/* ------------------------------------------------------------------ */
/* Module params (defaults in include/dxrt_recovery_cfg.h)            */
/* ------------------------------------------------------------------ */

unsigned int dxrt_perm_fail_window_ms = DXRT_PERM_FAIL_WINDOW_MS_DEFAULT;
module_param_named(perm_fail_window_ms, dxrt_perm_fail_window_ms, uint, 0644);
MODULE_PARM_DESC(perm_fail_window_ms,
"Sliding window (ms) for counting recovery attempts toward PERM_FAIL.");

unsigned int dxrt_perm_fail_max_retries = DXRT_PERM_FAIL_MAX_RETRIES_DEFAULT;
module_param_named(perm_fail_max_retries, dxrt_perm_fail_max_retries, uint, 0644);
MODULE_PARM_DESC(perm_fail_max_retries,
"Max recoveries within perm_fail_window_ms before entering PERM_FAIL.");

bool dxrt_auto_reset_on_fw_timeout = DXRT_AUTO_RESET_ON_FW_TIMEOUT_DEFAULT;
module_param_named(auto_reset_on_fw_timeout, dxrt_auto_reset_on_fw_timeout, bool, 0644);
MODULE_PARM_DESC(auto_reset_on_fw_timeout,
"Auto-reset on FW command timeout (default off).");

unsigned int dxrt_dlmsg_ready_timeout_ms = DXRT_DLMSG_READY_TIMEOUT_MS_DEFAULT;
module_param_named(dlmsg_ready_timeout_ms, dxrt_dlmsg_ready_timeout_ms, uint, 0644);
MODULE_PARM_DESC(dlmsg_ready_timeout_ms,
"Passive DLMSG mailbox-ready wait timeout after link-up (ms).");

unsigned int dxrt_ping_timeout_ms = DXRT_PING_TIMEOUT_MS_DEFAULT;
module_param_named(ping_timeout_ms, dxrt_ping_timeout_ms, uint, 0644);
MODULE_PARM_DESC(ping_timeout_ms,
"Active PING/PONG liveness probe timeout (ms).");

/* ------------------------------------------------------------------ */
/* String tables                                                      */
/* ------------------------------------------------------------------ */

static const char *dev_state_str(int s)
{
    switch (s) {
        case DXRT_STATE_READY:         return "ready";
        case DXRT_STATE_RECOVERING:    return "recovering";
        case DXRT_STATE_TRANSPORT_OK:  return "transport_ok";
        case DXRT_STATE_FW_HANG:       return "fw_hang";
        case DXRT_STATE_WAITING_USER:  return "waiting_user";
        case DXRT_STATE_PERM_FAIL:     return "perm_fail";
        default:                       return "unknown";
    }
}

static const char *reason_str(int r)
{
    switch (r) {
        case DX_RECOVERY_REASON_NONE:         return "none";
        case DX_RECOVERY_REASON_LINK_FLAP:    return "link_flap";
        case DX_RECOVERY_REASON_FW_TIMEOUT:   return "fw_timeout";
        case DX_RECOVERY_REASON_CPU_RESET:    return "cpu_reset";
        case DX_RECOVERY_REASON_FW_UPDATE:    return "fw_update";
        case DX_RECOVERY_REASON_FW_UPDATE_NODLMSG: return "fw_update_nodlmsg";
        default:                              return "unknown";
    }
}

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static void emit_recovery_event(struct dxdev *dev, uint32_t subcode,
uint32_t reason)
{
    if (!dev)
        return;

    /* FW update reboot is an expected maintenance window.  Keep the
     * internal recovery gate, but do not surface normal STARTED/DONE
     * transitions to user-space as recovery events. */
    if (reason == DX_RECOVERY_REASON_FW_UPDATE &&
        (subcode == DX_RECOVERY_STARTED || subcode == DX_RECOVERY_DONE))
        return;

    dx_pcie_enqueue_recovery_event(dev->id, subcode, reason,
    (uint32_t)atomic_read(&dev->recovery_count),
    (uint32_t)atomic_read(&dev->recovery_fail_count),
    (uint32_t)atomic_read(&dev->dev_state));
}

/*
 * perm_fail_check - Sliding-window guard.
 * Invoked on each recovery failure.  Returns true when the device
 * just entered PERM_FAIL (caller should stop retrying).
 */
static bool perm_fail_check(struct dxdev *dev)
{
    unsigned long now = jiffies;
    unsigned long window = msecs_to_jiffies(dxrt_perm_fail_window_ms);

    if (time_after(now, dev->perm_fail_window_start + window)) {
        dev->perm_fail_window_start = now;
        dev->perm_fail_count_in_window = 0;
    }
    dev->perm_fail_count_in_window++;

    if (dev->perm_fail_count_in_window >= dxrt_perm_fail_max_retries) {
        atomic_set(&dev->dev_state, DXRT_STATE_PERM_FAIL);
        pr_err(MODULE_NAME "%d: PERM_FAIL — %u recoveries in %u ms\n",
                dev->id,
                dev->perm_fail_count_in_window,
                dxrt_perm_fail_window_ms);
        emit_recovery_event(dev, DX_RECOVERY_PERM_FAIL,
            atomic_read(&dev->last_recovery_reason));
        return true;
    }
    return false;
}

static int dxrt_wait_fw_reboot_mailbox_cycle(struct dxdev *dev,
                                             int start_epoch,
                                             const char *tag)
{
    unsigned long total_deadline;
    unsigned long ack_deadline;
    bool saw_drop = false;

    if (!dev->dl)
        return 0;

    total_deadline = jiffies + msecs_to_jiffies(dxrt_dlmsg_ready_timeout_ms);
    ack_deadline = jiffies + msecs_to_jiffies(DXRT_FW_REBOOT_ACK_WINDOW_MS);
    if (time_after(ack_deadline, total_deadline))
        ack_deadline = total_deadline;

    pr_info(MODULE_NAME "%d: %s recovery — watching MAILBOX_READY edge "
            "(ack_window=%ums, total=%ums)\n",
            dev->id, tag, DXRT_FW_REBOOT_ACK_WINDOW_MS,
            dxrt_dlmsg_ready_timeout_ms);

    for (;;) {
        if (atomic_read(&dev->recovery_epoch) != start_epoch ||
            atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK)
            return -EAGAIN;
        if (!dx_dlmsg_mailbox_ready(dev->dl)) {
            saw_drop = true;
            break;
        }
        if (time_after_eq(jiffies, ack_deadline))
            break;
        msleep(20);
    }

    if (!saw_drop) {
        pr_err(MODULE_NAME "%d: %s recovery — MAILBOX_READY never "
               "cleared within %ums (ready_flags=0x%x)\n",
               dev->id, tag, DXRT_FW_REBOOT_ACK_WINDOW_MS,
               dx_dlmsg_ready_flags(dev->dl));
        return -ETIMEDOUT;
    }

    pr_info(MODULE_NAME "%d: %s recovery — MAILBOX_READY cleared; "
            "waiting for republish\n",
            dev->id, tag);

    for (;;) {
        if (atomic_read(&dev->recovery_epoch) != start_epoch ||
            atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK)
            return -EAGAIN;
        if (dx_dlmsg_mailbox_ready(dev->dl))
            return 0;
        if (time_after_eq(jiffies, total_deadline))
            break;
        msleep(20);
    }

    pr_err(MODULE_NAME "%d: %s recovery — MAILBOX_READY did not "
           "return within %ums (ready_flags=0x%x)\n",
            dev->id, tag, dxrt_dlmsg_ready_timeout_ms,
           dx_dlmsg_ready_flags(dev->dl));
    return -ETIMEDOUT;
}

/*
 * dxrt_wait_fw_reboot_legacy_settle - Legacy (<2.7.0) FW reboot settle wait.
 *
 * Old firmware is observably silent after a reboot: it publishes neither the
 * DLMSG ready_flags block, nor the PING/PONG mailbox protocol, nor
 * bt_step=DX_RTOS (all three are 2.7.0+ features — the romcode/bootloader only
 * advance bt_step as far as DX_2ND_BOOT, and the legacy RTOS app never sets
 * DX_RTOS).  With no readiness signal to observe, simply give the device a
 * fixed window to complete its reboot, then let the caller mark recovery done.
 * The normal IDENTIFY path has no readiness gate, so user-space works once the
 * old RTOS is back up.
 *
 * Return: 0 after the settle window elapsed, -EAGAIN if the recovery
 *         epoch/state changed underneath us.
 */
static int dxrt_wait_fw_reboot_legacy_settle(struct dxdev *dev,
                                             int start_epoch,
                                             const char *tag)
{
    unsigned long deadline =
        jiffies + msecs_to_jiffies(dxrt_dlmsg_ready_timeout_ms);

    pr_info(MODULE_NAME "%d: %s recovery — no readiness signal; "
            "settling %ums for reboot\n",
            dev->id, tag, dxrt_dlmsg_ready_timeout_ms);

    for (;;) {
        if (atomic_read(&dev->recovery_epoch) != start_epoch ||
            atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK)
            return -EAGAIN;
        if (time_after_eq(jiffies, deadline))
            return 0;
        msleep(50);
    }
}

/*
 * fw_ping_probe - Lightweight RTOS mailbox liveness check.
 *
 * Caller MUST have verified dx_dlmsg_mailbox_ready(dev->dl) == true.
 * Sends DXRT_CMD_PCIE / DX_PCIE_PING with a random seq and validates
 * the PONG reply (magic + echoed seq).  No NPU/DDR state is touched.
 * Returns 0 on PONG match, -ETIMEDOUT on silence, -EIO on bad reply,
 * -EAGAIN if recovery state changes mid-flight, -ENODEV on link gone.
 */
static atomic_t dxrt_ping_seq_gen = ATOMIC_INIT(0);

static int fw_ping_probe(struct dxdev *dev, unsigned int timeout_ms,
                         int start_epoch)
{
    unsigned long deadline;
    dx_pcie_ping_req_t  req = {0};
    dx_pcie_pong_resp_t resp = {0};
    u32 seq;
    int ret = -ETIMEDOUT;

    if (!dev || !dev->msg)
        return -ENODEV;

    if (atomic_read(&dev->recovery_epoch) != start_epoch ||
        atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK)
        return -EAGAIN;

    seq = (u32)atomic_inc_return(&dxrt_ping_seq_gen);
    req.magic = DX_PCIE_PING_MAGIC;
    req.seq   = seq;

    mutex_lock(&dev->msg_lock);

    if (atomic_read(&dev->recovery_epoch) != start_epoch ||
        atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK) {
        mutex_unlock(&dev->msg_lock);
        return -EAGAIN;
    }

    dx_memcpy_toio32(DXRT_MSG_DATA_ADDR(dev), &req, sizeof(req));
    dx_write32(dev->msg, dxrt_device_message_t, cmd, DXRT_CMD_PCIE);
    dx_write32(dev->msg, dxrt_device_message_t, sub_cmd, DX_PCIE_PING);
    dx_write32(dev->msg, dxrt_device_message_t, size, sizeof(req));
    dx_write32(dev->msg, dxrt_device_message_t, ack, 0);

    dx_pcie_notify_msg_to_device(dev->id);

    deadline = jiffies + msecs_to_jiffies(timeout_ms);
    while (time_before(jiffies, deadline)) {
        u32 ack = dx_read32(dev->msg, dxrt_device_message_t, ack);

        if (atomic_read(&dev->recovery_epoch) != start_epoch ||
            atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK) {
            ret = -EAGAIN;
            break;
        }
        if (ack == (u32)~0u) { ret = -ENODEV; break; }
        if (ack == 1)        { ret = 0;       break; }
        usleep_range(500, 1000);
    }

    if (ret == 0) {
        u32 sz = dx_read32(dev->msg, dxrt_device_message_t, size);

        if (sz < sizeof(resp) ||
            sz >= sizeof(dxrt_device_message_t)) {
            ret = -EIO;
        } else {
            dx_memcpy_fromio32(&resp, DXRT_MSG_DATA_ADDR(dev),
                               sizeof(resp));
            if (resp.magic != DX_PCIE_PONG_MAGIC || resp.seq != seq)
                ret = -EIO;
        }
    }

    mutex_unlock(&dev->msg_lock);

    if (ret == 0) {
        pr_info(MODULE_NAME "%d: PING ok seq=%u flags=0x%x "
                "fw=0x%x uptime=%ums\n",
                dev->id, resp.seq, resp.ready_flags,
                resp.fw_ver, resp.uptime_ms);
    } else {
        pr_warn_ratelimited(MODULE_NAME "%d: PING failed (%d) seq=%u\n",
                            dev->id, ret, seq);
    }
    return ret;
}

/* ------------------------------------------------------------------ */
/* Link-event bridge                                                  */
/* ------------------------------------------------------------------ */

void dxrt_link_event_notify(u32 dev_id, dx_pcie_link_event_t ev, void *data)
{
    struct dxdev *dev = (struct dxdev *)data;
    int i;

    if (!dev || dev->type == DX_STD)
        return;

    if (atomic_read(&dev->dev_state) == DXRT_STATE_PERM_FAIL) {
        pr_warn_ratelimited(MODULE_NAME "%d: ignoring link event %d in PERM_FAIL\n",
            dev_id, ev);
        return;
    }

    switch (ev) {
    case DX_PCIE_LINK_EV_DOWN:
        pr_info(MODULE_NAME "%d: link-down — blocking ioctls, waking waiters\n",
            dev_id);

        atomic_set(&dev->dev_state, DXRT_STATE_RECOVERING);
        atomic_set(&dev->last_recovery_reason,
            DX_RECOVERY_REASON_LINK_FLAP);

        atomic_set(&dev->recovering, 1);
        atomic_inc(&dev->recovery_epoch);
        smp_mb();

        cancel_delayed_work(&dev->recovery_ready_work);

        clear_queue_list(dev);
        dx_pcie_clear_response_queue(dev_id);
        dx_pcie_clear_event_response(dev_id);

        dxrt_clear_all_pending(dev);

        for (i = 0; i <= MAX_PCIE_CH_NUM; i++)
            wake_up_interruptible(&dev->response_wq[i]);
        wake_up_interruptible(&dev->event_wq);

        emit_recovery_event(dev, DX_RECOVERY_STARTED,
            DX_RECOVERY_REASON_LINK_FLAP);

        /*
         * Do not emit DMA-abort-shaped ERROR events for link-health recovery.
         * Upgraded dxrtd consumes the RECOVERY event above as the client cleanup
         * signal.  Reusing ERR_PCIE_DMA_CH<n>_ABORT for link flaps makes the
         * service print a false "PCIe-DMA HW Abort" and can trigger duplicate
         * service-owned recovery after the driver already handled the link flap.
         */
        break;

    case DX_PCIE_LINK_EV_UP:
        pr_info(MODULE_NAME "%d: link-up — scheduling readiness probe\n",
            dev_id);

        /*
         * Refresh BAR-derived iomem pointers before scheduling the
         * readiness worker.  An AER fatal + remove/re-probe cycle (or any
         * other path that destroys and re-creates struct dw_edma) leaves
         * dev->msg/dl/request_queue* pointing into a freed pcim_iomap
         * region.  The lazy refresh in dxrt_dev_ioctl() only fires on a
         * userspace ioctl — but recovery_ready_work runs from a kernel worker
         * with no fd open, so it would dereference the stale ioremap and
         * page-fault inside dx_dlmsg_mailbox_ready().
         */
        dxrt_bind_pcie_resources(dev, false);

        dx_pcie_set_init_completed(dev_id);

        atomic_set(&dev->dev_state, DXRT_STATE_TRANSPORT_OK);
        schedule_delayed_work(&dev->recovery_ready_work, 0);
        break;

    default:
        pr_warn(MODULE_NAME "%d: unknown link event %d\n", dev_id, ev);
        break;
    }
}

/*
 * dxrt_kick_fw_reboot_recovery - Synthesize recovery for FW-only reboot sources.
 *
 * Background: a firmware-initiated cpu_reset_with_reason() is invisible
 * to the PCIe link (LTSSM stays UP, BARs unchanged), so neither
 * dx_dma's reset_prepare/done nor the link-health worker fires.  But
 * the EP is effectively rebooted: RTOS task state, NPU init, response
 * queues, and in-flight session tracking on the dxrtd side are all
 * stale.  Without intervention dxrtd waits forever on responses that
 * will never arrive (proc_id mismatch after FW restart).
 *
 * Resolution: after the kernel acks the DX_PCIE_CPU_RESET ioctl back
 * to userspace, drive a CPU_RESET recovery sequence without claiming a
 * physical link transition.  This:
 *   - bumps recovery_epoch and parks dev_state in RECOVERING
 *   - clears request/response/event queues and wakes all waiters
 *   - emits DX_RECOVERY_STARTED (reason=CPU_RESET) so dxrtd flushes
 *     its session state
 *   - schedules recovery_ready_work which observes the MAILBOX_READY
 *     true→false→true edge for CPU reset, runs a PING probe,
 *     then transitions to READY and emits DX_RECOVERY_DONE
 *
 * BAR-derived ioremap pointers are NOT refreshed here: cpu_reset does
 * not destroy the PCI device, so dev->msg/dl/request_queue* remain
 * valid (unlike the AER + remove/re-probe path).
 */
static void dxrt_kick_fw_reboot_recovery(struct dxdev *dev, uint32_t reason,
                                         const char *tag)
{
    u32 dev_id;
    int i;

    if (!dev || dev->type == DX_STD)
        return;

    dev_id = dev->id;

    if (atomic_read(&dev->dev_state) == DXRT_STATE_PERM_FAIL) {
        pr_warn_ratelimited(MODULE_NAME "%d: %s kick ignored in PERM_FAIL\n",
            dev_id, tag);
        return;
    }

    pr_info(MODULE_NAME "%d: %s kick — starting FW reboot recovery\n",
        dev_id, tag);

    /* ----- reset-start cleanup phase ----- */
    atomic_set(&dev->dev_state, DXRT_STATE_RECOVERING);
    atomic_set(&dev->last_recovery_reason, reason);
    atomic_set(&dev->recovering, 1);
    atomic_inc(&dev->recovery_epoch);
    smp_mb();

    cancel_delayed_work(&dev->recovery_ready_work);

    clear_queue_list(dev);
    dx_pcie_clear_response_queue(dev_id);
    dx_pcie_clear_event_response(dev_id);

    dxrt_clear_all_pending(dev);

    for (i = 0; i <= MAX_PCIE_CH_NUM; i++)
        wake_up_interruptible(&dev->response_wq[i]);
    wake_up_interruptible(&dev->event_wq);

    emit_recovery_event(dev, DX_RECOVERY_STARTED, reason);

    /* ----- FW reboot observation phase ----- */
    dx_pcie_set_init_completed(dev_id);

    atomic_set(&dev->dev_state, DXRT_STATE_TRANSPORT_OK);
    schedule_delayed_work(&dev->recovery_ready_work, 0);
}

void dxrt_kick_cpu_reset_recovery(struct dxdev *dev)
{
    dxrt_kick_fw_reboot_recovery(dev, DX_RECOVERY_REASON_CPU_RESET,
        "cpu_reset");
}

void dxrt_kick_fw_update_recovery(struct dxdev *dev)
{
    dxrt_kick_fw_reboot_recovery(dev, DX_RECOVERY_REASON_FW_UPDATE,
        "fw_update");
}

/*
 * Legacy FW (< 2.7.0) publishes no host-observable readiness signal at all
 * (no DLMSG ready_flags, no PING/PONG, no bt_step=DX_RTOS).  Use a dedicated
 * reason so the readiness worker skips every readiness probe and instead waits
 * a fixed settle window for the reboot to complete.
 */
void dxrt_kick_fw_update_recovery_nodlmsg(struct dxdev *dev)
{
    dxrt_kick_fw_reboot_recovery(dev, DX_RECOVERY_REASON_FW_UPDATE_NODLMSG,
        "fw_update_nodlmsg");
}

/* ------------------------------------------------------------------ */
/* Recovery readiness probe                                           */
/* ------------------------------------------------------------------ */

void dxrt_recovery_ready_work_fn(struct work_struct *work)
{
    struct dxdev *dev = container_of(to_delayed_work(work),
        struct dxdev, recovery_ready_work);
    int ch;
    int start_epoch;
    int ret;
    bool skip_ping = false;

    if (!dev)
        return;
    if (atomic_read(&dev->dev_state) == DXRT_STATE_PERM_FAIL)
        return;
    start_epoch = atomic_read(&dev->recovery_epoch);

    if (atomic_read(&dev->recovery_epoch) != start_epoch ||
        atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK)
        return;

    /*
     * Step 1 (passive): wait for RTOS to publish DLMSG mailbox-ready.
     * CPU reset and FW update are special: the link stays up and FW acks before
     * it clears readiness, so a stale MAILBOX_READY=1 from the old RTOS image is
     * not proof that the new image is ready. Require the observable true→false→true
     * edge before sending PING.
     */
    if (dev->dl) {
        int reason = atomic_read(&dev->last_recovery_reason);

        if (reason == DX_RECOVERY_REASON_FW_UPDATE_NODLMSG) {
            /*
             * Legacy FW (< 2.7.0) is observably silent after a reboot: it
             * publishes neither the DLMSG ready_flags block, nor PING/PONG,
             * nor bt_step=DX_RTOS (all three are 2.7.0+ features).  There is
             * nothing to wait on, so give the device a fixed settle window to
             * finish rebooting, then skip the PING probe (the old image would
             * never answer it).  The normal IDENTIFY path has no readiness
             * gate, so user-space recovers once the old RTOS is back up.
             */
            ret = dxrt_wait_fw_reboot_legacy_settle(dev, start_epoch,
                                                    reason_str(reason));
            if (ret == -EAGAIN)
                return;
            pr_info(MODULE_NAME "%d: legacy FW settle window elapsed; "
                    "marking ready\n", dev->id);
            skip_ping = true;
        } else {
            if (reason == DX_RECOVERY_REASON_CPU_RESET ||
                reason == DX_RECOVERY_REASON_FW_UPDATE) {
                ret = dxrt_wait_fw_reboot_mailbox_cycle(dev, start_epoch,
                                                        reason_str(reason));
                if (ret == -EAGAIN)
                    return;
                if (ret != 0)
                    goto fw_hang;
            } else {
                bool ready = dx_dlmsg_wait_mailbox_ready(dev->dl,
                                                         dxrt_dlmsg_ready_timeout_ms);
                if (atomic_read(&dev->recovery_epoch) != start_epoch ||
                    atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK)
                    return;
                if (!ready) {
                    pr_warn(MODULE_NAME "%d: DLMSG not mailbox-ready within %ums "
                            "(bt_step=%d ready_flags=0x%x)\n",
                            dev->id, dxrt_dlmsg_ready_timeout_ms,
                            dx_get_boot_step(dev->dl),
                            dx_dlmsg_ready_flags(dev->dl));
                    goto fw_hang;
                }
            }
            pr_info(MODULE_NAME "%d: DLMSG mailbox-ready (ready_flags=0x%x)\n",
                    dev->id, dx_dlmsg_ready_flags(dev->dl));
        }
    }

    /* Step 2 (active): PING — proves RTOS message task actually responds.
     * Skipped for legacy FW (< 2.7.0), which does not implement PING/PONG;
     * its reboot was covered by the fixed settle window above. */
    if (!skip_ping) {
        ret = fw_ping_probe(dev, dxrt_ping_timeout_ms, start_epoch);
        if (atomic_read(&dev->recovery_epoch) != start_epoch ||
            atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK)
            return;
        if (ret != 0) {
            atomic_inc(&dev->recovery_fail_count);
            pr_err(MODULE_NAME "%d: PING probe failed (%d)\n",
                   dev->id, ret);
            goto fw_hang;
        }
    }

    if (atomic_read(&dev->recovery_epoch) != start_epoch ||
        atomic_read(&dev->dev_state) != DXRT_STATE_TRANSPORT_OK)
        return;

    atomic_inc(&dev->recovery_count);
    WRITE_ONCE(dev->last_recovery_jiffies, (u64)jiffies);

    atomic_set(&dev->recovering, 0);
    smp_mb();
    atomic_set(&dev->dev_state, DXRT_STATE_READY);

    for (ch = 0; ch <= MAX_PCIE_CH_NUM; ch++)
        wake_up_interruptible(&dev->response_wq[ch]);
    wake_up_interruptible(&dev->event_wq);

    emit_recovery_event(dev, DX_RECOVERY_DONE,
        atomic_read(&dev->last_recovery_reason));
    pr_info(MODULE_NAME "%d: recovery DONE (count=%u, reason=%s, mem=0x%llx/0x%llx, dma_ch=%u)\n",
        dev->id,
        (uint32_t)atomic_read(&dev->recovery_count),
        reason_str(atomic_read(&dev->last_recovery_reason)),
        dev->mem_addr,
        dev->mem_size,
        dev->num_dma_ch);
    return;

fw_hang:
    atomic_set(&dev->dev_state, DXRT_STATE_FW_HANG);
    atomic_set(&dev->recovering, 0);
    smp_mb();
    dxrt_clear_all_pending(dev);
    for (ch = 0; ch <= MAX_PCIE_CH_NUM; ch++)
        wake_up_interruptible(&dev->response_wq[ch]);
    wake_up_interruptible(&dev->event_wq);
    emit_recovery_event(dev, DX_RECOVERY_FW_HANG,
        atomic_read(&dev->last_recovery_reason));

    if (perm_fail_check(dev))
        return;
}

/* ------------------------------------------------------------------ */
/* Sysfs: /sys/class/dxrt/dxrtN/recovery/                             */
/* ------------------------------------------------------------------ */

static struct dxdev *dev_to_dxdev(struct device *d)
{
    return dev_get_drvdata(d);
}

static ssize_t link_state_show(struct device *d, struct device_attribute *a,
       char *buf)
{
    struct dxdev *dev = dev_to_dxdev(d); (void)a;
    return dev ? sysfs_emit(buf, "%s\n",
        dev_state_str(atomic_read(&dev->dev_state)))
        : -ENODEV;
}

static ssize_t recovery_count_show(struct device *d,
   struct device_attribute *a, char *buf)
{
    struct dxdev *dev = dev_to_dxdev(d); (void)a;
    return dev ? sysfs_emit(buf, "%u\n",
        atomic_read(&dev->recovery_count))
        : -ENODEV;
}

static ssize_t recovery_fail_count_show(struct device *d,
struct device_attribute *a, char *buf)
{
    struct dxdev *dev = dev_to_dxdev(d); (void)a;
    return dev ? sysfs_emit(buf, "%u\n",
        atomic_read(&dev->recovery_fail_count))
        : -ENODEV;
}

static ssize_t last_recovery_jiffies_show(struct device *d,
  struct device_attribute *a, char *buf)
{
    struct dxdev *dev = dev_to_dxdev(d); (void)a;
    return dev ? sysfs_emit(buf, "%llu\n",
        (unsigned long long)READ_ONCE(dev->last_recovery_jiffies))
        : -ENODEV;
}

static ssize_t last_recovery_reason_show(struct device *d,
 struct device_attribute *a, char *buf)
{
    struct dxdev *dev = dev_to_dxdev(d); (void)a;
    return dev ? sysfs_emit(buf, "%s\n",
        reason_str(atomic_read(&dev->last_recovery_reason)))
        : -ENODEV;
}

static ssize_t config_show(struct device *d, struct device_attribute *a,
   char *buf)
{
    (void)d; (void)a;
    return sysfs_emit(buf,
    "dlmsg_ready_timeout_ms=%u\n"
    "ping_timeout_ms=%u\n"
    "perm_fail_window_ms=%u\n"
    "perm_fail_max_retries=%u\n"
    "auto_reset_on_fw_timeout=%u\n",
    dxrt_dlmsg_ready_timeout_ms,
    dxrt_ping_timeout_ms,
    dxrt_perm_fail_window_ms,
    dxrt_perm_fail_max_retries,
    dxrt_auto_reset_on_fw_timeout ? 1 : 0);
}

static DEVICE_ATTR_RO(link_state);
static DEVICE_ATTR_RO(recovery_count);
static DEVICE_ATTR_RO(recovery_fail_count);
static DEVICE_ATTR_RO(last_recovery_jiffies);
static DEVICE_ATTR_RO(last_recovery_reason);
static DEVICE_ATTR_RO(config);

static struct attribute *dxrt_recovery_attrs[] = {
    &dev_attr_link_state.attr,
    &dev_attr_recovery_count.attr,
    &dev_attr_recovery_fail_count.attr,
    &dev_attr_last_recovery_jiffies.attr,
    &dev_attr_last_recovery_reason.attr,
    &dev_attr_config.attr,
    NULL,
};

static const struct attribute_group dxrt_recovery_group = {
    .name  = "recovery",
    .attrs = dxrt_recovery_attrs,
};

/* ------------------------------------------------------------------ */
/* Lifecycle                                                          */
/* ------------------------------------------------------------------ */

void dxrt_recovery_state_init(struct dxdev *dev)
{
    atomic_set(&dev->dev_state, DXRT_STATE_READY);
    atomic_set(&dev->recovery_count, 0);
    atomic_set(&dev->recovery_fail_count, 0);
    atomic_set(&dev->last_recovery_reason, DX_RECOVERY_REASON_NONE);
    WRITE_ONCE(dev->last_recovery_jiffies, 0ULL);
    dev->perm_fail_window_start = jiffies;
    dev->perm_fail_count_in_window = 0;
    INIT_DELAYED_WORK(&dev->recovery_ready_work, dxrt_recovery_ready_work_fn);
}

void dxrt_recovery_state_deinit(struct dxdev *dev)
{
    cancel_delayed_work_sync(&dev->recovery_ready_work);
}

int dxrt_sysfs_attach(struct dxdev *dev)
{
    int ret;

    if (!dev || !dev->dev)
        return -EINVAL;

    dev_set_drvdata(dev->dev, dev);

    ret = sysfs_create_group(&dev->dev->kobj, &dxrt_recovery_group);
    if (ret) {
        pr_err(MODULE_NAME "%d: sysfs_create_group failed (%d)\n",
            dev->id, ret);
        return ret;
    }
    return 0;
}

void dxrt_sysfs_detach(struct dxdev *dev)
{
    if (!dev || !dev->dev)
        return;
    sysfs_remove_group(&dev->dev->kobj, &dxrt_recovery_group);
}

#endif /* IS_ACCELERATOR */
