// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef _DX_MESSAGE_H
#define _DX_MESSAGE_H

#include <linux/types.h>
#include <linux/io.h>

typedef struct {
    uint32_t  req_id;
    uint32_t  inf_time;
    uint16_t  argmax;
    uint16_t  model_type;
    int32_t   status;
    int32_t   ppu_filter_num;
    int32_t   dma_ch;
    uint64_t  data;
    uint64_t  base;
    uint32_t  offset;
    uint32_t  size;
} dx_pcie_response_t;

typedef struct
{
    struct list_head list;
    dx_pcie_response_t response;
} dx_pcie_response_list_t;

typedef struct {
    uint32_t err_code;
    /* System Infomation power / temperature, etc,,,, */
} dx_pcie_dev_err_t;

struct dx_pcie_msg {
    void __iomem                *response[3];
    void __iomem                *errors;
    void __iomem                *irq_status;
    dx_pcie_response_list_t     responses[3];
    dx_pcie_dev_err_t           err_response;
    spinlock_t                  responses_lock[3];
    spinlock_t                  err_lock;
};

void dx_pcie_enqueue_response(u32 dev_id, int dma_ch);
int dx_pcie_message_init(int dev_id);
void dx_pcie_enqueue_error_response(u32 dev_id, uint32_t err_code);

#endif
