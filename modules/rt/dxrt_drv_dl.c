// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include "dxrt_drv.h"

#define PCIE_COMM_FAIL_VAL  (0xFF)
/*
    true : ready
    false : not ready
*/
bool dx_get_flash_ready(dx_download_msg __iomem *msg, int timeout)
{
    bool ret = true;
    void __iomem *sts = DX_FIELD_ADDR(msg, dx_download_msg, sts);

    do {
        if ((timeout-- < 0) || (readb(sts) == PCIE_COMM_FAIL_VAL)) {
            ret = false;
            break;
        }
        udelay(1);
    } while(readb(sts) != DW_READY);

    return ret;
}

bool dx_get_flash_done(dx_download_msg __iomem *msg)
{
    int timeout = 1000; /* 1ms */
    bool ret = true;
    void __iomem *sts = DX_FIELD_ADDR(msg, dx_download_msg, sts);

    do {
        if ((timeout-- < 0) || (readb(sts) == PCIE_COMM_FAIL_VAL)) {
            ret = false;
            break;
        }
        udelay(1);
    } while(readb(sts) != DW_DONE);

    return ret;
}
/*
    fail : negative 
*/
int8_t dx_get_boot_step(dx_download_msg __iomem *msg)
{
    uint8_t step = dx_read8(msg, dx_download_msg, bt_step);
    if(step <= DX_RTOS)
        return step;
    else
        return -1;
}

int8_t dx_get_dl_status(dx_download_msg __iomem *msg)
{
    return dx_read8(msg, dx_download_msg, sts);
}

/* ----------------------------------------------------------------- */
/* Boot-Aware passive readiness helpers                              */
/*                                                                   */
/* These read the RTOS-published portion of DLMSG (offset 0x20..)    */
/* and must never block on the mailbox.  Callers MUST gate any       */
/* mailbox write (IDENTIFY/PING/normal cmd) on                       */
/* dx_dlmsg_mailbox_ready() returning true.                          */
/* ----------------------------------------------------------------- */

bool dx_dlmsg_ready_valid(dx_download_msg __iomem *msg)
{
    uint32_t magic;
    uint16_t version;

    if (!msg)
        return false;

    magic = dx_read32(msg, dx_download_msg, ready_magic);
    if (magic == 0xFFFFFFFFu || magic != DX_DLMSG_READY_MAGIC)
        return false;

    /* ready_version is uint16_t; readw via field addr */
    version = readw(DX_FIELD_ADDR(msg, dx_download_msg, ready_version));
    if (version == 0 || version > DX_DLMSG_READY_VERSION)
        return false;

    return true;
}

uint32_t dx_dlmsg_ready_flags(dx_download_msg __iomem *msg)
{
    if (!dx_dlmsg_ready_valid(msg))
        return 0;
    return dx_read32(msg, dx_download_msg, ready_flags);
}

bool dx_dlmsg_mailbox_ready(dx_download_msg __iomem *msg)
{
    uint8_t step;
    uint32_t flags;

    if (!msg)
        return false;

    /* Boot step must be RTOS */
    step = dx_read8(msg, dx_download_msg, bt_step);
    if (step != DX_RTOS)
        return false;

    /* RTOS readiness block must be valid + required gate bit set.
     * REQUIRED is MAILBOX_READY only (FW sets it LAST, after all init).
     */
    if (!dx_dlmsg_ready_valid(msg))
        return false;

    flags = dx_read32(msg, dx_download_msg, ready_flags);
    return (flags & DX_DLMSG_READY_REQUIRED) == DX_DLMSG_READY_REQUIRED;
}

bool dx_dlmsg_wait_mailbox_ready(dx_download_msg __iomem *msg,
                                 unsigned int timeout_ms)
{
    unsigned long deadline;

    if (!msg)
        return false;

    deadline = jiffies + msecs_to_jiffies(timeout_ms);
    for (;;) {
        if (dx_dlmsg_mailbox_ready(msg))
            return true;
        if (time_after_eq(jiffies, deadline))
            return false;
        if (msleep_interruptible(20))
            return false;
    }
}
