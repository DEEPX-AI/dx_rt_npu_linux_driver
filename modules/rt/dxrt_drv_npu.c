// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#include "dxrt_drv.h"

struct dxnpu_cfg npu_cfg = {
#if DEVICE_VARIANT==DX_V3
    /* DX-V3 */
    .clock_khz = 800000,//74250,
    .default_values = {0x3C0, 0x200, 0x2C0},
    .init = dx_v3_npu_init,
    .prepare_inference = dx_v3_npu_prepare_inference,
    .run = dx_v3_npu_run,
    .reg_dump = dx_v3_npu_reg_dump,
    .deinit = dx_v3_npu_deinit,    
#else
    0,
#endif
};

struct dxnpu *dxrt_npu_init(void *dxdev_)
{
    struct dxdev *dxdev = (struct dxdev *)dxdev_;
    struct dxnpu *npu = NULL;
    pr_debug( "%s\n", __func__);
    if (dxdev->type == DX_STD) {
        int variant = dxdev->variant;
        int idx = variant - 100;
        int i;
        npu = kmalloc(sizeof(struct dxnpu), GFP_KERNEL);
        pr_debug( "%s: %d, %d\n", __func__, variant, idx);
        if (!npu)
        {
            pr_err( "%s: failed to allocate memory\n", __func__);
            return NULL;
        }
        npu->clock_khz = npu_cfg.clock_khz;
        for(i=0;i<3;i++) npu->default_values[i] = npu_cfg.default_values[i];
        npu->dev = dxdev->dev;
        npu->init = npu_cfg.init;
        npu->prepare_inference = npu_cfg.prepare_inference;
        npu->run = npu_cfg.run;
        npu->reg_dump = npu_cfg.reg_dump;
        npu->deinit = npu_cfg.deinit;        
        npu->dx = dxdev;
        npu->response = &dxdev->response;
        // npu->status = 0;
        spin_lock_init(&npu->irq_event_lock);
        // setup from platform device
        {
            struct platform_device *pdev = dxdev->pdev;
            struct device_node *np = pdev->dev.of_node;
            struct resource *res;
            const __be32 *prop;
            
            res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
            if (res==NULL) {
                pr_err( "%s: failed to find IO resource for npu.\n", __func__);
                return NULL;
            }
            npu->reg_ctrl_addr = res->start;

            res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
            if (res==NULL) {
                pr_err( "%s: failed to find IO resource for npu.\n", __func__);
                return NULL;
            }
            npu->reg_base_addr = res->start;
            
            npu->irq_num = platform_get_irq(pdev, 0);
            if (npu->irq_num < 0) {
                pr_err( "%s: failed to find IRQ number for npu.\n", __func__);
                return NULL;
            }
            {
                prop = of_get_property(np, "device-id", NULL);
                if (prop==NULL)
                {
                    pr_err( "%s: failed to find device-id for npu.\n", __func__);
                    return NULL;
                }
                npu->id = be32_to_cpup(prop);
            }
            {
                prop = of_get_property(np, "dma-buf-size", NULL);
                if (prop==NULL)
                {
                    pr_err( "%s: failed to find dma-buf-size for npu.\n", __func__);
                    return NULL;
                }
                npu->dma_buf_size = be32_to_cpup(prop);
            }
        }
    }
    if (npu) {
        npu->init(npu);
    }
    return npu;
}
void dxrt_npu_deinit(void *dxdev_)
{
    // struct dxdev *dxdev = (struct dxdev *)dxdev_;
    struct dxnpu *npu = NULL;    
    pr_debug( "%s\n", __func__);
    if (npu) {
        npu->deinit(npu);
        kfree(npu);
    }
}