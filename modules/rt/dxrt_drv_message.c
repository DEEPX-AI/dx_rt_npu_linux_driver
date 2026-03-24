// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#include "dxrt_drv.h"
#include "dxrt_version.h"

extern bool dxrt_fault_inject_skip_addr_check;

/* Shorthand macros for dxrt_device_message_t (dev->msg) access */
#define dx_msg_read32(dev, field) \
    dx_read32((dev)->msg, dxrt_device_message_t, field)

#define dx_msg_write32(dev, field, val) \
    dx_write32((dev)->msg, dxrt_device_message_t, field, val)

#define DX_MSG_DATA_ADDR(dev) \
    DX_FIELD_ADDR((dev)->msg, dxrt_device_message_t, data)

/* Callback invoked by PCIe module when a response is enqueued */
static void dxrt_response_notify(u32 dev_id, int dma_ch, void *data)
{
    struct dxdev *dev = (struct dxdev *)data;
    atomic_inc(&dev->response_pending[dma_ch]);
    wake_up_interruptible(&dev->response_wq[dma_ch]);
}

/* Callback invoked by PCIe module when an event is enqueued */
static void dxrt_event_notify(u32 dev_id, void *data)
{
    struct dxdev *dev = (struct dxdev *)data;
    wake_up_interruptible(&dev->event_wq);
}

/*
* Initialization function to drive the device
*/
void dxrt_device_init(struct dxdev* dev)
{
    int num = dev->id;
    int i;
    pr_debug("%s:%d type:%d\n", __func__, num, dev->type);

    for (i = 0; i <= MAX_PCIE_CH_NUM; i++) {
        init_waitqueue_head(&dev->response_wq[i]);
        atomic_set(&dev->response_pending[i], 0);
    }
    init_waitqueue_head(&dev->event_wq);

    if (dev->type == DX_STD) {
        /* do nothing */
    } else {
        dev->msg                = (dxrt_device_message_t*)dx_pcie_get_message_area(num);
        dev->log                = (uint32_t*)dx_pcie_get_log_area(num);
        dev->dl                 = (dx_download_msg*)dx_pcie_get_dl_area(num);
        dev->request_queue      = (dxrt_queue_t*)dx_pcie_get_request_queue(num, DX_NORMAL_QUEUE0);
        dev->request_queue1     = (dxrt_queue_t*)dx_pcie_get_request_queue(num, DX_NORMAL_QUEUE1);
        dev->request_queue2     = (dxrt_queue_t*)dx_pcie_get_request_queue(num, DX_NORMAL_QUEUE2);
        dev->request_high_queue = (dxrt_queue_t*)dx_pcie_get_request_queue(num, DX_HIGH_QUEUE);
        dx_pcie_clear_response_queue(num);

        /* Register ISR→RT wake-up callbacks */
        dx_pcie_register_response_callback(num, dxrt_response_notify, dev);
        dx_pcie_register_event_callback(num, dxrt_event_notify, dev);
    }
}

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
        dx_memcpy_fromio32(buffer, src, copy_size);
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
        dx_memcpy_toio32(dest, buffer, copy_size);
        src += copy_size;
        dest += copy_size;
        remaining_size -= copy_size;
    }
    kfree(buffer);

    return ret;
}

static int dxrt_copy_resp_to_user_acc(struct dxdev* dev, dxrt_message_t *msg, dx_pcie_response_t *response)
{
    int num = dev->id;
    dxrt_response_t user_response;

    user_response.req_id = response->req_id;
    user_response.inf_time = response->inf_time;
    user_response.argmax = response->argmax;
    user_response.model_type = response->model_type;
    user_response.status = response->status;
    user_response.ppu_filter_num = response->ppu_filter_num;
    user_response.proc_id = response->proc_id;
    user_response.queue = response->queue;
    user_response.dma_ch = response->dma_ch;
    user_response.ddr_rd_bw = response->ddr_rd_bw;
    user_response.ddr_wr_bw = response->ddr_wr_bw;

    if (copy_to_user((void __user*)msg->data, &user_response, sizeof(dxrt_response_t))) {
        pr_err( MODULE_NAME "%d: %s: memcpy failed.\n", num, __func__);
        return -EFAULT;
    }
    return 0;
}

#define ACK_POLLING_THRESHOLD  (1000)
static int dxrt_polling_ack(struct dxdev *dev, int ms)
{
    int fail_cnt = 0, ret = 0;
    int sleep_ms = max(ms, 1);

    while (true) {
        if (dx_msg_read32(dev, ack) == 1)
            break;

        if (++fail_cnt > ACK_POLLING_THRESHOLD) {
            pr_err(MODULE_NAME "%s: timeout (dev %d, last cmd %d).\n",
                __func__, dev->id, dx_msg_read32(dev, cmd));
            /* Notify user-space event monitor about FW timeout */
            dx_pcie_enqueue_event_response(dev->id, ERR_FW_TIMEOUT);
            ret = -ETIMEDOUT;
            break;
        }

        /* Check for pending signals so user-space can cancel via Ctrl-C etc. */
        if (signal_pending(current)) {
            pr_debug(MODULE_NAME "%s: interrupted by signal.\n", __func__);
            ret = -ERESTARTSYS;
            break;
        }

        /* Yield CPU time while waiting for firmware ack to avoid soft lockups. */
        if (sleep_ms >= 20)
            msleep(sleep_ms);
        else
            usleep_range(sleep_ms * 1000, sleep_ms * 1000 + 500);
        cond_resched();
    }

    return ret;
}

static void dxrt_msg_to_dev(struct dxdev *dev, dxrt_message_t *msg)
{
    dx_msg_write32(dev, cmd, msg->cmd);
    dx_msg_write32(dev, sub_cmd, msg->sub_cmd);
    dx_msg_write32(dev, size, msg->size);
    dx_msg_write32(dev, ack, 0);
}

/**
 * dxrt_msg_general - Read/Write data from/to the dxrt device 
 * @dev: The deepx device on kernel
 * @msg: User-space pointer including the data buffer
 *
 * This function copies the user-space datas to deepx device provided by the ioctl command.
 * Also, this function is general interface to communicate with the deepx device.
 * 
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel).
 *        -ETIMEDOUT if an error occurs during waiting from response of deepx device
 *        -ENOMEM    if an error occurs during memory allocation on kernel space
 */
