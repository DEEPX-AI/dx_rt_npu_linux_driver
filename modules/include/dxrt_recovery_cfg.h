/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DeepX RT Driver — Recovery Configuration
 *
 * Tunable defaults for the recovery state machine.  All values are
 * module_param-backed in dxrt_drv_recovery.c, so operators can override
 * at load time without rebuilding.  Change defaults here if the baseline
 * needs to move.
 *
 * See: docs/recovery/RECOVERY_IMPLEMENTATION_PLAN.md  §9
 */
#ifndef _DXRT_RECOVERY_CFG_H
#define _DXRT_RECOVERY_CFG_H

/*
 * PERM_FAIL window (ms) and max retries.
 *
 * If FW_HANG failures counted by the RT readiness path occur within
 * `perm_fail_window_ms`, the device is marked PERM_FAIL.  General ioctls
 * return -EIO; an explicit DXRT_CMD_RECOVERY or module reload is required
 * to leave PERM_FAIL in the current implementation.
 */
#define DXRT_PERM_FAIL_WINDOW_MS_DEFAULT        60000
#define DXRT_PERM_FAIL_MAX_RETRIES_DEFAULT      5

/*
 * Auto reset on FW timeout (bool).
 *
 * Current behavior: the parameter is exposed, but no auto-reset path is
 * wired.  On FW command timeout (Case 1), dxrt_polling_ack() enqueues
 * ERR_FW_TIMEOUT and returns -ETIMEDOUT; it does not set WAITING_USER or
 * trigger full_reinit directly.
 */
#define DXRT_AUTO_RESET_ON_FW_TIMEOUT_DEFAULT   0

/*
 * Passive DLMSG mailbox-ready wait (ms).
 *
 * After PCIe link-up, RTOS may still be cold-booting (ROM → 2nd-boot
 * → RTOS init).  The driver passively polls the DLMSG ready_flags in
 * BAR3 message-RAM until DX_DLMSG_READY_REQUIRED is set, with this
 * timeout.  Must cover worst-case RTOS boot.
 */
#define DXRT_DLMSG_READY_TIMEOUT_MS_DEFAULT     5000

/*
 * FW reboot DLMSG mailbox-ready wait (ms).
 *
 * FW update, CPU reset and DXRT_CMD_RECOVERY reboot the RTOS with the link
 * up.  boot2nd may retrain LPDDR on that path (every runtime DFS level,
 * twice after an update from older FW, plus the LPDDR5X clear), so the
 * reboot can take far longer than a plain RTOS restart.
 */
#define DXRT_FW_REBOOT_READY_TIMEOUT_MS_DEFAULT 30000

/*
 * Late readiness watch (ms).
 *
 * After a readiness timeout lands in FW_HANG, keep polling DLMSG for this
 * long and resume (PING -> READY) once the FW turns mailbox-ready, so a
 * slow boot does not need a driver reload.  0 disables the watch.
 */
#define DXRT_LATE_READY_WINDOW_MS_DEFAULT       120000

/*
 * Active PING timeout (ms).
 *
 * Once DLMSG says mailbox is ready, the driver sends DX_PCIE_PING and
 * waits for ack within this window.  Must be short — by this point
 * the RTOS mailbox path is supposed to be healthy.
 */
#define DXRT_PING_TIMEOUT_MS_DEFAULT            500

/*
 * Stable-check ack timeout (ms).
 *
 * The FW runs PcieHostDriverInit() for DX_PCIE_STABLE_CHECK before it acks,
 * which takes about 200 ms on M1. The driver holds the mailbox until that ack,
 * so the next command cannot take the stable-check ack as its own.
 */
#define DXRT_STABLE_CHECK_ACK_TIMEOUT_MS        1000

#endif /* _DXRT_RECOVERY_CFG_H */
