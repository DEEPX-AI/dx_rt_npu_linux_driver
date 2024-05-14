// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */

#include "dxrt_drv_npu.h"
#include "dxrt_drv.h"

struct enx_en677_cdma_desc *dma_desc;

inline void npu_reg_write(volatile void __iomem *base, uint32_t addr, uint32_t val)
{
    pr_debug("write: %x, %x\n", addr, val);
    iowrite32(val, base + addr);
}
inline void npu_reg_write_mask(volatile void __iomem *base, uint32_t addr, uint32_t val, uint32_t mask, uint32_t bit_offset)
{
    uint32_t read_val = ioread32(base + addr);
    read_val &= ~mask;
    read_val |= (val << bit_offset) & mask;
    pr_debug("write_mask: %x, %x\n", addr, read_val);
    iowrite32(read_val, base + addr);
}
inline uint32_t npu_reg_read(volatile void __iomem *base, uint32_t addr)
{
    pr_debug("read: %x\n", addr);
    return ioread32(base + addr);
}
inline uint32_t npu_reg_read_mask(volatile void __iomem *base, uint32_t addr, uint32_t mask, uint32_t bit_offset)
{
    uint32_t read_val = ioread32(base + addr);
    pr_debug("read_mask: %x, %x, %x\n", addr, mask, bit_offset);
    return (read_val & mask) >> bit_offset;
}