static int dxrt_msg_general(struct dxdev *dev, dxrt_message_t *msg, struct dxrt_file_ctx *ctx)
{
    int ret, num = dev->id;
    (void)ctx;
    pr_debug("%s: %d, %d: %llx %d\n", __func__, dev->id, dev->type, (uint64_t)msg->data, msg->size);
    if (dev->type == DX_STD) {
        ret = 0;
    } else {
        if (dev->msg) {
            mutex_lock(&dev->msg_lock);
            // dev->msg->cmd = msg->cmd;
            if (msg->size>0 && msg->size<sizeof(dxrt_device_message_t)) {
                // dev->msg->size = msg->size;
                if (dxrt_copy_from_user_io(num, DX_MSG_DATA_ADDR(dev), (void __user*)msg->data, msg->size)) {
                    pr_debug("%d: %s: failed.\n", num, __func__);
                    return -EFAULT;
                }
            }
            dxrt_msg_to_dev(dev, msg);
            dx_pcie_notify_msg_to_device(num);
            ret = dxrt_polling_ack(dev, 1);
            {
                u32 dev_msg_size = dx_msg_read32(dev, size);
                if (dev_msg_size>0 && dev_msg_size<sizeof(dxrt_device_message_t) && ret==0) {
                    ret = dxrt_copy_to_user_io(num, (void __user*)(msg->data), DX_MSG_DATA_ADDR(dev), dev_msg_size);
                }
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

/**
 * dxrt_identify_device - Read data from the dxrt device
 * @dev: The deepx device on kernel
 * @msg: User-space pointer including the data buffer
 *
 * This function reads data[memory / size..] from the deepx device and
 * copies it to the user-space buffer provided by the ioctl command.
 *
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel).
 *        -ETIMEDOUT if an error occurs during waiting from response of deepx device
 *        -ENOMEM    if an error occurs during memory allocation on kernel space
 *        -ECOMM     if an error occurs because of pcie data transaction fail
 */
static int dxrt_identify_device(struct dxdev* dev, dxrt_message_t *msg, struct dxrt_file_ctx *ctx)
{
    int ret, num = dev->id;
    dxrt_device_info_t info;
    (void)ctx;
    pr_debug("%s: %d, %d: %llx, %d\n", __func__, dev->id, dev->type, (uint64_t)msg->data, msg->size);
    info.type = dev->type;
    info.variant = dev->variant;

    if (dev->type == DX_STD)
    {
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
        dev->mem_addr = dev->npu->dma_buf_addr;
        dev->mem_size = dev->npu->dma_buf_size;
        dev->num_dma_ch = 1;
        info.mem_addr = dev->npu->dma_buf_addr;
        info.mem_size = dev->npu->dma_buf_size;
        info.num_dma_ch = 1;
        pr_debug("%d: %s: [%llx, %llx], %d\n", num, __func__,
            info.mem_addr, info.mem_size,
            info.num_dma_ch);
        ret = dev->npu->prepare_inference(dev->npu);
        if (msg->data!=NULL)
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
        if (msg->sub_cmd == DX_IDENTIFY_FWUPLOAD) {
            if (dx_get_flash_ready(dev->dl, 5*1000*100)) { /* Timeout: 5s */
                pr_info("> Enter Flash mode(boot_step:%d)\n", dx_get_boot_step(dev->dl));
                ret = 0;
                info.mem_addr = 0xD3000000; //dummy
                info.mem_size = 0x200000; //dummy
                info.num_dma_ch = 3;
                info.interface = 1;
                info.fw_ver = 202;
                dev->mem_addr = info.mem_addr;
                dev->mem_size = info.mem_size;
                dev->num_dma_ch = info.num_dma_ch;
                if (copy_to_user((void __user*)msg->data, &info, sizeof(info))) {
                    pr_debug("%d: %s failed.\n", num, __func__);
                    return -EFAULT;
                }
            } else {
                return -ECOMM;
            }
        } else { 
            if (dev->msg) {
                ret = dxrt_msg_general(dev, msg, ctx);
                if (ret<0) return -1;
                if (copy_from_user(&info, (void __user*)(msg->data), sizeof(info))) {
                    pr_err("%d: %s failed to copy data from user space.\n", num, __func__);
                    return -EFAULT;
                }
                pr_debug("%d: %s: [%llx, %llx], %d\n", num, __func__,
                    info.mem_addr, info.mem_size,
                    info.num_dma_ch);
                dev->mem_addr = info.mem_addr;
                dev->mem_size = info.mem_size;
                dev->num_dma_ch = info.num_dma_ch;
            } else {
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
        }
        return ret;
    }
    // return ret;
}

static void disable_queue(struct dxdev* dev, req_queue_t queue)
{
    switch (queue)
    {
    case DX_NORMAL_QUEUE0:
        if (dev->request_queue)
            dxrt_disable_queue(dev->request_queue);
        break;
    case DX_NORMAL_QUEUE1:
        if (dev->request_queue1)
            dxrt_disable_queue(dev->request_queue1);
        break;
    case DX_NORMAL_QUEUE2:
        if (dev->request_queue2)
            dxrt_disable_queue(dev->request_queue2);
        break;
    case DX_HIGH_QUEUE:
        if (dev->request_high_queue)
            dxrt_disable_queue(dev->request_high_queue);
        break;
    default:
        break;
    }
}
/**
 * dxrt_schedule - Send scheduler datas to the dxrt device
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function copies it to the user-space buffer provided by the ioctl command.
 *
 * Return: 0 on success,
 *        -EFAULT   if an error occurs during the copy(user <-> kernel)
 *        -ENOMEM   if an error occurs as memory allocation fail
 *                  This return is only for sub command of 'DX_SCHED_ADD'
 *        -EBUSY    There are no queues to assign to the user.
 *                  User scheduler options need modification.
 *                  This return is only for sub command of 'DX_SCHED_ADD'
 *        -ENOENT   There are no matching queues in the list.
 *                  This return is only for sub command of 'DX_SCHED_DELETE'
 *        -EINVAL   if an error occurs as sub-command is not supported
 *        -ETIMEDOUT if an error occurs during waiting from response of deepx device
 */
static int dxrt_schedule(struct dxdev* dev, dxrt_message_t *msg, struct dxrt_file_ctx *ctx)
{
    int ret = 0, num = dev->id;
    dx_shced_data data;
    (void)ctx;
    if (dev->type == DX_ACC) {
        if (msg->data!=NULL) {
            if (copy_from_user(&data.bound, (void __user*)msg->data, sizeof(npu_bound_op))) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            if (data.bound >= N_BOUND_INF_MAX) return -EINVAL;
            pr_debug("%d: %s - %s\n", num, __func__,
                (msg->sub_cmd == DX_SCHED_ADD) ? "ADD" : "DELETE");

            if (msg->sub_cmd == DX_SCHED_ADD) {
                ret = add_queue_from_sched_op(dev, data.bound);
                if (ret == 0) {
                    ret = get_queue_from_sched_op(dev, data.bound, &data.queue);
                } else if (ret == -EEXIST) {
                    return 0;
                }
            } else if (msg->sub_cmd == DX_SCHED_DELETE) {
                ret = get_queue_from_sched_op(dev, data.bound, &data.queue);
                if (ret == 0) {
                    ret = delete_matching_queue(dev, data.bound);
                    if (ret == 0) {
                        disable_queue(dev, data.queue);
                    } else if (ret == -EBUSY) {
                        return 0;
                    }
                }
            } else {
                pr_err("%s:Not matched sub_cmd(%d)\n", __func__, msg->sub_cmd);
                ret = -EINVAL;
            }
            pr_debug("%d: %s: sub_cmd:%d, bound:%d, queue:%d\n", num, __func__, msg->sub_cmd, data.bound, data.queue);
            if (ret == 0) {
                if (dev->msg) {
                    void __iomem *data_base = DX_MSG_DATA_ADDR(dev);
                    mutex_lock(&dev->msg_lock);
                    dxrt_msg_to_dev(dev, msg);
                    writel(data.bound, data_base);
                    writel(data.queue, data_base + sizeof(u32));
                    dx_pcie_notify_msg_to_device(num);
                    ret = dxrt_polling_ack(dev, 1);
                    mutex_unlock(&dev->msg_lock);
                }
            }
        }
    }
    return ret;
}

/**
 * dxrt_write_mem - Write data to the dxrt device
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function copies it to the user-space buffer provided by the ioctl command.
 *
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel)
 *        -ECOMM     if an error occurs because of pcie data transaction fail
 */
static int dxrt_write_mem(struct dxdev* dev, dxrt_message_t *msg, struct dxrt_file_ctx *ctx)
{
    int ret = 0, num = dev->id;
    uint32_t ch;
    dxrt_req_meminfo_t meminfo;
    (void)ctx;
    pr_debug("%d: %s\n", num, __func__);

    /* Reject DMA writes while recovery/SBR is in progress.
     * Without this, a client process can enter the expensive SG
     * setup path only to be rejected at the kernel admission gate. */
    if (atomic_read(&dev->recovering))
        return -EBUSY;

    if (msg->data!=NULL)
    {
        if (copy_from_user(&meminfo, (void __user*)msg->data, sizeof(dxrt_req_meminfo_t))) {
            pr_debug("%d: %s: failed.\n", num, __func__);
            return -EFAULT;
        }
        ch = meminfo.ch;
        pr_debug( MODULE_NAME "%d:%d %s: [%llx, %llx + %x(%x), %llx(%llx)]\n",
            num, ch,
            __func__,
            meminfo.data,
            meminfo.base,
            meminfo.offset,
            meminfo.size,
            dev->mem_addr,
            dev->mem_size
        );
        if (ch>MAX_PCIE_CH_NUM) {
            pr_err( MODULE_NAME "%d: %s: invalid channel.\n", num, __func__);
            return -EINVAL;
        }
        if ( meminfo.base + meminfo.offset < dev->mem_addr ||
            meminfo.base + meminfo.offset + meminfo.size > dev->mem_addr + dev->mem_size )
        {
            pr_err("%d: %s: invalid address: %llx + %x @ %llx, %llx \n",
                num,
                __func__,
                meminfo.base,
                meminfo.offset,
                dev->mem_addr,
                dev->mem_size);
            if (!dxrt_fault_inject_skip_addr_check)
                return -EINVAL;
            pr_warn("%d: %s: FAULT_INJECT: bypassing address check!\n",
                num, __func__);
        }
#if IS_STANDALONE        
        if (dev->type == DX_STD)
        {
            pr_debug( MODULE_NAME "%d:%d %s: [%llx, %llx + %x(%x), %x]\n",
                num, ch,
                __func__,
                meminfo.data,
                meminfo.base,
                meminfo.offset,
                meminfo.size,
                dev->npu->dma_buf
            );

            if (copy_from_user(dev->npu->dma_buf + meminfo.offset, (void __user*)meminfo.data, meminfo.size)) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            //sbi_l2cache_flush(meminfo.base + meminfo.offset, meminfo.size);
            //caches_clean_inval_pou(meminfo.data + meminfo.offset, meminfo.data + meminfo.offset + meminfo.size);//virtual address
            ret = 0;
        }
        else
#endif        
        {
            if (meminfo.data) {
                ssize_t size = dx_sgdma_write((char *)meminfo.data,
                    meminfo.base + meminfo.offset,
                    meminfo.size,
                    num,
                    ch,
                    false,
                    USER_SPACE_BUF, 0);
                if (size != meminfo.size)
                {
                    pr_err("Pcie write error!(%ld)\n", size);
                    ret = -ECOMM;
                }
            } else {
                pr_debug("%s:Write Data is null.\n", __func__);
            }
        }
    }
    return ret;
}

static int dxrt_npu_run_requset_acc(struct dxdev* dev, dxrt_request_acc_t *req, dxrt_queue_t *queue, struct mutex *lock, int q_num)
{
    int ret = 0, num = dev->id;

    /* Reject requests while recovery is in progress */
    if (atomic_read(&dev->recovering))
        return -EBUSY;

    mutex_lock(lock);
    if (dxrt_is_queue_full(queue)) {
        ret = -EBUSY;
        pr_debug( "%s: %d rejected.\n", __func__, req->req_id);
    } else {
        if ((ret = dxrt_lock_queue(queue))) {
            (void)dx_pcie_notify_req_to_device(num, q_num, 1);
            if ((ret = dxrt_lock_check(queue))) {
                ret = dxrt_enqueue(queue, req);
                dxrt_unlock_queue(queue);
                (void)dx_pcie_notify_req_to_device(num, q_num, 0);
                if (dxrt_enqueue_irq_done(queue) != 0) {
                    pr_err("enqueue interrupt handshake timeout(%d)\n", req->req_id);
                    ret = -EINVAL;
                }
            } else {
                pr_err("lock check failed(device is abnomal state)\n");
                ret = -EINVAL;
            }
        }
    }
    mutex_unlock(lock);
    return ret;
}

static int dxrt_write_req_to_dev_acc(struct dxdev* dev, dxrt_request_acc_t *req, dxrt_queue_t *queue, struct mutex *lock, int q_num)
{
    int ret = 0, num = dev->id;
    if (req->input.data) {
        ssize_t size = dx_sgdma_write((char *)req->input.data,
            req->input.base + req->input.offset,
            req->input.size,
            num,
            req->dma_ch,
            false,
            USER_SPACE_BUF, 0);
        if (size != req->input.size) {
            pr_err("Pcie input write error!(%ld)\n", size);
            dx_pcie_enqueue_event_response(num, ERR_PCIE_DMA_CH0_FAIL + req->dma_ch);
            ret = -ECOMM;
        }
    } else {
        pr_debug("%s:Input Data is null.\n", __func__);
    }
    if (ret == 0) {
        dxrt_npu_run_requset_acc(dev, req, queue, lock, q_num);
    }
    return ret;
}

/**
 * dxrt_write_input 
 *  - Write input data to the dxrt device and model meta-datas insert queue
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function copies user datas to memory of deepx device by the ioctl command.
 * Model meta datas insert to queue via pcie if user datas copy to device successfully.
 *
 * Return: 0 on success,
 *        -EFAULT   if an error occurs during the copy(user <-> kernel)
 *        -EBUSY    if an error occurs inserting queue as the queue is full (retry) 
 *        -EINVAL   if an error occurs inserting queue as the queue is disable
 *                  if an error occurs as interrupt handshake is fail with device
 *                  if an error occurs as sub-command is not supported
 *        -ECOMM    if an error occurs because of pcie data transaction fail
 *        -ENOENT   There are no matching queues in the list.
 */
static int dxrt_write_input(struct dxdev* dev, dxrt_message_t *msg, struct dxrt_file_ctx *ctx)
{
    int ret = -1, num = dev->id;
    pr_debug("%d: %s\n", num, __func__);
    if (dev->type == DX_ACC) {
        dxrt_request_acc_t req;
        if (msg->data!=NULL) {
            if (copy_from_user(&req, (void __user*)msg->data, sizeof(req))) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            ret = get_queue_from_sched_op(dev, req.bound, &req.queue);
            pr_debug("%d: %s: ch %d, req %d, type %d, [%d cmds @ %x, weight @ %x], [%d cmds @ %x, weight @ %x], input @ %llx+%x(%x), output @ %llx+%x(%x), %x, pr:%d, bw:%d, bd:%d, q:%d\n", 
                num, __func__, req.dma_ch,
                req.req_id, req.model_type, 
                req.model_cmds, req.cmd_offset, req.weight_offset,
                req.model_cmds2, req.cmd_offset2, req.weight_offset2,
                req.input.base, req.input.offset, req.input.size,
                req.output.base, req.output.offset, req.output.size,
                req.arg0, req.prior, req.bandwidth, req.bound,
                req.queue
            );
            if (ret == 0) {
                switch (req.queue) {
                    case DX_NORMAL_QUEUE0:
                        ret = dxrt_write_req_to_dev_acc(dev,
                            &req,
                            dev->request_queue,
                            &dev->request_queue_mutex,
                            DX_NORMAL_QUEUE0);
                        break;
                    case DX_NORMAL_QUEUE1:
                        ret = dxrt_write_req_to_dev_acc(dev,
                            &req,
                            dev->request_queue1,
                            &dev->request_queue1_mutex,
                            DX_NORMAL_QUEUE1);
                        break;
                    case DX_NORMAL_QUEUE2:
                        ret = dxrt_write_req_to_dev_acc(dev,
                            &req,
                            dev->request_queue2,
                            &dev->request_queue2_mutex,
                            DX_NORMAL_QUEUE2);
                        break;
                    case DX_HIGH_QUEUE:
                        ret = dxrt_write_req_to_dev_acc(dev,
                            &req,
                            dev->request_high_queue,
                            &dev->request_high_queue_mutex,
                            DX_HIGH_QUEUE);
                        break;
                    default:
                        pr_err("%s:sub_command error:%d", __func__, msg->sub_cmd);
                        ret = -	EINVAL;
                        break;
                }
            } else {
                pr_err("%s: no matching queues in the list(%d)\n", __func__, ret);
            }
        }
        return ret;
    } else {
        dxrt_request_t req;
        if (msg->data!=NULL) {
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

/**
 * dxrt_npu_run_request 
 *  - Write model meta-datas insert queue
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function copies user datas to memory of deepx device by the ioctl command.
 * Model meta datas insert to queue via pcie if user datas copy to device successfully.
 *
 * Return: 0 on success,
 *        -EFAULT   if an error occurs during the copy(user <-> kernel)
 *        -EBUSY    if an error occurs inserting queue as the queue is full (retry) 
 *        -EINVAL   if an error occurs inserting queue as the queue is disable
 *                  if an error occurs as interrupt handshake is fail with device
 *                  if an error occurs as sub-command is not supported
 *        -ENOENT   There are no matching queues in the list.
 */
static int dxrt_npu_run_request(struct dxdev* dev, dxrt_message_t *msg, struct dxrt_file_ctx *ctx)
{
    int ret = -1, num = dev->id;
    (void)ctx;
    pr_debug("%d: %s\n", num, __func__);
    if (dev->type == DX_ACC) {
        dxrt_request_acc_t req;
        if (msg->data!=NULL) {
            if (copy_from_user(&req, (void __user*)msg->data, sizeof(req))) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            ret = get_queue_from_sched_op(dev, req.bound, &req.queue);
            pr_debug("%d: %s: ch %d, req %d, type %d, [%d cmds @ %x, weight @ %x], [%d cmds @ %x, weight @ %x], input @ %llx+%x(%x), output @ %llx+%x(%x), %x, pr:%d, bw:%d, bd:%d, q:%d\n", 
                num, __func__, req.dma_ch,
                req.req_id, req.model_type, 
                req.model_cmds, req.cmd_offset, req.weight_offset,
                req.model_cmds2, req.cmd_offset2, req.weight_offset2,
                req.input.base, req.input.offset, req.input.size,
                req.output.base, req.output.offset, req.output.size,
                req.arg0, req.prior, req.bandwidth, req.bound,
                req.queue
            );
            if (ret == 0) {
                switch (req.queue) {
                    case DX_NORMAL_QUEUE0:
                        ret = dxrt_npu_run_requset_acc(dev,
                            &req,
                            dev->request_queue,
                            &dev->request_queue_mutex,
                            DX_NORMAL_QUEUE0);
                        break;
                    case DX_NORMAL_QUEUE1:
                        ret = dxrt_npu_run_requset_acc(dev,
                            &req,
                            dev->request_queue1,
                            &dev->request_queue1_mutex,
                            DX_NORMAL_QUEUE1);
                        break;
                    case DX_NORMAL_QUEUE2:
                        ret = dxrt_npu_run_requset_acc(dev,
                            &req,
                            dev->request_queue2,
                            &dev->request_queue2_mutex,
                            DX_NORMAL_QUEUE2);
                        break;
                    case DX_HIGH_QUEUE:
                        ret = dxrt_npu_run_requset_acc(dev,
                            &req,
                            dev->request_high_queue,
                            &dev->request_high_queue_mutex,
                            DX_HIGH_QUEUE);
                        break;
                    default:
                        pr_err("%s:sub_command error:%d", __func__, msg->sub_cmd);
                        ret = -	EINVAL;
                        break;
                }
            } else {
                pr_err("%s: no matching queues in the list(%d)\n", __func__, ret);
            }
        }
        return ret;
    } else {
        int ret = -1;
        /* TODO */
        return ret;
    }
}


/**
 * dxrt_npu_run_response 
 *  - Pop device response data from queue
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function copies data on deepx device memory to user buffer by the ioctl command.
 * If there is data in the internal response queue,
 * 
 * Return: 0 on success,
 *        -EFAULT   if an error occurs during the copy(user <-> kernel)
 *        -ENODATA  if an error occurs inserting queue as the queue is full (retry)
 *        -EINVAL   if an error occurs because the pcie dma channel is not supported
 */
static int dxrt_npu_run_response_v1(struct dxdev* dev, dxrt_message_t *msg, struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    pr_debug("%d: %s (v1)\n", num, __func__);
    if (dev->type == DX_ACC) {
        dx_pcie_response_t response = {0};
        int ret = -1;
        unsigned int mask = 0;
        uint32_t ch;
        if (msg->data!=NULL)
        {
            if (copy_from_user(&ch, (void __user*)msg->data, sizeof(ch))) {
                pr_err( MODULE_NAME "%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            if (ch>MAX_PCIE_CH_NUM) {
                pr_err( MODULE_NAME "%d: %s: invalid channel.\n", num, __func__);
                return -EINVAL;
            }
            if (atomic_read(&dev->response_pending[ch]) == 0) {
                pr_debug(MODULE_NAME "%d: %s, ch%d: start to wait.\n", num, __func__, ch);
                mask = dx_pcie_interrupt(num, ch);
                pr_debug(MODULE_NAME "%d: %s, ch%d: wake up.\n", num, __func__, ch);
            } else {
                dx_pcie_interrupt_clear(num, ch);
                mask = 1;
            }
            if (mask==1) {
                if ((ret = dx_pcie_dequeue_response(num, ch, &response)) != 0) {
                    ret = -ENODATA;
                }
                if (ret == 0) {
                    atomic_dec(&dev->response_pending[ch]);
                    ret = dxrt_copy_resp_to_user_acc(dev, msg, &response);
                }
            }
        }
        return ret;
    } else {
        int ret = -1;
        /* TODO */
        return ret;
    }
}

/**
 * dxrt_npu_run_response_v2 - Wait for and retrieve NPU inference response (V2)
 * @dev: The deepx device structure
 * @msg: User-space message containing channel info and response buffer
 * @ctx: File context containing termination flag
 *
 * This function waits for an inference response on the specified channel.
 * It supports graceful termination when the file descriptor is closed.
 * The response contains proc_id from firmware for multi-process routing.
 *
 * Return: 0 on success,
 *        -EFAULT    if copy_from/to_user fails
 *        -EINVAL    if invalid channel specified
 *        -ECANCELED if terminated due to close() (graceful shutdown)
 *        -ERESTARTSYS if interrupted by signal
 *        -ENODATA   if failed to dequeue response
 */
static int dxrt_npu_run_response_v2(struct dxdev *dev, dxrt_message_t *msg,
				    struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    pr_debug("%d: %s (v2)\n", num, __func__);

    if (dev->type == DX_ACC) {
        dx_pcie_response_t response = {0};
        int ret = -1;
        int wait_ret;
        uint32_t ch;

        if (msg->data != NULL) {
            if (copy_from_user(&ch, (void __user*)msg->data, sizeof(ch))) {
                pr_err(MODULE_NAME "%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            if (ch > MAX_PCIE_CH_NUM) {
                pr_err(MODULE_NAME "%d: %s: invalid channel.\n", num, __func__);
                return -EINVAL;
            }

            /* Check termination before waiting */
            if (ctx && atomic_read(&ctx->terminating)) {
                pr_debug("%d: %s: terminated before wait\n", num, __func__);
                return -ECANCELED;
            }

            /* Reject immediately if recovery is in progress */
            if (atomic_read(&dev->recovering))
                return -ENODATA;

            wait_ret = wait_event_interruptible(
                dev->response_wq[ch],
                atomic_read(&dev->response_pending[ch]) > 0 ||
                    atomic_read(&dev->recovering) ||
                    (ctx && atomic_read(&ctx->terminating)));

            if (wait_ret == -ERESTARTSYS) {
                pr_debug("%d: %s: interrupted by signal\n", num, __func__);
                return -ERESTARTSYS;
            }

            /* Check termination after wakeup */
            if (ctx && atomic_read(&ctx->terminating)) {
                pr_debug("%d: %s: terminated during wait\n", num, __func__);
                return -ECANCELED;
            }

            /* Check recovery after wakeup — response queues were cleared */
            if (atomic_read(&dev->recovering)) {
                pr_debug("%d: %s: recovery in progress\n", num, __func__);
                return -ENODATA;
            }

            /* FIFO dequeue - response contains proc_id from firmware */
            if ((ret = dx_pcie_dequeue_response(num, ch, &response)) != 0) {
                ret = -ENODATA;
            }
            if (ret == 0) {
                atomic_dec(&dev->response_pending[ch]);
                pr_debug("%d: %s: ch %d, proc_id %d, req_id %d done\n",
                    num, __func__, ch, response.proc_id, response.req_id);
                ret = dxrt_copy_resp_to_user_acc(dev, msg, &response);
            }
        }
        return ret;
    } else {
        int ret = -1;
        /* TODO */
        return ret;
    }
}

/**
 * dxrt_read_output 
 *  - Read output data from the dxrt device and pop device response data from queue
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function copies data on deepx device memory to user buffer by the ioctl command.
 * If there is data in the internal response queue,
 * the data is read from the deepx device immediately.
 * Otherwise, it waits until the queue is not empty.
 * 
 * Return: 0 on success,
 *        -EFAULT   if an error occurs during the copy(user <-> kernel)
 *        -ENODATA  if an error occurs inserting queue as the queue is full (retry)
 *        -EINVAL   if an error occurs because the pcie dma channel is not supported
 *        -ECOMM    if an error occurs because of pcie data transaction fail
 */
static int dxrt_read_output(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    pr_debug("%d: %s\n", num, __func__);
    if (dev->type == DX_ACC) {
        dx_pcie_response_t response = {0};
        int ret = -1;
        int wait_ret;
        // unsigned long flags;
        uint32_t ch;
        if (msg->data!=NULL)
        {
            if (copy_from_user(&ch, (void __user*)msg->data, sizeof(ch))) {
                pr_err( MODULE_NAME "%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            if (ch>MAX_PCIE_CH_NUM) {
                pr_err( MODULE_NAME "%d: %s: invalid channel.\n", num, __func__);
                return -EINVAL;
            }
            if (ctx && atomic_read(&ctx->terminating))
                return -ECANCELED;
            if (atomic_read(&dev->recovering))
                return -ENODATA;
            wait_ret = wait_event_interruptible(
                dev->response_wq[ch],
                atomic_read(&dev->response_pending[ch]) > 0 ||
                    atomic_read(&dev->recovering) ||
                    (ctx && atomic_read(&ctx->terminating)));
            if (wait_ret == -ERESTARTSYS)
                return -ERESTARTSYS;
            if (ctx && atomic_read(&ctx->terminating))
                return -ECANCELED;
            if (atomic_read(&dev->recovering))
                return -ENODATA;
            if ((ret = dx_pcie_dequeue_response(num, ch, &response)) != 0) {
                ret = -ENODATA;
            }
            if (ret == 0) {
                atomic_dec(&dev->response_pending[ch]);
            }
            if (ret == 0 && response.req_id>0)
            {
                if ((response.model_type==0) || (response.model_type==2))
                {
                    ssize_t size;
                    size = dx_sgdma_read((char *)response.data,
                        response.base + response.offset,
                        response.size,
                        num,
                        ch,
                        USER_SPACE_BUF);
                    if (size != response.size)
                    {
                        pr_err("Pcie output read error!(%ld)\n", size);
                        response.status = ERR_PCIE_DMA_CH0_FAIL + ch;
                        dx_pcie_enqueue_event_response(num, ERR_PCIE_DMA_CH0_FAIL + ch);
                        ret = -ECOMM;
                    }
                }
                if (ret == 0)
                    ret = dxrt_copy_resp_to_user_acc(dev, msg, &response);
            }
        }
        return ret;
    } else {
        dxnpu_t *npu = dev->npu;
        // dxrt_response_t response;
        int ret = -1;
        unsigned long flags;
        if (msg->data!=NULL) {
            if (list_empty(&dev->responses.list)) {
                pr_debug(MODULE_NAME "%d: %s: start to wait.\n", num, __func__);
                ret = wait_event_interruptible(npu->irq_wq,
                    npu->irq_event==1 || atomic_read(&ctx->terminating));
                pr_debug(MODULE_NAME "%d: %s: wake up.\n", num, __func__);
                if (atomic_read(&ctx->terminating))
                    return -ECANCELED;
                spin_lock_irqsave(&npu->irq_event_lock, flags);
                npu->irq_event = 0;
                spin_unlock_irqrestore(&npu->irq_event_lock, flags);
            }
            {
                spin_lock_irqsave(&dev->responses_lock, flags);
                if (!list_empty(&dev->responses.list)) {
                    dxrt_response_list_t *entry = list_first_entry(&dev->responses.list, dxrt_response_list_t, list);
                    pr_debug(MODULE_NAME "%d: %s: %d\n", num, __func__, entry->response.req_id);
                    if (copy_to_user((void __user*)msg->data, &entry->response, sizeof(dxrt_response_t))) {
                        pr_err( MODULE_NAME "%d: %s: memcpy failed.\n", num, __func__);
                        return -EFAULT;
                    }
                    list_del(&entry->list);
                    kfree(entry);
                    ret = 0;
                } else {
                    pr_debug(MODULE_NAME "%d: %s: empty\n", num, __func__);
                    ret = -1;
                }
                spin_unlock_irqrestore(&dev->responses_lock, flags);
            }
        }
        return ret;
    }
}

/**
 * dxrt_terminate_event - Wakeup wait event.
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * If the user wants to terminate normally,
 * the corresponding API is called and the driver is notified of termination.
 * 
 * Return: 0 on success,
 *        Currently no other return values ​​are defined. 
 */
static int dxrt_terminate_event(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    (void)ctx;
    pr_debug("%d:%d %s\n", num, current->tgid, __func__);
    if (dev->type == DX_ACC)
    {        
        unsigned int mask = 0;
        mask = dx_pcie_interrupt_event_wakeup(num);
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

/**
 * dxrt_terminate - Wakeup output event.
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * If the user wants to terminate normally,
 * the corresponding API is called and the driver is notified of termination.
 * 
 * Return: 0 on success,
 *        Currently no other return values ​​are defined. 
 */
static int dxrt_terminate(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    pr_debug(MODULE_NAME "%d: %s\n", num, __func__);
    if (dev->type == DX_ACC)
    {        
        unsigned int mask = 0;
        // unsigned long flags;
        uint32_t ch;
        if (msg->data!=NULL)
        {
            if (copy_from_user(&ch, (void __user*)msg->data, sizeof(ch))) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
            if (ch<0 || ch>MAX_PCIE_CH_NUM) {
                pr_debug("%d: %s: invalid channel.\n", num, __func__);
                return -EINVAL;
            }  
            pr_debug("%d:%d %s, %d\n", num, current->tgid, __func__, ch);
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

/**
 * dxrt_read_mem - Read data from the dxrt device
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function copies data on deepx device memory to user buffer by the ioctl command
 *
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel)
 *        -EINVAL    if an error occurs because of invalid address from user
 *        -ECOMM     if an error occurs because of pcie data transaction fail
 */
static int dxrt_read_mem(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    uint32_t ch;
    dxrt_req_meminfo_t meminfo;
    (void)ctx;
    pr_debug("%d: %s\n", num, __func__);

    /* Reject DMA reads while recovery/SBR is in progress. */
    if (atomic_read(&dev->recovering))
        return -EBUSY;

    if (msg->data!=NULL) {
        if (copy_from_user(&meminfo, (void __user*)msg->data, sizeof(meminfo))) {
            pr_debug("%d: %s: failed.\n", num, __func__);
            return -EFAULT;
        }
        ch = meminfo.ch;
        pr_debug( MODULE_NAME "%d:%d %s: [%llx, %llx + %x(%x), %llx(%llx)]\n",
            num, ch,
            __func__,
            meminfo.data,
            meminfo.base,
            meminfo.offset,
            meminfo.size,
            dev->mem_addr,
            dev->mem_size
        );
        if (ch>MAX_PCIE_CH_NUM) {
            pr_err( MODULE_NAME "%d: %s: invalid channel.\n", num, __func__);
            return -EINVAL;
        }
        if ( meminfo.base + meminfo.offset < dev->mem_addr - 0x100000 ||
            meminfo.base + meminfo.offset + meminfo.size > dev->mem_addr + dev->mem_size )
        {
            pr_err("%d: %s: invalid address: %llx + %x @ %llx, %llx \n",
                num,
                __func__,
                meminfo.base,
                meminfo.offset,
                dev->mem_addr,
                dev->mem_size);
            if (!dxrt_fault_inject_skip_addr_check)
                return -EINVAL;
            pr_warn("%d: %s: FAULT_INJECT: bypassing address check!\n",
                num, __func__);
        }
        if (dev->type == DX_STD) {
            if (copy_to_user((void __user*)meminfo.data, dev->npu->dma_buf + meminfo.offset,  meminfo.size)) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                return -EFAULT;
            }
        } else {
            if (meminfo.data) {
                ssize_t size = dx_sgdma_read((char *)meminfo.data,
                    meminfo.base + meminfo.offset,
                    meminfo.size,
                    num,
                    ch,
                    USER_SPACE_BUF); /*TODO*/
                if (size != meminfo.size) {
                    pr_err("PCIe read error!(%ld)\n", size);
                    return -ECOMM;
                }
            } else {
                pr_err("%s:Read Data is null.\n", __func__);
            }
        }
    }
    return 0;
}

/**
 * dxrt_cpu_cache_flush - Execute cpu cache flush
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function is executed by the ioctl command
 * If you feel that copying data between master devices
 * causes a data inconsistency problem recognized by the CPU,
 * you can call the corresponding API.
 * 
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel)
 *        -EINVAL    if an error occurs because of invalid address from user
 */
static int dxrt_cpu_cache_flush(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    dxrt_meminfo_t meminfo;
    (void)ctx;
    pr_debug("%d: %s: %llx\n", num, __func__, (uint64_t)msg->data);
    if (msg->data!=NULL) {
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
        if ( meminfo.base + meminfo.offset < dev->mem_addr ||            
            meminfo.base + meminfo.offset + meminfo.size > dev->mem_addr + dev->mem_size ) {
            pr_debug("%d: %s: invalid address: %llx + %x\n", num, __func__, meminfo.base, meminfo.offset);
            return -EINVAL;
        }
#if IS_STANDALONE        
        if (dev->type == DX_STD) {
            //sbi_l2cache_flush(meminfo.base + meminfo.offset, meminfo.size);//physical address
            //caches_clean_inval_pou(meminfo.data + meminfo.offset, meminfo.data + meminfo.offset + meminfo.size);//virtual address
        }
#endif         
    }
   
    return 0;
}

static int dxrt_soc_custom(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int ret = -1, num = dev->id;
    (void)ctx;
    pr_info("%d: %s: %llx\n", num, __func__, (uint64_t)msg->data);
    if (dev->type == DX_STD)
    {
        /* do nothing */
    }
    else
    {
        // pr_info("--> %p\n", dev->msg);

        mutex_lock(&dev->msg_lock);
        if (msg->size>0 && msg->size<sizeof(dxrt_device_message_t))
        {
            char *buffer = kmalloc(msg->size, GFP_KERNEL);
            if (!buffer) {
                mutex_unlock(&dev->msg_lock);
                return -ENOMEM;
            }
            if (copy_from_user(buffer, (void __user*)msg->data, msg->size)) {
                pr_debug("%d: %s: failed.\n", num, __func__);
                kfree(buffer);
                mutex_unlock(&dev->msg_lock);
                return -EFAULT;
            }
            dx_memcpy_toio32(DX_MSG_DATA_ADDR(dev), buffer, msg->size);
            kfree(buffer);
        }
        dxrt_msg_to_dev(dev, msg);
        dx_pcie_notify_msg_to_device(num);
        ret = dxrt_polling_ack(dev, 1);
        {
            u32 dev_msg_size = dx_msg_read32(dev, size);
            if (dev_msg_size>0 && dev_msg_size<sizeof(dxrt_device_message_t))
            {
                ret = dxrt_copy_to_user_io(num, (void __user*)msg->data, DX_MSG_DATA_ADDR(dev), dev_msg_size);
            }
        }
        mutex_unlock(&dev->msg_lock);
    }
    return 0;
}

static int dxrt_get_log(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int ret = 0, num = dev->id;
    (void)ctx;
    pr_debug("%s: %d, %d: %llx\n", __func__, dev->id, dev->type, (uint64_t)msg->data);
    if (dev->type == DX_STD) {
        ret = 0;
    } else {
        if (dxrt_copy_to_user_io(num, (void __user*)msg->data, dev->log, 16*1024)) {
            pr_debug("%d: %s failed.\n", num, __func__);
            ret = -EFAULT;
        }
    }
    return ret;
}

static int dxrt_update_firmware(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int ret, num = dev->id;
    (void)ctx;
    if (dev->type == DX_STD) {
        ret = 0;
    } else {
        uint32_t size = dx_pcie_get_download_size(num);
        dxrt_meminfo_t meminfo = {
            (uint64_t)(msg->data),
            dx_pcie_get_download_region(num),
            0,
            (msg->size>size) ? size : msg->size
        };
        pr_debug(MODULE_NAME "%d: %s: %llx, %llx, %x, %x\n", 
            num, __func__, meminfo.data, meminfo.base, meminfo.offset, meminfo.size
        );

        if (dev->msg) {
            if (msg->data!=NULL) {
                ssize_t size = dx_sgdma_write((char *)meminfo.data,
                    meminfo.base + meminfo.offset,
                    meminfo.size,
                    num,
                    0, /*TODO*/
                    false,
                    USER_SPACE_BUF, 0);
                if (size != meminfo.size) {
                    pr_err("Pcie write error!(%ld)\n", size);
                    ret = -1;
                }
            }
            mutex_lock(&dev->msg_lock);
            dxrt_msg_to_dev(dev, msg);
            dx_pcie_notify_msg_to_device(num);
            ret = dxrt_polling_ack(dev, 250);
            if (ret==0) {
                ret = (int)readl(DX_MSG_DATA_ADDR(dev));
            }
            mutex_unlock(&dev->msg_lock);
            pr_info(MODULE_NAME "%d: %s: done.\n", num, __func__);
        } else {
            ret = 0;
        }
    }
    return ret;
}

static int dxrt_write_firmware(int num, dxrt_message_t* msg, dma_addr_t dma_addr, uint64_t src, uint64_t dst)
{
    int ret = 0;
    dxrt_meminfo_t meminfo = {
        src,
        dst,
        0,
        msg->size
    };
    if (msg->data!=NULL) {
        ssize_t size = dx_sgdma_write(
            (char *)meminfo.data,
            meminfo.base + meminfo.offset,
            meminfo.size,
            num,
            0, /*TODO*/
            false,
            KERNEL_SPACE_BUF, dma_addr);
        if (size != meminfo.size) {
            pr_err("Pcie write error!(%ld)\n", size);
            ret = -ECOMM;
        }
    }
    return ret;
}

/**
 * dxrt_upload_firmware - Upload firmware file to device
 * 
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel)
 *        -EINVAL    if an error occurs because of invalid address from user
 *                   if an error occurs because device is wrong state(Invalid Boot Step / not ready / flash done fail)
 *        -ENOMEM    if an error occurs during memory allocation on kernel space
 *        -ECOMM     if an error occurs because of pcie data transaction fail
 */
static int dxrt_upload_firmware(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int ret, num = dev->id;
    (void)ctx;
    if (dev->type == DX_STD) {
        ret = 0;
    } else {
        dma_addr_t dma_addr;
        void *vaddr;
        int8_t boot_step;
        uint32_t size = 1*1024*1024;

        vaddr = dma_alloc_coherent(dev->dev, size, &dma_addr, GFP_KERNEL);
        if (!vaddr) {
            pr_err("%s:%d Failed to allocate coherent memory\n", __func__, num);
            return -ENOMEM;
        }
        if ((ret = copy_from_user(vaddr, (void __user *)msg->data, msg->size)) != 0) {
            pr_err("%s:%d Failed to copy data from user space, %d bytes couldn't be copied\n",
                 __func__, num, ret);
            ret = -EFAULT;
        }

        if (!ret) {
            if (dx_get_flash_ready(dev->dl, 2*1000*1000)) { /* timeout:2s */
                switch (boot_step = dx_get_boot_step(dev->dl)) {
                    case DX_ROM:
                        dxrt_write_firmware(num,
                            msg,
                            dma_addr,
                            (uint64_t)vaddr,
                            dx_pcie_get_booting_region(num, DX_ROM-1));
                        break;
                    case DX_2ND_BOOT:
                        dxrt_write_firmware(num,
                            msg,
                            dma_addr,
                            (uint64_t)vaddr,
                            dx_pcie_get_booting_region(num, DX_2ND_BOOT-1));
                        break;
                    default:
                        pr_err("invalid boot step(%d)", boot_step);
                        ret = -EINVAL;
                }
                if (!dx_get_flash_done(dev->dl)) {
                    pr_err("%s:%d flash done failed\n", __func__, num);
                    ret = -EINVAL;
                }
            } else {
                pr_err("%s:%d device is not ready\n", __func__, num);
                ret = -EINVAL;
            }
        }
        dma_free_coherent(dev->dev, size, vaddr, dma_addr);
    }
    return ret;
}

/**
 * dxrt_reset_device - Reset device
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function is executed by the ioctl command
 * User can reset device by using this function.
 *  [reset level]
 *    0 : NPU IP
 *    1 : entire device
 * 
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel)
 *        -ENOMEM    if an error occurs during memory allocation on kernel space
*/
static int dxrt_reset_device(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int ret, num = dev->id;
    (void)ctx;
    if (dev->type == DX_STD) {
        ret = 0;
    } else {
        if (dev->msg) {
            mutex_lock(&dev->msg_lock);
            if (msg->size>0 && msg->size<sizeof(dxrt_device_message_t)) {
                ret = dxrt_copy_from_user_io(num, DX_MSG_DATA_ADDR(dev), msg->data, msg->size);
            }
            dxrt_msg_to_dev(dev, msg);
            dx_pcie_notify_msg_to_device(num);
            mdelay(100);
            mutex_unlock(&dev->msg_lock);
        } else {
            ret = 0;
        }
    }
    return ret;
}

/**
 * dxrt_handle_event - Report event to user (only accelator device)
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function is executed by the ioctl command
 * 
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel)
*/
static int dxrt_handle_event(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    unsigned long flags;
    dx_pcie_dev_event_t dev_event;
    struct deepx_pcie_info info;
    (void)ctx;

    dev->error = 0;
    pr_debug(MODULE_NAME "%d:%d %s: start to wait. error %d\n", num, current->tgid, __func__, dev->error);
    {
        dx_pcie_dequeue_event_response(num, &dev_event);
        if (dev_event.event_type == DXRT_EVENT_ERROR) {
            dev->error = dev_event.dx_rt_err.err_code;
            dev->notify = NTFY_NONE;
        } else {
            dev->error = ERR_NONE;
            dev->notify = dev_event.dx_rt_ntfy_throt.ntfy_code;
        }
        dx_pcie_get_driver_info(&info, num);
    }
    pr_debug(MODULE_NAME "%d:%d %s: wake up. error %d\n", num, current->tgid, __func__, dev->error);

    spin_lock_irqsave(&dev->error_lock, flags);
    if (dev_event.event_type == DXRT_EVENT_ERROR) {
        dev_event.dx_rt_err.rt_driver_version          = DXRT_MOD_VERSION_NUMBER;
        strscpy(dev_event.dx_rt_err.rt_driver_version_suffix, __stringify(RT_VERSION_SUFFIX), sizeof(dev_event.dx_rt_err.rt_driver_version_suffix));
        dev_event.dx_rt_err.pcie_driver_version        = info.driver_version;
        dev_event.dx_rt_err.bus                        = info.bus;
        dev_event.dx_rt_err.dev                        = info.dev;
        dev_event.dx_rt_err.func                       = info.func;
        dev_event.dx_rt_err.speed                      = info.speed;
        dev_event.dx_rt_err.width                      = info.width;
    }
    spin_unlock_irqrestore(&dev->error_lock, flags);

    if (msg->data!=NULL) {
        if (copy_to_user((void __user*)msg->data, &dev_event, sizeof(dx_pcie_dev_event_t))) {
            pr_debug("%d: %s failed.\n", num, __func__);
            return -EFAULT;
        }
    }
    return 0;
}

/**
 * dxrt_handle_event_v2 - Report event to user with termination support (only accelerator device)
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 * @ctx: File context containing terminating flag
 *
 * This function is executed by the ioctl command.
 * V2 version supports graceful termination via close().
 * Also handles PROC_EXIT events from the proc_exit_queue.
 * 
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel)
 *        -ECANCELED if termination is requested
 */
static int dxrt_handle_event_v2(struct dxdev *dev, dxrt_message_t *msg,
				struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    unsigned long flags;
    dx_pcie_dev_event_t dev_event;
    struct deepx_pcie_info info;
    int wait_ret;
    pid_t exit_proc_id;

    if (ctx && atomic_read(&ctx->terminating))
        return -ECANCELED;

retry_after_recovery:
    /*
     * If recovery is in progress, wait for it to complete rather than
     * returning -ECANCELED.  Returning immediately causes user-space
     * event threads to busy-loop, flooding the log with errors.
     */
    if (atomic_read(&dev->recovering)) {
        pr_debug(MODULE_NAME "%d:%d %s: recovery in progress, waiting...\n",
            num, current->tgid, __func__);
        wait_ret = wait_event_interruptible(dev->event_wq,
            !atomic_read(&dev->recovering) ||
            (ctx && atomic_read(&ctx->terminating)));
        if (wait_ret == -ERESTARTSYS)
            return -ERESTARTSYS;
        if (ctx && atomic_read(&ctx->terminating))
            return -ECANCELED;
        /* Recovery done — fall through to normal event wait.
         * Queues were cleared during recovery, so the wait below
         * will block until the next real event arrives. */
    }

    dev->error = 0;
    pr_debug(MODULE_NAME "%d:%d %s: start to wait. error %d\n",
        num, current->tgid, __func__, dev->error);

    /* Wait for event with termination and recovery check */
    /* Check regular events, proc_exit events, and recovery flag */
    wait_ret = wait_event_interruptible(
        dev->event_wq,
        dx_pcie_is_event_pending(num) ||
            (dx_pcie_is_proc_exit_pending(num) > 0) ||
            atomic_read(&dev->recovering) ||
            (ctx && atomic_read(&ctx->terminating)));
    if (wait_ret == -ERESTARTSYS)
        return -ERESTARTSYS;
    if (ctx && atomic_read(&ctx->terminating))
        return -ECANCELED;
    /* If woken by recovery starting, loop back and wait for it to finish
     * instead of returning -ECANCELED to user-space. */
    if (atomic_read(&dev->recovering))
        goto retry_after_recovery;

    /* Check for PROC_EXIT event first (priority) */
    if (dx_pcie_dequeue_proc_exit_event(num, &exit_proc_id) == 0) {
        memset(&dev_event, 0, sizeof(dev_event));
        dev_event.event_type = DXRT_EVENT_PROC_EXIT;
        dev_event.dx_rt_proc_exit.proc_id = exit_proc_id;
        pr_debug(MODULE_NAME "%d: %s: proc_exit event, proc_id=%d (caller=%d)\n",
            num, __func__, exit_proc_id, current->tgid);

        if (msg->data != NULL) {
            if (copy_to_user((void __user *)msg->data, &dev_event,
                     sizeof(dx_pcie_dev_event_t))) {
                pr_err("%d: %s failed.\n", num, __func__);
                return -EFAULT;
            }
        }
        return 0;
    }

    /* Handle regular events */
    dx_pcie_dequeue_event_response(num, &dev_event);
    if (dev_event.event_type == DXRT_EVENT_ERROR) {
        dev->error = dev_event.dx_rt_err.err_code;
        dev->notify = NTFY_NONE;
    } else {
        dev->error = ERR_NONE;
        dev->notify = dev_event.dx_rt_ntfy_throt.ntfy_code;
    }
    dx_pcie_get_driver_info(&info, num);

    pr_debug(MODULE_NAME "%d:%d %s: wake up. error %d\n",
        num, current->tgid, __func__, dev->error);

    spin_lock_irqsave(&dev->error_lock, flags);
    if (dev_event.event_type == DXRT_EVENT_ERROR) {
        dev_event.dx_rt_err.rt_driver_version          = DXRT_MOD_VERSION_NUMBER;
        strscpy(dev_event.dx_rt_err.rt_driver_version_suffix,
            __stringify(RT_VERSION_SUFFIX),
            sizeof(dev_event.dx_rt_err.rt_driver_version_suffix));
        dev_event.dx_rt_err.pcie_driver_version        = info.driver_version;
        dev_event.dx_rt_err.bus                        = info.bus;
        dev_event.dx_rt_err.dev                        = info.dev;
        dev_event.dx_rt_err.func                       = info.func;
        dev_event.dx_rt_err.speed                      = info.speed;
        dev_event.dx_rt_err.width                      = info.width;
    }
    spin_unlock_irqrestore(&dev->error_lock, flags);

    if (msg->data != NULL) {
        if (copy_to_user((void __user *)msg->data, &dev_event,
                 sizeof(dx_pcie_dev_event_t))) {
            pr_err("%d: %s failed.\n", num, __func__);
            return -EFAULT;
        }
    }
    return 0;
}

/**
 * dxrt_handle_rt_drv_info_sub - Get device driver version
 * @dev: The deepx device on kernel structure
 * @msg: User-space pointer including the data buffer
 *
 * This function is executed by the ioctl command
 * 
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel)
 *        -EINVAL    if an error occurs because of unsupported command from user
*/
static int dxrt_handle_rt_drv_info_sub(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int num = dev->id;
    int ret = 0;
    (void)ctx;
    pr_debug(MODULE_NAME "%d: %s, [%d]\n", num, __func__, msg->sub_cmd);
    switch (msg->sub_cmd) {
        case DRVINFO_CMD_GET_RT_INFO:
            if (msg->data!=NULL) {
                struct dxrt_drv_info info;
                info.driver_version = DXRT_MOD_VERSION_NUMBER;
                if (copy_to_user((void __user*)msg->data, &info, sizeof(info))) {
                    pr_err("%d: %s cmd:%d failed.\n", num, __func__, msg->sub_cmd);
                    ret = -EFAULT;
                }
            }
            break;
        case DRVINFO_CMD_GET_PCIE_INFO:
            if (dev->type == DX_ACC) {
                struct deepx_pcie_info info;
                dx_pcie_get_driver_info(&info, num);
                if (msg->data!=NULL) {
                    if (copy_to_user((void __user*)msg->data, &info, sizeof(info))) {
                        pr_err("%d: %s cmd:%d failed.\n", num, __func__, msg->sub_cmd);
                        ret = -EFAULT;
                    }
                }
            } else {
                pr_err("CMD(%d) is not supported for device type(%d)\n", msg->cmd, dev->type);
            }
            break;
        case DRVINFO_CMD_GET_RT_INFO_V2:
            if (msg->data!=NULL) {
                struct dxrt_drv_info_v2 info;
                info.driver_version = DXRT_MOD_VERSION_NUMBER;
                strscpy(info.driver_version_suffix, __stringify(RT_VERSION_SUFFIX), sizeof(info.driver_version_suffix));
                if (copy_to_user((void __user*)msg->data, &info, sizeof(info))) {
                    pr_err("%d: %s cmd:%d failed.\n", num, __func__, msg->sub_cmd);
                    ret = -EFAULT;
                }
            }
            break;
    default:
        pr_err("Unsupported sub command(%d/%d)\n", msg->cmd, msg->sub_cmd);
        ret = -EINVAL;
        break;
    }
    return ret;
}

/**
 * dxrt_recovery_device - Driver and firmware recovery in unusual situations
 * @dev: The deepx device on kernel
 * @msg: User-space pointer including the data buffer
 *
 * This function copies the user-space datas to deepx device provided by the ioctl command.
 * Also, this function is general interface to communicate with the deepx device.
 * 
 * Return: 0 on success,
 *        -EFAULT    if an error occurs during the copy(user <-> kernel).
 *        -ETIMEDOUT if an error occurs during waiting from response of deepx device
 *        -ENOMEM    if an error occurs during memory allocation on kernel space
 */
static int dxrt_recovery_device(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    int ret = 0;
    int i, num = dev->id;
    (void)ctx;
    if (dev->type == DX_STD) {
        /* TODO */
    } else {
        pr_info(MODULE_NAME "%d: %s: starting recovery\n", num, __func__);

        /* 1. Set recovering flag early — blocks new requests
         *    and enables waiter threads to detect recovery wakeup */
        atomic_set(&dev->recovering, 1);
        atomic_inc(&dev->recovery_epoch);

        /* 2. Clear scheduler queue list */
        clear_queue_list(dev);

        /* 3. Reset DMA channels - terminate in-flight transfers
         *    Returns: 0 = success, 1 = PCIe SBR performed (FW was reset),
         *             <0 = error */
        ret = dx_pcie_reset_dma_channels(num);
        if (ret < 0)
            pr_warn(MODULE_NAME "%d: %s: DMA channel reset error (%d)\n",
                num, __func__, ret);

        /* 4. Clear PCIe response queues */
        dx_pcie_clear_response_queue(num);

        /* 4b. Clear stale event queue to prevent spurious events post-recovery */
        dx_pcie_clear_event_response(num);

        /* 4c. Clear proc_exit queue — stale exit events from terminated
         *     processes are no longer relevant after full recovery. */
        dx_pcie_clear_proc_exit_queue(num);

        /* 5. Reset response_pending counters */
        for (i = 0; i <= MAX_PCIE_CH_NUM; i++)
            atomic_set(&dev->response_pending[i], 0);

        /* 6. Disable and re-enable HW request queues (under mutex to
         *    prevent collision with concurrent request submission) */
        if (dev->request_queue) {
            mutex_lock(&dev->request_queue_mutex);
            dxrt_disable_queue(dev->request_queue);
            dxrt_enable_queue(dev->request_queue);
            mutex_unlock(&dev->request_queue_mutex);
        }
        if (dev->request_queue1) {
            mutex_lock(&dev->request_queue1_mutex);
            dxrt_disable_queue(dev->request_queue1);
            dxrt_enable_queue(dev->request_queue1);
            mutex_unlock(&dev->request_queue1_mutex);
        }
        if (dev->request_queue2) {
            mutex_lock(&dev->request_queue2_mutex);
            dxrt_disable_queue(dev->request_queue2);
            dxrt_enable_queue(dev->request_queue2);
            mutex_unlock(&dev->request_queue2_mutex);
        }
        if (dev->request_high_queue) {
            mutex_lock(&dev->request_high_queue_mutex);
            dxrt_disable_queue(dev->request_high_queue);
            dxrt_enable_queue(dev->request_high_queue);
            mutex_unlock(&dev->request_high_queue_mutex);
        }

        /* 7. Wake up any threads waiting for responses or events
         *    Waiters check dev->recovering and return -ENODATA/-ECANCELED */
        for (i = 0; i <= MAX_PCIE_CH_NUM; i++)
            wake_up_interruptible(&dev->response_wq[i]);
        wake_up_interruptible(&dev->event_wq);

        /* 8. Notify firmware of recovery.
         *    Skip if PCIe SBR was performed (ret==1 from step 3) because
         *    SBR resets the entire endpoint including FW — the shared
         *    memory mailbox is gone, so dxrt_msg_general would timeout
         *    waiting for an ack that will never come.
         *    After SBR, FW re-initialization is handled by the boot
         *    sequence (romcode → bootloader → FreeRTOS). */
        if (ret == 1) {
            pr_info(MODULE_NAME "%d: %s: PCIe SBR was performed, skipping FW notify\n",
                num, __func__);

            /* SBR reset the entire endpoint including FW.
             * Invalidate cached device memory range so stale values
             * don't cause false address validation failures.
             * The next DXRT_CMD_IDENTIFY will refresh them from FW. */
            dev->mem_addr = 0;
            dev->mem_size = 0;

            /* Trigger dxrt_device_init() on the next ioctl call
             * so that dev->msg, dev->dl, dev->request_queue etc.
             * are refreshed to match the rebooted FW layout. */
            dx_pcie_set_init_completed(num);

            ret = 0;
        } else if (ret == 0) {
            ret = dxrt_msg_general(dev, msg, ctx);
        }
        /* else ret < 0: DMA reset failed, skip FW notify */

        /* 9. Clear recovering flag — normal operation resumes */
        atomic_set(&dev->recovering, 0);

        /* Wake up event and response waiters that are blocking on
         * !recovering.  Step 7 above wakes them while recovering is
         * still 1, so they re-sleep.  This second wakeup lets them
         * see recovering==0 and proceed. */
        for (i = 0; i <= MAX_PCIE_CH_NUM; i++)
            wake_up_interruptible(&dev->response_wq[i]);
        wake_up_interruptible(&dev->event_wq);

        pr_info(MODULE_NAME "%d: %s: recovery %s (ret=%d)\n",
            num, __func__, ret ? "failed" : "done", ret);
    }
    return ret;
}

static int dxrt_handle_drv_info(struct dxdev* dev, dxrt_message_t* msg, struct dxrt_file_ctx *ctx)
{
    return dxrt_handle_rt_drv_info_sub(dev, msg, ctx);
}

int message_handler_general(struct dxdev *dx, dxrt_message_t *msg, struct dxrt_file_ctx *ctx)
{
    return message_handler[msg->cmd](dx, msg, ctx);
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
    [DXRT_CMD_TERMINATE_EVENT]      = dxrt_terminate_event,
    [DXRT_CMD_SOC_CUSTOM]           = dxrt_soc_custom,
    [DXRT_CMD_GET_STATUS]           = dxrt_msg_general,
    [DXRT_CMD_RESET]                = dxrt_reset_device,
    [DXRT_CMD_UPDATE_CONFIG]        = dxrt_msg_general,
    [DXRT_CMD_UPDATE_CONFIG_JSON]   = dxrt_msg_general,
    [DXRT_CMD_UPDATE_FIRMWARE]      = dxrt_update_firmware,
    [DXRT_CMD_GET_LOG]              = dxrt_get_log,
    [DXRT_CMD_DUMP]                 = dxrt_msg_general,
    [DXRT_CMD_EVENT]                = dxrt_handle_event,
    [DXRT_CMD_DRV_INFO]             = dxrt_handle_drv_info,
    [DXRT_CMD_SCHEDULE]             = dxrt_schedule,
    [DXRT_CMD_UPLOAD_FIRMWARE]      = dxrt_upload_firmware,
    [DXRT_CMD_NPU_RUN_REQ]          = dxrt_npu_run_request,
    [DXRT_CMD_NPU_RUN_RESP]         = dxrt_npu_run_response_v1,
    [DXRT_CMD_NPU_RUN_RESP_V2]      = dxrt_npu_run_response_v2,
    [DXRT_CMD_EVENT_V2]             = dxrt_handle_event_v2,
    [DXRT_CMD_RECOVERY]             = dxrt_recovery_device,
    [DXRT_CMD_CUSTOM]               = dxrt_msg_general,
    [DXRT_CMD_START]                = dxrt_msg_general,
    [DXRT_CMD_TERMINATE]            = dxrt_terminate,
    [DXRT_CMD_PCIE]                 = dxrt_msg_general,
};