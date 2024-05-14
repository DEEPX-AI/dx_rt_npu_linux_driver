// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#include "dxrt_drv.h"
#include "dxrt_version.h"

#define INTERNAL_BUFF_MAX   (1024)
/* size : The number of bytes */
static int dxrt_copy_to_user_io(int num, void __user *dest, void __iomem *src, size_t size)
{
    size_t remaining_size = size;
    int ret = 0;
    char *buffer = kmalloc(INTERNAL_BUFF_MAX, GFP_KERNEL);

    if (!buffer) {
        pr_err("%d: %s failed to allocate memory.\n", num, __func__);
        return -ENOMEM;
    }

    while (remaining_size > 0) {
        size_t copy_size = (remaining_size < INTERNAL_BUFF_MAX) ? remaining_size : INTERNAL_BUFF_MAX;
        // pr_info("%d: %s, cp_size:%ld, dest:%p, src:%p\n", num, __func__, copy_size, dest, src);
        memcpy_fromio(buffer, src, copy_size);
        if (copy_to_user(dest, buffer, copy_size)) {
            pr_err("%d: %s failed to copy data to user space.\n", num, __func__);
            ret = -EFAULT;
            break;
        }
        dest += copy_size;
        src += copy_size;
        remaining_size -= copy_size;
    }
    kfree(buffer);

    return ret;
}

static int dxrt_copy_from_user_io(int num, void __iomem *dest, const void __user *src, size_t size)
{
    size_t remaining_size = size;
    int ret = 0;
    char *buffer = kmalloc(INTERNAL_BUFF_MAX, GFP_KERNEL);

    if (!buffer) {
        pr_err("%d: %s failed to allocate memory.\n", num, __func__);
        return -ENOMEM;
    }

    while (remaining_size > 0) {
        size_t copy_size = (remaining_size < INTERNAL_BUFF_MAX) ? remaining_size : INTERNAL_BUFF_MAX;
        // pr_info("%d: %s, cp_size:%ld, dest:%p, src:%p\n", num, __func__, copy_size, dest, src);
        if (copy_from_user(buffer, src, copy_size)) {
            pr_err("%d: %s failed to copy data from user space.\n", num, __func__);
            ret = -EFAULT;
            break;
        }
        memcpy_toio(dest, buffer, copy_size);
        src += copy_size;
        dest += copy_size;
        remaining_size -= copy_size;
    }
    kfree(buffer);

    return ret;
}

static int dxrt_msg_general(struct dxdev *dev, dxrt_message_t *msg)
{
    int ret, num = dev->id;
    pr_debug("%s: %d, %d: %llx %d\n", __func__, dev->id, dev->type, (uint64_t)msg->data, msg->size);
    if(dev->type==1)
    {
        ret = 0;
    }
    else
    {
        // dxrt_device_message_t *dev_msg = dev->msg;
        if(dev->msg)
        {
            int fail_cnt = 0;
            mutex_lock(&dev->msg_lock);
            dev->msg->cmd = msg->cmd;
            if(msg->size>0 && msg->size<sizeof(dxrt_device_message_t))
            {
                if (dxrt_copy_from_user_io(num, &dev->msg->data, (void __user*)msg->data, msg->size)) {
                    pr_debug("%d: %s: failed.\n", num, __func__);
                    return -EFAULT;
                }
            }
            dev->msg->size = 0;
            dev->msg->ack = 0;
            while(true)
            {
                mdelay(10);
                if(fail_cnt>1000)
                {
                    ret = -1;
                    break;
                }
                if(dev->msg->ack == 1)
                {
                    ret = 0;
                    break;
                }
                fail_cnt++;
            }
            if(dev->msg->size>0 && dev->msg->size<sizeof(dxrt_device_message_t) && ret==0)
            {
                ret = dxrt_copy_to_user_io(num, (void __user*)(msg->data), &dev->msg->data, dev->msg->size);
            }
            mutex_unlock(&dev->msg_lock);
        }
        else
        {
            ret = 0;
        }
    }
    return ret;
}

