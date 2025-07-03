// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#include <linux/io.h>
#include <linux/delay.h>
#include "dxrt_drv.h"

#define PCIE_COMM_FAIL_VAL  (0xFF)
/*
    true : ready
    false : not ready
*/
bool dx_get_flash_ready(dx_download_msg *msg, int timeout)
{
    bool ret = true;

    do {
        if ((timeout-- < 0) || (msg->sts == PCIE_COMM_FAIL_VAL)) {
            ret = false;
            break;
        }
        udelay(1);
    } while(msg->sts != DW_READY);
    // pr_info("%s:status:%d\n", __func__, msg->sts);

    return ret;
}

bool dx_get_flash_done(dx_download_msg *msg)
{
    int timeout = 1000; /* 1ms */
    bool ret = true;

    do {
        if ((timeout-- < 0) || (msg->sts == PCIE_COMM_FAIL_VAL)) {
            ret = false;
            break;
        }
        udelay(1);
    } while(msg->sts != DW_DONE);

    return ret;
}
/*
    fail : negative 
*/
int8_t dx_get_boot_step(dx_download_msg *msg)
{
    if(msg->bt_step <= DX_RTOS)
        return msg->bt_step;
    else
        return -1;
}

int8_t dx_get_dl_status(dx_download_msg *msg)
{
    return msg->sts;
}