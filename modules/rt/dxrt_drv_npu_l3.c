// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */

#include "dxrt_drv_npu.h"
#include "dxrt_drv.h"

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
static void get_ppu_data(dxnpu_t *npu)
{
    volatile void __iomem *reg = npu->reg_base;
    uint32_t filter_num;
    pr_debug("%s, %x\n", __func__, READ_SYSTEM_PPU_OUT_FILTER_INFO(reg));

    // check
    filter_num = READ_SYSTEM_PPU_OUT_FILTER_INFO(reg) & 0xffff;
    if(filter_num>0)
    {
        // flush dma
        pr_debug("%s: flush dma\n", __func__);
        WRITE_DMA_DMA_CTRL(reg, 1);
        while(READ_DMA_FLUSH_STATUS(reg)==0);

        // unflush dma
        WRITE_DMA_DMA_CTRL(reg, 0);
        pr_debug("%s: unflush dma\n", __func__);

        // dma transfer
        pr_debug("%s: dma req. %x\n", __func__, READ_SYSTEM_SWREG2(reg));
        WRITE_DMA_DMA_CTRL(reg, 2);
        WRITE_DMA_AXI4_WADDR(reg, READ_SYSTEM_SWREG2(reg));
        WRITE_DMA_SRAM_SRC_ADDR(reg, 0x04003800);
        WRITE_DMA_WRITE_SIZE(reg, 128*1024);
        WRITE_DMA_CMD(reg, 2);
        
        // poll dma irq
        pr_debug("%s: poll dma irq\n", __func__);
        // mdelay(500); // temp.
        while( (READ_DMA_STATUS(reg)&2) != 2);

        // clear dma irq
        pr_debug("%s: clear dma irq\n", __func__);
        WRITE_DMA_STATUS(reg, 2);

        // get data
        npu->response->ppu_filter_num = filter_num;
    }
}
static irqreturn_t npu_irq_handler(int irq, void *data)
{
    unsigned long flags;
    dxnpu_t *npu = (dxnpu_t*)data;    
    dxrt_response_t *response = npu->response;
    volatile void __iomem *reg = npu->reg_base;
    uint32_t filter_num;
    pr_debug("npu%d irq: %x\n", npu->id, READ_SYSTEM_IRQ_STATUS(reg)); // this log causes worse latency.

    // clear IRQ
    WRITE_SYSTEM_IRQ_STATUS(reg, 0xC0000000);

    // ppu processing
    get_ppu_data(npu);

    // argmax processing
    response->argmax = READ_SYSTEM_STATUS1_ARGMAX_ID(reg);

    // get inference time    
    READ_DEBUG_FSM_RD(reg);
    response->inf_time = (uint32_t)((uint64_t)READ_DEBUG_STAMP2(reg) * 1000 / npu->clock_khz);
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
int dx_l3_npu_init(dxnpu_t *npu)
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
    WRITE_SYSTEM_IRQ_STATUS(reg, 0xC0000000);
    ret = request_irq(npu->irq_num, npu_irq_handler, 0, "deepx-npu", (void*)npu);
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
    /* Init */    
	pr_info("    npu%d @ %x: CLOCK: %dKHz, IRQ: %d, ID: %X, MODE: %X, AXI_CFG: %X\n", 
        npu->id, npu->reg_base_addr, npu->clock_khz, npu->irq_num, READ_SYSTEM_ID(npu->reg_base),
        READ_SYSTEM_RUN_OPT(npu->reg_base), READ_DMA_AXI_CFG0(npu->reg_base));
    // npu_reg_dump(npu); // temp

	return 0;
}
int dx_l3_npu_prepare_inference(dxnpu_t *npu)
{    
    volatile void __iomem *reg = npu->reg_base;
    pr_debug("%s\n", __func__);
    WRITE_SYSTEM_IRQ_SET_IRQ_FRAME_SET(reg, 1);
    WRITE_SYSTEM_IRQ_SET_IRQ_TILE_SET(reg, 0);
    WRITE_SYSTEM_IRQ_SET_IRQ_INPUT_DONE_SET(reg, 0);
    WRITE_SYSTEM_NET_INFO_LAST_CMD_NUM(reg, 0);
    WRITE_SYSTEM_RUN_OPT_AUTORUN_EN(reg, 1);
    WRITE_DMA_AXI_BASE_ADDR_LOW(reg, ((uint64_t)npu->dma_buf_addr)&0xFFFFFFFF);
    WRITE_DMA_AXI_BASE_ADDR_HIGH(reg, ((uint64_t)npu->dma_buf_addr)>>32);
    WRITE_DMA_SFR_BASE_ADDR_LOW(reg, ((uint64_t)npu->dma_buf_addr)&0xFFFFFFFF);
    WRITE_DMA_SFR_BASE_ADDR_HIGH(reg, ((uint64_t)npu->dma_buf_addr)>>32);
    WRITE_DMA_SFR_ADDR_OFFS(reg, npu->default_values[0]);
    WRITE_DMA_SFR_START_ADDR(reg, npu->default_values[1]);
    WRITE_DMA_SFR_MOVE_CNT(reg, npu->default_values[2]);
	pr_debug("    npu%d (DX-L3) @ %x: IRQ: %d, ID: %X, MODE: %X, AXI_CFG: %X\n",
        npu->id, npu->reg_base_addr, npu->irq_num, READ_SYSTEM_ID(npu->reg_base), READ_SYSTEM_RUN_OPT(npu->reg_base), READ_DMA_AXI_CFG0(npu->reg_base));
    return 0;
}
int dx_l3_npu_run(dxnpu_t *npu, void *data)
{
    unsigned long flags;
    int ready = 0;
    dxrt_request_t *req = (dxrt_request_t*)data;    
    volatile void __iomem *reg = npu->reg_base;
    pr_debug("%s: %d\n", __func__, req->req_id);
    // npu_reg_dump(npu);
    while(READ_SYSTEM_SWREG1(reg)==0xFFAA);
    WRITE_SYSTEM_NET_INFO_LAST_CMD_NUM(reg, req->model_cmds);
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
    WRITE_SYSTEM_IN_FEATURE_BASE(reg, (req->input.offset) >> 6);
    WRITE_SYSTEM_OUT_FEATURE_BASE(reg, (req->output.offset) >> 6);
    WRITE_SYSTEM_SWREG0(reg, req->req_id);
    WRITE_SYSTEM_SWREG2(reg, req->output.offset + req->last_output_offset);
    WRITE_SYSTEM_SWREG1(reg, 0xFFAA);
    WRITE_DEBUG_STAMP0_STAMP_EN(reg, 1);
    WRITE_SYSTEM_CMD_START(reg, 1);
    return 0;
}
int dx_l3_npu_reg_dump(dxnpu_t *npu)
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
int dx_l3_npu_deinit(dxnpu_t *npu)
{
    pr_debug("%s\n", __func__);
    dma_free_coherent(npu->dev, npu->dma_buf_size, npu->dma_buf, npu->dma_buf_addr);
    free_irq(npu->irq_num, (void*)npu);
    iounmap(npu->reg_base);
    return 0;
}