static int dxrt_identify_device(struct dxdev* dev, dxrt_message_t *msg)
{
    int ret, num = dev->id;
    dxrt_device_info_t info;
    pr_debug("%s: %d, %d: %llx, %d\n", __func__, dev->id, dev->type, (uint64_t)msg->data, msg->size);
    info.type = dev->type;
    info.variant = dev->variant;
    memset(&dev->response, 0, sizeof(dxrt_response_t));

    {
        dxrt_response_list_t *entry, *tmp;
        spin_lock(&dev->responses_lock);
        list_for_each_entry_safe(entry, tmp, &dev->responses.list, list) {
            list_del(&entry->list);
            kfree(entry);
        }
        spin_unlock(&dev->responses_lock);
    }
    if(dev->type==1)
    {
        // dxnpu_t *npu = dev->npu;        
        dev->mem_addr = dev->npu->dma_buf_addr;
        dev->mem_size = dev->npu->dma_buf_size;
        dev->num_dma_ch = 1;
        info.mem_addr = dev->npu->dma_buf_addr;
        info.mem_size = dev->npu->dma_buf_size;
        info.num_dma_ch = 1;
        pr_debug("%d: %s: [%llx, %llx], %d\n", num, __func__,
            info.mem_addr, info.mem_size,
            info.num_dma_ch);
        ret = dev->npu->prefare_inference(dev->npu);
        if(msg->data!=NULL)
        {
            if (copy_to_user((void __user*)msg->data, &info, sizeof(info))) {
                pr_debug("%d: %s failed.\n", num, __func__);
                return -EFAULT;
            }
        }
        return ret;
    }
    else
    {
        dev->msg = (dxrt_device_message_t*)dx_pcie_get_message_area(num);
        dev->log = (uint32_t*)dx_pcie_get_log_area(num);
        dev->request_queue = (dxrt_queue_t*)dx_pcie_get_request_queue(num);
        if(dev->msg)
        {
            ret = dxrt_msg_general(dev, msg);
            if(ret<0) return -1;
            memcpy_fromio(&info, dev->msg->data, sizeof(info));
            pr_debug("%d: %s: [%llx, %llx], %d\n", num, __func__,
                info.mem_addr, info.mem_size,
                info.num_dma_ch);
            dev->mem_addr = info.mem_addr;
            dev->mem_size = info.mem_size;
            dev->num_dma_ch = info.num_dma_ch;
            
            dx_pcie_clear_response_queue(num);
        }
        else
        {
            ret = 0;
            info.mem_addr = 0x01000000;
            info.mem_size = 0xFF000000;
            info.num_dma_ch = 1;
            info.interface = 1; // for fpga
            info.fw_ver = 202; // for fpga
            dev->mem_addr = info.mem_addr;
            dev->mem_size = info.mem_size;
            dev->num_dma_ch = info.num_dma_ch;
            if (copy_to_user((void __user*)msg->data, &info, sizeof(info))) {
                pr_debug("%d: %s failed.\n", num, __func__);
                return -EFAULT;
            }
        }

        return ret;
    }
    // return ret;
}
static int dxrt_write_mem(struct dxdev* dev, dxrt_message_t *msg)
{
    int ret = 0, num = dev->id;
    dxrt_meminfo_t meminfo;
    pr_debug("%d: %s\n", num, __func__);
    if(msg->data!=NULL)
    {
        if (copy_from_user(&meminfo, (void __user*)msg->data, sizeof(meminfo))) {
            pr_debug("%d: %s: failed.\n", num, __func__);
            return -EFAULT;
        }
        pr_debug( MODULE_NAME "%d: %s: [%llx, %llx + %x(%x)]\n",
            num,
            __func__,
            meminfo.data,
            meminfo.base,
            meminfo.offset,
            meminfo.size
        );
        if( meminfo.base + meminfo.offset < dev->mem_addr ||            
            meminfo.base + meminfo.offset + meminfo.size > dev->mem_addr + dev->mem_size )
        {
            pr_debug("%d: %s: invalid address: %llx + %x (%llx + %llx)\n", num, __func__, 
                meminfo.base, meminfo.offset, dev->mem_addr, dev->mem_size);
            return -EINVAL;
        }
        if(dev->type==1)
        {
            if (copy_from_user(dev->npu->dma_buf + meminfo.offset, (void __user*)meminfo.data, meminfo.size)) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            sbi_l2cache_flush(meminfo.base + meminfo.offset, meminfo.size);
            ret = 0;
        }
        else
        {
            ssize_t size = dx_sgdma_write((char *)meminfo.data,
                meminfo.base + meminfo.offset,
                meminfo.size,
                num,
                0, /*TODO*/
                false);
            if (size != meminfo.size)
            {
                pr_err("Pcie write error!(%ld)\n", size);
                ret = -1;
            }
        }
    }
    return ret;
}
static int dxrt_write_input(struct dxdev* dev, dxrt_message_t *msg)
{
    int ret = -1, num = dev->id;
    pr_debug("%d: %s\n", num, __func__);
    if(dev->type==0)
    {
        dxrt_request_acc_t req;
        unsigned long flags;        
        if(msg->data!=NULL)
        {
            if (copy_from_user(&req, (void __user*)msg->data, sizeof(req))) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            pr_debug("%d: %s: ch %d, req %d, type %d, [%d cmds @ %x, weight @ %x], [%d cmds @ %x, weight @ %x], input @ %llx+%x(%x), output @ %llx+%x(%x), %x\n", 
                num, __func__, req.dma_ch,
                req.req_id, req.model_type, 
                req.model_cmds, req.cmd_offset, req.weight_offset,
                req.model_cmds2, req.cmd_offset2, req.weight_offset2,
                req.input.base, req.input.offset, req.input.size,
                req.output.base, req.output.offset, req.output.size,
                req.arg0
            );
            if (req.input.data)
            {
                if(dxrt_is_queue_full(dev->request_queue))
                {
                    // pr_info("queue full\n");
                    return -EBUSY;
                }
                else
                {
                    // pr_info( MODULE_NAME "%d: %s: ch %d, req %d, type %d, [%d cmds @ %x, weight @ %x], [%d cmds @ %x, weight @ %x], input %llx @ %llx+%x(%x), output %llx @ %llx+%x(%x), %x\n", 
                    //     num, __func__, req.dma_ch,
                    //     req.req_id, req.model_type, 
                    //     req.model_cmds, req.cmd_offset, req.weight_offset,
                    //     req.model_cmds2, req.cmd_offset2, req.weight_offset2,
                    //     req.input.data, req.input.base, req.input.offset, req.input.size,
                    //     req.output.data, req.output.base, req.output.offset, req.output.size,
                    //     req.arg0
                    // );
                    ssize_t size = dx_sgdma_write((char *)req.input.data,
                        req.input.base + req.input.offset,
                        req.input.size,
                        num,
                        req.dma_ch, // TODO : fixme
                        // 0, // temp.
                        false);
                    if (size != req.input.size) {
                        pr_err("Pcie input write error!(%ld)\n", size);
                        dx_pcie_enqueue_error_response(num, ERR_PCIE_DMA_CH0_FAIL + req.dma_ch);
                    }
                }
            }
            else
            {
                pr_debug("Input Data is null.\n");
            }
            spin_lock_irqsave(&dev->request_queue_lock, flags);
            ret = dxrt_enqueue(dev->request_queue, &req);
            spin_unlock_irqrestore(&dev->request_queue_lock, flags);
        }
        return ret;
    }
    else
    {
        dxrt_request_t req;        
        if(msg->data!=NULL)
        {
            if (copy_from_user(&req, (void __user*)msg->data, sizeof(req))) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            pr_debug( MODULE_NAME "%d: %s: req %d, type %d, %d cmds @ %x, weight @ %x, input @ %llx+%x(%x), output @ %llx+%x(%x)\n", 
                num, __func__, req.req_id, req.model_type, req.model_cmds,
                req.cmd_offset, req.weight_offset,
                req.input.base, req.input.offset, req.input.size,
                req.output.base, req.output.offset, req.output.size
            );
            dev->npu->run(dev->npu, &req);
        }        
        return 0;
    }
}
static int dxrt_read_output(struct dxdev* dev, dxrt_message_t* msg)
{
    int num = dev->id;
    pr_debug("%d: %s\n", num, __func__);
    if(dev->type==0)
    {
        dx_pcie_response_t response = {0};
        int ret = -1;
        unsigned int mask = 0;
        // unsigned long flags;
        uint32_t ch;
        if(msg->data!=NULL)
        {
            if (copy_from_user(&ch, (void __user*)msg->data, sizeof(ch))) {
                pr_err( MODULE_NAME "%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            if (ch>2) {
                pr_err( MODULE_NAME "%d: %s: invalid channel.\n", num, __func__);
                return -EINVAL;
            }        
            if(dx_pcie_is_response_queue_empty(num, ch))
            {
                pr_debug(MODULE_NAME "%d: %s, ch%d: start to wait.\n", num, __func__, ch);
                mask = dx_pcie_interrupt(num, ch);
                pr_debug(MODULE_NAME "%d: %s, ch%d: wake up.\n", num, __func__, ch);
            }
            else
            {
                mask = 1;
            }
            if(mask==1)
            {
                ret = dx_pcie_dequeue_response(num, ch, &response);
                // pr_info( MODULE_NAME "%d: %s: ch%d, id %d, type %d, output @ %llx, %llx+%x(%x), ret %d\n", 
                //     num, __func__, ch, response.req_id, response.model_type,
                //     response.data, response.base, response.offset, response.size,
                //     ret
                // );
                if (ret == 0 && response.req_id>0)
                {
                    // pr_info( MODULE_NAME "%d: %d\n", num, response.req_id);
                    if((response.model_type==0) || (response.model_type==2))
                    {
                        ssize_t size;
                        size = dx_sgdma_read((char *)response.data,
                            response.base + response.offset,
                            response.size,
                            num,
                            ch);
                        if (size != response.size)
                        {
                            pr_err("Pcie output read error!(%ld)\n", size);
                            response.status = ERR_PCIE_DMA_CH0_FAIL + ch;
                            dx_pcie_enqueue_error_response(num, ERR_PCIE_DMA_CH0_FAIL + ch);
                        }
                    }
                    if (copy_to_user((void __user*)msg->data, &response, sizeof(dxrt_response_t))) {
                        pr_err( MODULE_NAME "%d: %s: memcpy failed.\n", num, __func__);
                        return -EFAULT;
                    }
                }
                else
                {
                    // pr_info( MODULE_NAME "%d: %s, skip. %d, %d, %d\n", num, __func__, ch, ret, response.req_id);
                }
                // pr_debug("%d: %s, %d: response %d\n", num, __func__, ch, response.req_id);
            }
        }
        return ret;
    }
    else
    {
        dxnpu_t *npu = dev->npu;
        // dxrt_response_t response;
        int ret = -1;
        unsigned long flags;
        if(msg->data!=NULL)
        {
            if(list_empty(&dev->responses.list))
            {                
                pr_debug(MODULE_NAME "%d: %s: start to wait.\n", num, __func__);
                ret = wait_event_interruptible(npu->irq_wq, npu->irq_event==1);
                pr_debug(MODULE_NAME "%d: %s: wake up.\n", num, __func__);
                spin_lock_irqsave(&npu->irq_event_lock, flags);
                npu->irq_event = 0;
                spin_unlock_irqrestore(&npu->irq_event_lock, flags);
            }
            {
                spin_lock_irqsave(&dev->responses_lock, flags);
                if(!list_empty(&dev->responses.list))
                {
                    dxrt_response_list_t *entry = list_first_entry(&dev->responses.list, dxrt_response_list_t, list);
                    pr_debug(MODULE_NAME "%d: %s: %d\n", num, __func__, entry->response.req_id);
                    if (copy_to_user((void __user*)msg->data, &entry->response, sizeof(dxrt_response_t))) {
                        pr_err( MODULE_NAME "%d: %s: memcpy failed.\n", num, __func__);
                        return -EFAULT;
                    }
                    list_del(&entry->list);
                    kfree(entry);
                    ret = 0;
                }
                else
                {
                    pr_debug(MODULE_NAME "%d: %s: empty\n", num, __func__);
                    ret = -1;
                }
                spin_unlock_irqrestore(&dev->responses_lock, flags);
            }
        }
        return ret;
    }
}

static int dxrt_terminate(struct dxdev* dev, dxrt_message_t* msg)
{
    int num = dev->id;
    pr_debug(MODULE_NAME "%d: %s\n", num, __func__);
    if(dev->type==0)
    {        
        unsigned int mask = 0;
        // unsigned long flags;
        uint32_t ch;
        if(msg->data!=NULL)
        {
            if (copy_from_user(&ch, (void __user*)msg->data, sizeof(ch))) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            if (ch<0 || ch>2) {
                pr_debug("%d: %s: invalid channel.\n", num, __func__);
                return -EINVAL;
            }  
            pr_debug("%d: %s, %d\n", num, __func__, ch);
            if(dev->request_queue)
            {
                dxrt_disable_queue(dev->request_queue);
            }
            mask = dx_pcie_interrupt_wakeup(num, ch);
        }
        // spin_lock_irqsave(&dev->error_lock, flags);
        // dev->error = 99;
        // wake_up_interruptible(&dev->error_wq);
        // spin_unlock_irqrestore(&dev->error_lock, flags);
        return mask;
    }
    else
    {
        dxnpu_t *npu = dev->npu;
        unsigned long flags;
        pr_debug(MODULE_NAME "%d: %s start \n", num, __func__);
        spin_lock_irqsave(&npu->irq_event_lock, flags);
        npu->irq_event = 1;
        wake_up_interruptible(&npu->irq_wq);
        spin_unlock_irqrestore(&npu->irq_event_lock, flags);
        {
            spin_lock_irqsave(&dev->error_lock, flags);
            dev->error = 99;
            wake_up_interruptible(&dev->error_wq);
            spin_unlock_irqrestore(&dev->error_lock, flags);
        }
        pr_debug(MODULE_NAME "%d: %s done.\n", num, __func__);
        return 0;
    }
}
static int dxrt_read_mem(struct dxdev* dev, dxrt_message_t* msg)
{
    int num = dev->id;
    dxrt_meminfo_t meminfo;
    pr_debug("%d: %s\n", num, __func__);
    if(msg->data!=NULL)
    {
        if (copy_from_user(&meminfo, (void __user*)msg->data, sizeof(meminfo))) {
            pr_debug("%d: %s: failed.\n", num, __func__);
            return -EFAULT;
        }        
        pr_debug( MODULE_NAME "%d: %s: [%llx, %llx + %x(%x)]\n",
            num,
            __func__,
            meminfo.data,
            meminfo.base,
            meminfo.offset,
            meminfo.size
        );
        if( meminfo.base + meminfo.offset < dev->mem_addr ||            
            meminfo.base + meminfo.offset + meminfo.size > dev->mem_addr + dev->mem_size )
        {
            pr_debug("%d: %s: invalid address: %llx + %x\n", num, __func__, meminfo.base, meminfo.offset);
            return -EINVAL;
        }
        if(dev->type==1)
        {
            if (copy_to_user((void __user*)meminfo.data, dev->npu->dma_buf + meminfo.offset,  meminfo.size)) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
        }
        else
        {
            ssize_t size = dx_sgdma_read((char *)meminfo.data,
                meminfo.base + meminfo.offset,
                meminfo.size,
                num,
                0); /*TODO*/
            if (size != meminfo.size)
                pr_err("PCIe read error!(%ld)\n", size);
        }
    }
    return 0;
}
static int dxrt_cpu_cache_flush(struct dxdev* dev, dxrt_message_t* msg)
{
    int num = dev->id;
    dxrt_meminfo_t meminfo;
    pr_debug("%d: %s: %llx\n", num, __func__, (uint64_t)msg->data);
    if(msg->data!=NULL)
    {
        if (copy_from_user(&meminfo, (void __user*)msg->data, sizeof(meminfo))) {
            pr_debug("%d: %s: failed.\n", num, __func__);
            return -EFAULT;
        }
        pr_debug(MODULE_NAME "%d: %s: [%llx, %llx + %x(%x)]\n",
            num,
            __func__,
            meminfo.data,
            meminfo.base,
            meminfo.offset,
            meminfo.size
        );
        if( meminfo.base + meminfo.offset < dev->mem_addr ||            
            meminfo.base + meminfo.offset + meminfo.size > dev->mem_addr + dev->mem_size )
        {
            pr_debug("%d: %s: invalid address: %llx + %x\n", num, __func__, meminfo.base, meminfo.offset);
            return -EINVAL;
        }
        if(dev->type==1)
        {
            sbi_l2cache_flush(meminfo.base + meminfo.offset, meminfo.size);
        }
    }
    return 0;
}
static int dxrt_soc_custom(struct dxdev* dev, dxrt_message_t* msg)
{
    int ret = -1, num = dev->id;
    pr_debug("%d: %s: %llx\n", num, __func__, (uint64_t)msg->data);
    if(dev->type==1)
    {
#if DEVICE_VARIANT==DX_L1
        if(msg->data!=NULL)
        {
            uint32_t data[3];
            if (copy_from_user(data, (void __user*)msg->data, sizeof(uint32_t)*3)) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            en677_npu_dma_copy(data);
        }
#endif
    }
    else
    {
        dxrt_device_message_t *dev_msg = dev->msg;
        // pr_info("--> %p\n", dev->msg);
        int fail_cnt = 0;
        
        mutex_lock(&dev->msg_lock);
        dev_msg->cmd = msg->cmd;
        if(msg->size>0 && msg->size<sizeof(dxrt_device_message_t))
        {
            if (copy_from_user(&dev_msg->data, (void __user*)msg->data, msg->size)) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
        }
        dev_msg->size = 0;
        dev_msg->ack = 0;
        while(true)
        {
            mdelay(1);
            if(fail_cnt>1000)
            {
                ret = -1;
                break;
            }
            if(dev_msg->ack == 1)
            {
                ret = 0;
                break;
            }
            fail_cnt++;
        }
        if(dev_msg->size>0 && dev_msg->size<sizeof(dxrt_device_message_t))
        {
            ret = dxrt_copy_to_user_io(num, (void __user*)msg->data, &dev_msg->data, dev_msg->size);
        }
        mutex_unlock(&dev->msg_lock);
    }
    return 0;
}
static int dxrt_get_log(struct dxdev* dev, dxrt_message_t* msg)
{
    int ret = 0, num = dev->id;
    pr_debug("%s: %d, %d: %llx\n", __func__, dev->id, dev->type, (uint64_t)msg->data);
    if(dev->type==1)
    {
        ret = 0;
    }
    else
    {
        if (dxrt_copy_to_user_io(num, (void __user*)msg->data, dev->log, 16*1024)) {
            pr_debug("%d: %s failed.\n", num, __func__);
            ret = -EFAULT;
        }
    }
    return ret;
}
static int dxrt_update_firmware(struct dxdev* dev, dxrt_message_t* msg)
{
    int ret, num = dev->id;
    if(dev->type==1)
    {
        ret = 0;
    }
    else
    {
        dxrt_device_message_t *dev_msg = dev->msg;
        dxrt_meminfo_t meminfo = {(uint64_t)(msg->data), 0xd8110000, 0, (msg->size>0xef000) ? 0xef000 : msg->size };
        pr_info(MODULE_NAME "%d: %s: %llx, %llx, %x, %x\n", 
            num, __func__, meminfo.data, meminfo.base, meminfo.offset, meminfo.size
        );

        if(dev_msg)
        {
            int fail_cnt = 0;
            if(msg->data!=NULL)
            {
                ssize_t size = dx_sgdma_write((char *)meminfo.data,
                    meminfo.base + meminfo.offset,
                    meminfo.size,
                    num,
                    0, /*TODO*/
                    false);
                if (size != meminfo.size)
                {
                    pr_err("Pcie write error!(%ld)\n", size);
                    ret = -1;
                }
            }
            mutex_lock(&dev->msg_lock);
            dev_msg->cmd    = msg->cmd;
            dev_msg->sub_cmd= msg->sub_cmd;
            dev_msg->size   = 0;
            dev_msg->ack    = 0;
            while(true)
            {
                mdelay(1000);
                if(fail_cnt>20)
                {
                    ret = -1;
                    break;
                }
                if(dev_msg->ack == 1)
                {
                    ret = 0;
                    break;
                }
                fail_cnt++;
            }
            if(ret==0)
            {
                ret = *((int*)dev_msg->data);
            }
            mutex_unlock(&dev->msg_lock);
            pr_info(MODULE_NAME "%d: %s: done.\n", num, __func__);
        }
        else
        {
            ret = 0;
        }
    }
    return ret;
}
static int dxrt_reset_device(struct dxdev* dev, dxrt_message_t* msg)
{
    int ret, num = dev->id;
    if(dev->type==1)
    {
        ret = 0;
    }
    else
    {
        // dxrt_device_message_t * dev_msg = dev->msg;
        if(dev->msg)
        {
            // int fail_cnt = 0;
            mutex_lock(&dev->msg_lock);
            dev->msg->cmd = msg->cmd;
            if(msg->size>0 && msg->size<sizeof(dxrt_device_message_t))
            {
                ret = dxrt_copy_from_user_io(num, &dev->msg->data, msg->data, msg->size);
            }
            dev->msg->size = 0;
            dev->msg->ack = 0;
            mdelay(100);
            mutex_unlock(&dev->msg_lock);
        }
        else
        {
            ret = 0;
        }
    }
    return ret;
}

static int dxrt_handle_error(struct dxdev* dev, dxrt_message_t* msg)
{
    int num = dev->id;
    unsigned long flags;
    dxrt_error_t error;
    
    pr_debug(MODULE_NAME "%d: %s: start to wait.\n", num, __func__);
    {
        dx_pcie_dev_err_t dev_err = {0,};
        dx_pcie_dequeue_error_response(num, &dev_err);
        dev->error = dev_err.err_code;
    }
    // ret = wait_event_interruptible( dev->error_wq, dev->error!=0 );
    pr_debug(MODULE_NAME "%d: %s: wake up. error %d\n", num, __func__, dev->error);
    spin_lock_irqsave(&dev->error_lock, flags);
    error = dev->error;
    dev->error = 0;
    spin_unlock_irqrestore(&dev->error_lock, flags);
    if(msg->data!=NULL)
    {
        if (copy_to_user((void __user*)msg->data, &error, sizeof(error))) {
            pr_debug("%d: %s failed.\n", num, __func__);
            return -EFAULT;
        }
    }
    return 0;
}

static int dxrt_handle_rt_drv_info_sub(struct dxdev* dev, dxrt_message_t* msg)
{
    int num = dev->id;
    int ret = 0;
    pr_debug(MODULE_NAME "%d: %s, [%d]\n", num, __func__, msg->sub_cmd);
    switch (msg->sub_cmd) {
        case DRVINFO_CMD_GET_RT_INFO:
            if(msg->data!=NULL) {
                struct dxrt_drv_info info;
                info.driver_version = DXRT_MOD_VERSION_NUMBER;
                if (copy_to_user((void __user*)msg->data, &info, sizeof(info))) {
                    pr_err("%d: %s cmd:%d failed.\n", num, __func__, msg->sub_cmd);
                    ret = -EFAULT;
                }
            }
            break;
        case DRVINFO_CMD_GET_PCIE_INFO:
            if(dev->type == 0) {
                struct deepx_pcie_info info;
                dx_pcie_get_driver_info(&info, num);
                if(msg->data!=NULL) {
                    if (copy_to_user((void __user*)msg->data, &info, sizeof(info))) {
                        pr_err("%d: %s cmd:%d failed.\n", num, __func__, msg->sub_cmd);
                        ret = -EFAULT;
                    }
                }
            } else {
                pr_err("CMD(%d) is not supported for device type(%d)\n", msg->cmd, dev->type);
            }
            break;
    default:
        pr_err("Unsupported sub command(%d/%d)\n", msg->cmd, msg->sub_cmd);
        ret = -EFAULT;
        break;
    }
    return ret;
}

static int dxrt_handle_drv_info(struct dxdev* dev, dxrt_message_t* msg)
{
    return dxrt_handle_rt_drv_info_sub(dev, msg);
}

int message_handler_general(struct dxdev *dx, dxrt_message_t *msg)
{
    return message_handler[msg->cmd](dx, msg);
}

dxrt_message_handler message_handler[] = {
    [DXRT_CMD_IDENTIFY_DEVICE]      = dxrt_identify_device,
    [DXRT_CMD_WRITE_MEM]            = dxrt_write_mem,
    [DXRT_CMD_READ_MEM]             = dxrt_read_mem,
    [DXRT_CMD_CPU_CACHE_FLUSH]      = dxrt_cpu_cache_flush,
    [DXRT_CMD_WRITE_INPUT_DMA_CH0]  = dxrt_write_input,
    [DXRT_CMD_WRITE_INPUT_DMA_CH1]  = dxrt_write_input,
    [DXRT_CMD_WRITE_INPUT_DMA_CH2]  = dxrt_write_input,
    [DXRT_CMD_READ_OUTPUT_DMA_CH0]  = dxrt_read_output,
    [DXRT_CMD_READ_OUTPUT_DMA_CH1]  = dxrt_read_output,
    [DXRT_CMD_READ_OUTPUT_DMA_CH2]  = dxrt_read_output,
    [DXRT_CMD_TERMINATE]            = dxrt_terminate,
    [DXRT_CMD_SOC_CUSTOM]           = dxrt_soc_custom,
    [DXRT_CMD_GET_STATUS]           = dxrt_msg_general,
    [DXRT_CMD_RESET]                = dxrt_reset_device,
    [DXRT_CMD_UPDATE_CONFIG]        = dxrt_msg_general,
    [DXRT_CMD_UPDATE_FIRMWARE]      = dxrt_update_firmware,
    [DXRT_CMD_GET_LOG]              = dxrt_get_log,
    [DXRT_CMD_DUMP]                 = dxrt_msg_general,
    [DXRT_CMD_ERROR]                = dxrt_handle_error,
    [DXRT_CMD_DRV_INFO]             = dxrt_handle_drv_info,
};