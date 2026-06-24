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
 * Active PING timeout (ms).
 *
 * Once DLMSG says mailbox is ready, the driver sends DX_PCIE_PING and
 * waits for ack within this window.  Must be short — by this point
 * the RTOS mailbox path is supposed to be healthy.
 */
#define DXRT_PING_TIMEOUT_MS_DEFAULT            500

#endif /* _DXRT_RECOVERY_CFG_H */
