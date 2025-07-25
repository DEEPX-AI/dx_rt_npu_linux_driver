// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#ifndef __DXRT_DRV_NPU_H
#define __DXRT_DRV_NPU_H
#include <linux/types.h>
#include <linux/sizes.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include "dxrt_drv_common.h"

#if IS_STANDALONE
#include "npu_reg_sys_DX_V3.h"
#include "npu_reg_dma_DX_V3.h"
#include "npu_reg_debug_DX_V3.h"
typedef dxSYSTEM_t npu_reg_sys_t;
typedef dxDMA_t npu_reg_dma_t;
#endif
struct dxdev;
struct _dxrt_response_t;
struct dxnpu {
    int id;
    struct dxdev *dx;
    struct device *dev;
    uint32_t clock_khz;
    uint32_t reg_base_addr;
    volatile void __iomem *reg_base;
#if IS_STANDALONE
    uint32_t reg_ctrl_addr;
    volatile void __iomem *reg_ctrl;
    npu_reg_sys_t *reg_sys;
    npu_reg_dma_t *reg_dma;
#endif
    dma_addr_t dma_buf_addr;
    size_t dma_buf_size;
    void *dma_buf;
    int irq_num;
    int irq_event;
    // spinlock_t status_lock;
    // int status;
    spinlock_t irq_event_lock;
    wait_queue_head_t irq_wq;
    uint32_t default_values[3];
    struct _dxrt_response_t *response;
    int (*init)(struct dxnpu*);
    int (*prepare_inference)(struct dxnpu*);
    int (*run)(struct dxnpu*, void *);
    int (*reg_dump)(struct dxnpu*);
    int (*deinit)(struct dxnpu*);
};
struct dxnpu_cfg {
    uint32_t clock_khz;
    uint32_t reg_base_addr;
    size_t dma_buf_size;    
    int irq_num;
    uint32_t default_values[3];
    int (*init)(struct dxnpu*);
    int (*prepare_inference)(struct dxnpu*);
    int (*run)(struct dxnpu*, void *);
    int (*reg_dump)(struct dxnpu*);
    int (*deinit)(struct dxnpu*);
};
typedef struct dxnpu dxnpu_t;
typedef struct dxnpu_cfg dxnpu_cfg_t;

struct dxnpu *dxrt_npu_init(void *);
void dxrt_npu_deinit(void *dxdev_);

int dx_v3_npu_init(dxnpu_t *npu);
int dx_v3_npu_prepare_inference(dxnpu_t *npu);
int dx_v3_npu_run(dxnpu_t *npu, void*);
int dx_v3_npu_reg_dump(dxnpu_t *npu);
int dx_v3_npu_deinit(dxnpu_t *npu);
#endif // __DXRT_DRV_NPU_H
