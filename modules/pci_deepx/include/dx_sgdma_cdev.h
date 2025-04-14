// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef _DX_SGDMA_CDEV_H
#define _DX_SGDMA_CDEV_H

#include "dx_cdev.h"

/* PCIe internal API */
extern void dx_cdev_sgdma_init(struct dx_dma_cdev *xcdev);
extern ssize_t dx_sgdma_write_user(struct dw_edma *dw, char __user *buf, u64 pos, size_t count, int npu_id, bool npu_run);
extern ssize_t dx_sgdma_read_user(struct dw_edma *dw, char __user *buf, u64 pos, size_t count, int npu_id);
#endif