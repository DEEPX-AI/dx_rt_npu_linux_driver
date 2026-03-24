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