#if 0
static irqreturn_t npu_irq_handler(int irq, void *data)
{
    unsigned long flags;    
    dxnpu_t *npu = (dxnpu_t*)data;
    struct dxdev *dx = npu->dx;
    volatile void __iomem *reg = npu->reg_base;
    dxrt_response_t response;
    pr_debug("npu%d irq: %d\n", npu->id, READ_SYSTEM_STATUS(reg)); // this log causes worse latency.
    // clear IRQ
    WRITE_SYSTEM_STATUS(reg, 0x7);
    // get inference time
    response.inf_time = READ_DEBUG_STAMP1_CLK_CNT(reg) / npu->clock_khz;
    WRITE_DEBUG_STAMP0_STAMP_EN(reg, 0);
    // get response
    response.req_id = READ_SYSTEM_SWREG0(reg);    
    response.status = 0; // TODO
    // set npu state to idle
    WRITE_SYSTEM_SWREG1(reg, 0x0);
    // enqueue response
    {
        dxrt_response_list_t *entry;
        entry = kmalloc(sizeof(dxrt_response_list_t), GFP_KERNEL);
        if(!entry)
        {
            pr_err("npu%d irq: Failed to allocate memory for response\n", npu->id);
        }
        else
        {
            *((dxrt_response_t*)&entry->response) = response;
            spin_lock_irqsave(&dx->responses_lock, flags);
            list_add_tail(&entry->list, &dx->responses.list);
            spin_unlock_irqrestore(&dx->responses_lock, flags);
        }
    }
    // wakeup waitqueue
	spin_lock_irqsave(&npu->irq_event_lock, flags);
	npu->irq_event = 1;	
    wake_up_interruptible(&npu->irq_wq);
	spin_unlock_irqrestore(&npu->irq_event_lock, flags);
	return IRQ_HANDLED;
}
#else
static irqreturn_t npu_irq_handler(int irq, void *data)
{
    unsigned long flags;
    dxnpu_t *npu = (dxnpu_t*)data;    
    dxrt_response_t *response = npu->response;
    volatile void __iomem *reg = npu->reg_base;
    pr_debug("npu%d irq: %d\n", npu->id, READ_SYSTEM_STATUS(reg)); // this log causes worse latency.
    // clear IRQ
    WRITE_SYSTEM_STATUS(reg, 0x7);
    // get inference time
    response->inf_time = (uint32_t)((uint64_t)READ_DEBUG_STAMP1(reg) * 1000 / npu->clock_khz);
    WRITE_DEBUG_STAMP0_STAMP_EN(reg, 0);
    // get response
    response->req_id = READ_SYSTEM_SWREG0(reg);
    // response.status = 0; // TODO
    // set npu state to idle
    WRITE_SYSTEM_SWREG1(reg, 0x0);
    // wakeup waitqueue
	spin_lock_irqsave(&npu->irq_event_lock, flags);
	npu->irq_event = 1;	
	spin_unlock_irqrestore(&npu->irq_event_lock, flags);
    wake_up_interruptible(&npu->irq_wq);
	return IRQ_HANDLED;
}
#endif
int dx_l1_npu_init(dxnpu_t *npu)
{
    int ret, i;
    volatile void __iomem *reg;
    pr_debug("%s\n", __func__);
    /* Register */
    npu->reg_base = ioremap(npu->reg_base_addr, 0x10200);
    if(!npu->reg_base)
    {
        pr_err("Failed to map npu registers\n");
        return -ENOMEM;
    }
    npu->reg_sys = (volatile npu_reg_sys_t*)npu->reg_base;
    npu->reg_dma = (volatile npu_reg_dma_t*)(npu->reg_base + 0x10000);
    reg = npu->reg_base;
    /* IRQ */
    WRITE_SYSTEM_STATUS(reg, 0x7);
    ret = request_irq(npu->irq_num, npu_irq_handler, 0, "deepx-npu", (void*)npu);
    // ret = request_irq(npu->irq_num, npu_irq_handler, 0, "deepx-npu",NULL);
    if(ret)
    {
        pr_err("Failed to request IRQ %d.\n", npu->irq_num);
        return ret;
    }
    npu->irq_event = 0;
    init_waitqueue_head(&npu->irq_wq);
    /* DMA Buffer */    
    npu->dma_buf = dma_alloc_coherent(npu->dev, npu->dma_buf_size, &npu->dma_buf_addr, GFP_KERNEL);
    if (!npu->dma_buf) {
        pr_err("Failed to allocate dma_buf.\n");
        return -1;
    }        
    pr_info("dma_buf : virt 0x%p, phys 0x%llx, virt_to_phys 0x%lx, size 0x%lx\n", 
        npu->dma_buf, npu->dma_buf_addr, virt_to_phys(npu->dma_buf), npu->dma_buf_size);
    /* Eyenix CDMA */
	// EYENIX CDMA ALLOC
	dma_desc = enx_en677_cdma_alloc("NPU CDMA");
	if (!dma_desc) {
		pr_err("CDMA Error\n");
		return -1;
	}
	else{
		pr_info("Alloc CDMA for NPU\n");
	}
    /* Init */
    // npu_prepare_inference(npu);
	pr_info("    npu%d @ %x: CLOCK: %dKHz, IRQ: %d, ID: %X, MODE: %X, AXI_CFG: %X\n", 
        npu->id, npu->reg_base_addr, npu->clock_khz, npu->irq_num, READ_SYSTEM_ID(npu->reg_base),
        READ_SYSTEM_MODE(npu->reg_base), READ_DMA_AXI_CFG0(npu->reg_base));
    // npu_reg_dump(npu); // temp

	return 0;
}
int dx_l1_npu_prepare_inference(dxnpu_t *npu)
{    
    volatile void __iomem *reg = npu->reg_base;
    pr_debug("%s\n", __func__);
    WRITE_SYSTEM_MODE_IRQ_INPUT_DONE_EN(reg, 0);
    WRITE_SYSTEM_MODE_IRQ_STEP_EN(reg, 0);
    WRITE_SYSTEM_MODE_IRQ_INF_EN(reg, 1);
    WRITE_SYSTEM_MODE_LAST_CMD_NUM(reg, 0);
    WRITE_SYSTEM_MODE_AUTORUN_EN(reg, 1);
    WRITE_DMA_AXI_BASE_ADDR_LOW(reg, ((uint64_t)npu->dma_buf_addr)&0xFFFFFFFF);
    WRITE_DMA_AXI_BASE_ADDR_HIGH(reg, ((uint64_t)npu->dma_buf_addr)>>32);
    WRITE_DMA_SFR_BASE_ADDR_LOW(reg, ((uint64_t)npu->dma_buf_addr)&0xFFFFFFFF);
    WRITE_DMA_SFR_BASE_ADDR_HIGH(reg, ((uint64_t)npu->dma_buf_addr)>>32);
    WRITE_DMA_SFR_ADDR_OFFS(reg, npu->default_values[0]);
    WRITE_DMA_SFR_START_ADDR(reg, npu->default_values[1]);
    WRITE_DMA_SFR_MOVE_CNT(reg, npu->default_values[2]);
	pr_debug("    npu%d (DX-L1) @ %x: IRQ: %d, ID: %X, MODE: %X, AXI_CFG: %X\n",
        npu->id, npu->reg_base_addr, npu->irq_num, READ_SYSTEM_ID(npu->reg_base), READ_SYSTEM_MODE(npu->reg_base), READ_DMA_AXI_CFG0(npu->reg_base));

    return 0;
}
int dx_l1_npu_run(dxnpu_t *npu, void *data)
{
    dxrt_request_t *req = (dxrt_request_t*)data;    
    volatile void __iomem *reg = npu->reg_base;
    pr_debug("%s: %d, %d\n", __func__, req->req_id);
    // npu_reg_dump(npu);
    // while(READ_SYSTEM_STATUS_BUSY(reg));
    while(READ_SYSTEM_SWREG1(reg)==0xFFAA);
    WRITE_SYSTEM_MODE_LAST_CMD_NUM(reg, req->model_cmds);
    if( req->input.base != (uint64_t)npu->dma_buf_addr )
    {
        WRITE_DMA_AXI_BASE_ADDR_LOW(reg, 0);
        WRITE_DMA_AXI_BASE_ADDR_HIGH(reg, 0);
    }
    WRITE_DMA_AXI_BASE_ADDR_LOW(reg, ((uint64_t)npu->dma_buf_addr)&0xFFFFFFFF);
    WRITE_DMA_AXI_BASE_ADDR_HIGH(reg, ((uint64_t)npu->dma_buf_addr)>>32);
    WRITE_DMA_SFR_BASE_ADDR_LOW(reg, (((uint64_t)npu->dma_buf_addr) + req->cmd_offset)&0xFFFFFFFF );
    WRITE_DMA_SFR_BASE_ADDR_HIGH(reg, (((uint64_t)npu->dma_buf_addr) + req->cmd_offset)>>32 );
    WRITE_SYSTEM_WEIGHT_BASE(reg, (req->weight_offset) >> 6);
    WRITE_SYSTEM_FEATURE_BASE(reg, (req->input.offset) >> 6);
    WRITE_SYSTEM_SWREG0(reg, req->req_id);
    WRITE_SYSTEM_SWREG1(reg, 0xFFAA);
    // TODO: Timestamp
    WRITE_DEBUG_STAMP0_STAMP_EN(reg, 1);
    WRITE_SYSTEM_CMD_START(reg, 1);
    return 0;
}
int dx_l1_npu_reg_dump(dxnpu_t *npu)
{
    int i;
    pr_info("==== NPU SYSTEM REG ====\n");
    for(i=0;i<sizeof(npu_reg_sys_t)/4;i++)
    {
        pr_info("0x%08X: %X\n", i*4, npu_reg_read(npu->reg_base, i*4));
    }
    pr_info("==== NPU DMA REG ====\n");
    for(i=0;i<sizeof(npu_reg_dma_t)/4;i++)
    {
        pr_info("0x%08X: %X\n", REG_DMA_OFFSET + i*4, npu_reg_read(npu->reg_base, REG_DMA_OFFSET + i*4));
    }
    return 0;
}
int dx_l1_npu_deinit(dxnpu_t *npu)
{
    pr_debug("%s\n", __func__);
    dma_free_coherent(npu->dev, npu->dma_buf_size, npu->dma_buf, npu->dma_buf_addr);
    free_irq(npu->irq_num, (void*)npu);
    iounmap(npu->reg_base);
    enx_en677_cdma_free(dma_desc);
    return 0;
}

// EN677 CDMA
int en677_npu_dma_copy(uint32_t *data)
{
    // pr_info("%s: %x, %x, %x\n", __func__, data[0], data[1], data[2]);
	if (dma_desc) {
		if (!dma_desc->memcpy(dma_desc, data[0], data[1], data[2], 3)) {
            pr_err("%s: error -2\n", __func__);
			return -2;
		}
		if (!dma_desc->wait_for_request_complete(dma_desc, 1)) {
            pr_err("%s: error -3\n", __func__);
			return -3;
		}
	}
	return 0;
}
