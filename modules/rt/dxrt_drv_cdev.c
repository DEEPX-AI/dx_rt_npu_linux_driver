// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/of.h>

#if DEVICE_TYPE==1
/* L2 cache flush api */
//#include <asm/sbi.h>
#endif

#include "dxrt_drv.h"
#if DEVICE_TYPE==0
#include "dx_pcie_api.h"
#endif

static const u64 dmamask = DMA_BIT_MASK(32);

#if IS_STANDALONE 
static atomic_t dxdev_refcnt = ATOMIC_INIT(0);
#endif

static int dxrt_dev_open(struct inode *i, struct file *f)
{
    struct dxdev *dx;
    int num = iminor(f->f_inode);
    pr_debug( "%s: %s\n", f->f_path.dentry->d_iname, __func__);
    dx = container_of(i->i_cdev, struct dxdev, cdev);
    f->private_data = dx;
    if(dx->type==DX_ACC)
    {
        dx_sgdma_init(num);
    }
    else//dx->type==DX_STD
    {
        dx->npu->irq_event = 0;// irq init
    }	
    dx->response.req_id = 0;
    return 0;
}
static int dxrt_dev_release(struct inode *i, struct file *f)
{
    int num = iminor(f->f_inode);
    struct dxdev *dx = f->private_data;
    pr_debug( "%s: %s\n", f->f_path.dentry->d_iname, __func__);
    if(dx->type==DX_ACC)
    {
        dx_sgdma_deinit(num);
    }
    return 0;
}
static ssize_t dxrt_dev_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    // unsigned long flags;
    struct dxdev *dx = f->private_data;
    // dxrt_response_list_t *entry;
    // dxrt_response_list_t *responses = &dx->responses;
    pr_debug( "%s: %s\n", f->f_path.dentry->d_iname, __func__);

#if 0
    if(list_empty(&responses->list)) {
        pr_debug( "%s: read(): list empty\n", f->f_path.dentry->d_iname);
        return 0;
    }
    spin_lock_irqsave(&dx->responses_lock, flags);
    entry = list_first_entry(&responses->list, dxrt_response_list_t, list);
    spin_unlock_irqrestore(&dx->responses_lock, flags);
    if(!entry)
    {
        pr_err( "%s: queue empty\n", f->f_path.dentry->d_iname);
        return -EINVAL;
    }
    if(copy_to_user(buf, &entry->response, sizeof(dxrt_response_t))) {
        pr_err( "%s: failed to copy response\n", f->f_path.dentry->d_iname);
        return -EFAULT;
    }
    pr_debug( "%s: %s: response %d\n", f->f_path.dentry->d_iname, __func__, entry->response.req_id);
    list_del(&entry->list);
    kfree(entry);
#else
    if(copy_to_user(buf, &dx->response, sizeof(dxrt_response_t))) {
        pr_err( "%s: failed to copy response\n", f->f_path.dentry->d_iname);
        return -EFAULT;
    }
#endif
    return sizeof(dxrt_response_t);
}
static ssize_t dxrt_dev_write(struct file *f, const char __user *buf, size_t len,
    loff_t *off)
{
    // unsigned long flags;
    struct dxdev *dx = f->private_data;
    pr_debug( "%s: %s\n", f->f_path.dentry->d_iname, __func__);
    if(dx->request_handler)
    {
        dxrt_request_list_t *entry;
        entry = kmalloc(sizeof(dxrt_request_list_t), GFP_KERNEL);
        if(!entry)
        {
            printk(KERN_ALERT "Failed to allocate memory for request queue entry\n");
            return -ENOMEM;
        }    
        if (len != sizeof(dxrt_request_t)) {
            printk(KERN_ALERT "Invalid request size: %lu\n", len);
            return -EINVAL;
        }
        if (copy_from_user(&(entry->request), buf, len)) {
            printk(KERN_ALERT "Failed to copy request data from user space\n");
            return -EFAULT;
        }        
        spin_lock(&dx->requests_lock);
        list_add_tail(&entry->list, &dx->requests.list);
        spin_unlock(&dx->requests_lock);
        // {
        //     dxrt_request_t *req = &entry->request;
        //     pr_debug( MODULE_NAME "%d: %s: req %d, type %d, %d cmds @ %x, weight @ %x, input @ %llx+%x(%x), output @ %llx+%x(%x)\n", 
        //         num, __func__, req->req_id, req->model_type, req->model_cmds,
        //         req->cmd_offset, req->weight_offset,
        //         req->input.base, req->input.offset, req->input.size,
        //         req->output.base, req->output.offset, req->output.size
        //     );
        // }
        wake_up_interruptible(&dx->request_wq);
        return len;
    }
    return 0;
}
static int dxrt_dev_mmap(struct file *f, struct vm_area_struct *vma)
{
    int ret = -1;
    struct dxdev *dx = f->private_data;
    pr_debug( "%s: %s\n", f->f_path.dentry->d_iname, __func__);
    if(dx->type==DX_STD)
    {
        struct dxnpu *npu = dx->npu;
        unsigned long size = vma->vm_end - vma->vm_start;
        ret = dma_mmap_coherent(dx->dev, vma, npu->dma_buf, npu->dma_buf_addr, size);
        pr_debug( "%s: mmap 0x%lx bytes, returned %d\n", f->f_path.dentry->d_iname, size, ret);        
    }
    return ret;
}
static unsigned int dxrt_dev_poll(struct file *f, poll_table *wait)
{
    struct dxdev *dx = f->private_data;
    // int num = iminor(f->f_inode);
    unsigned int mask = 0;
    unsigned long flags;
    pr_debug( "%s: %s\n", f->f_path.dentry->d_iname, __func__);
    if(dx->type==DX_STD)
    {
        struct dxnpu *npu = dx->npu;
        poll_wait(f, &npu->irq_wq, wait);
        spin_lock_irqsave(&npu->irq_event_lock, flags);
        if(npu->irq_event)
        {
            mask = POLLIN | POLLRDNORM;
            npu->irq_event = 0;
        }
        spin_unlock_irqrestore(&npu->irq_event_lock, flags);
        return mask;
    }
    else
    {
        return 0;
    }
}

/**
 * dxrt_dev_ioctl - Handle ioctl commands for the deepx device
 * @f: Pointer to the file structure
 * @cmd: The ioctl command
 * @arg: The argument for the ioctl command
 *
 * This function handles various ioctl commands for the deepx device.
 * It performs different actions based on the command received.
 *
 * Supported commands:
 *   - DXRT_IOCTL_MESSAGE: Get device information
 *
 * Return: 0 on success,
 *        -EINVAL if the command is invalid,
 *        or appropriate error code for specific commands.
 */
static long dxrt_dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    int num = iminor(f->f_inode);
    struct dxdev *dx = f->private_data;
    dxrt_message_t msg;
    pr_debug( "%s: ioctl() cmd %d\n", f->f_path.dentry->d_iname, cmd);    
    if (_IOC_TYPE(cmd) != DXRT_IOCTL_MAGIC || \
        _IOC_NR(cmd) >= DXRT_IOCTL_MAX)
    {
        pr_debug( "%s: ioctl() invalid cmd %d\n", f->f_path.dentry->d_iname, cmd);
        return -EINVAL;
    }
    switch(cmd) {
        case DXRT_IOCTL_MESSAGE:
            if (copy_from_user(&msg, (void __user*)arg, sizeof(msg)))
            {
                pr_debug( MODULE_NAME "%d: ioctl cmd 0x%x failed.\n", num, cmd);
                return -EFAULT;
            }
            if(msg.cmd >=0 && msg.cmd < DXRT_CMD_MAX)
            {
                pr_debug( MODULE_NAME "%d: message %d\n", num, msg.cmd);
                if (dx_pcie_get_init_completed(dx->id)) {
                    dx_pcie_set_init_completed(dx->id);
                    dxrt_device_init(dx);
                }
                return message_handler_general(dx, &msg);
                // return message_handler[msg.cmd](dx, msg.data);
            }
            else
            {
                pr_debug( MODULE_NAME "%d: invalid message %d\n", num, msg.cmd);
                return -EINVAL;
            }
            break;
        default:
            pr_debug( MODULE_NAME "%d: ioctl cmd 0x%x: unknown command.\n", num, cmd);
            break;
    }
    return 0;
}

static struct file_operations dxrt_cdev_fops =
{
    .owner = THIS_MODULE,
    .open = dxrt_dev_open,
    .release = dxrt_dev_release,
    .read = dxrt_dev_read,
    .write = dxrt_dev_write,
    .mmap = dxrt_dev_mmap,
    .unlocked_ioctl = dxrt_dev_ioctl,
    .poll = dxrt_dev_poll
};

static struct dxdev* create_dxrt_device(int id, struct dxrt_driver *drv, struct file_operations *fops)
{
    int ret;
    struct dxdev* dxdev = kmalloc(sizeof(struct dxdev), GFP_KERNEL);
    if(!dxdev)
    {
        pr_err( "%s: failed to allocate memory\n", __func__);
        return NULL;
    }
    memset(dxdev, 0, sizeof(struct dxdev));

    dxdev->pdev = drv->pdev;
    dxdev->id = id;
    dxdev->type = DEVICE_TYPE;
    dxdev->variant = DEVICE_VARIANT;
    cdev_init(&dxdev->cdev, fops);
    dxdev->cdev.owner = THIS_MODULE;
    if ((ret = cdev_add(&dxdev->cdev, drv->dev_num + id, 1)) < 0)
    {
        pr_err( "%s: failed to add character device\n", __func__);
        device_destroy(drv->dev_class, drv->dev_num);
        class_destroy(drv->dev_class);
        unregister_chrdev_region(drv->dev_num, drv->num_devices);
        return NULL;
    }

    if (IS_ERR(dxdev->dev = device_create(drv->dev_class, NULL, drv->dev_num + id, NULL, MODULE_NAME"%d", id)))
    {
        pr_err( "%s: failed to create device\n", __func__);
        class_destroy(drv->dev_class);
        unregister_chrdev_region(drv->dev_num, drv->num_devices);
        return NULL;
    }
    dxdev->dev->dma_mask = (u64 *)&dmamask;
    dxdev->dev->coherent_dma_mask = DMA_BIT_MASK(32);
#if IS_STANDALONE
    dxdev->npu = dxrt_npu_init(dxdev);
#endif
    INIT_LIST_HEAD(&dxdev->requests.list);
    INIT_LIST_HEAD(&dxdev->responses.list);
    INIT_LIST_HEAD(&dxdev->sched);
    init_waitqueue_head(&dxdev->request_wq);
    init_waitqueue_head(&dxdev->error_wq);
    spin_lock_init(&dxdev->request_queue_lock);
    spin_lock_init(&dxdev->request_queue1_lock);
    spin_lock_init(&dxdev->request_queue2_lock);
    spin_lock_init(&dxdev->request_high_queue_lock);
    spin_lock_init(&dxdev->sched_lock);
    spin_lock_init(&dxdev->requests_lock);
    spin_lock_init(&dxdev->responses_lock);
    spin_lock_init(&dxdev->error_lock);
    mutex_init(&dxdev->msg_lock);
#if IS_STANDALONE
    if (dxdev->type == DX_STD) {
        dxdev->request_handler = kthread_run(
            dxrt_request_handler, (void*)dxdev, "dxrt-th%d", dxdev->id
        );
    }
#endif
    pr_info(" [%d] created device %d:%d:%d, %p, %p\n",
        dxdev->variant, id,
        MAJOR(drv->dev_num + id), MINOR(drv->dev_num + id), dxdev->dev, dxdev->npu);
    return dxdev;
}
static void remove_dxrt_device(struct dxrt_driver *drv, struct dxdev* dxdev)
{

    device_destroy(drv->dev_class, drv->dev_num + dxdev->id);
    cdev_del(&dxdev->cdev);
    kfree(dxdev);    
}

#if IS_STANDALONE
static int dxrt_dev_get_device_id(struct dxrt_driver *drv)
{
    int device_id_from_dtsi = 0;   
    struct platform_device *pdev = drv->pdev;
    struct device_node *np = pdev->dev.of_node; 
    const __be32 *prop = of_get_property(np, "device-id", NULL);
    if (prop==NULL)
    {
        pr_err( "%s: failed to find device-id for npu.\n", __func__);
        return NULL;
    }
    device_id_from_dtsi = be32_to_cpup(prop);

    return device_id_from_dtsi;
}
#endif

int dxrt_driver_cdev_init(struct dxrt_driver *drv)
{
    int i, ret;
    pr_debug( "%s\n", __func__);
#if NUM_DEVICES==0
    drv->num_devices = dx_pcie_get_dev_num();
#else
    drv->num_devices = NUM_DEVICES;
#endif
    pr_info("%s: %d devices\n", __func__, drv->num_devices);

#if IS_STANDALONE    
    if (atomic_inc_return(&dxdev_refcnt) == 1)//Only called during the initial driver creation phase
#endif    
    {
        if ((ret = alloc_chrdev_region(&drv->dev_num, 0, drv->num_devices, MODULE_NAME)) < 0)
        {
            return ret;
        }
    
        {
    #if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0))
            if (IS_ERR(drv->dev_class = class_create(MODULE_NAME)))
    #else
            if (IS_ERR(drv->dev_class = class_create(THIS_MODULE, MODULE_NAME)))
    #endif
            {
                unregister_chrdev_region(drv->dev_num, drv->num_devices);
                return PTR_ERR(drv->dev_class);
            }
        }
        //pr_info("%s: alloc_chrdev & class_create done!\n", __func__);
    }

#if IS_STANDALONE
	i = dxrt_dev_get_device_id(drv);
#else
    for(i=0;i<drv->num_devices;i++)
#endif	
    {
        drv->devices[i] = create_dxrt_device(i, drv, &dxrt_cdev_fops);
        if(drv->devices[i]==NULL)
        {
            return -1;
        }
    }
    pr_debug( "%s done.\n", __func__);
    return 0;

}
void dxrt_driver_cdev_deinit(struct dxrt_driver *drv)
{
    int i;
    dxnpu_t *npu;
    struct task_struct *request_handler;
    pr_debug( "%s\n", __func__);
#if IS_STANDALONE
	//i = dxrt_dev_get_device_id(drv);
    i = atomic_read(&dxdev_refcnt) - 1;//device_id    

    pr_info( "%s: device_id = %d\n", __func__, i);
#else
    for(i=0;i<drv->num_devices;i++)
#endif	
    {
        remove_dxrt_device(drv, drv->devices[i]);
        npu = drv->devices[i]->npu;
        request_handler = drv->devices[i]->request_handler;
        if(npu)
        {
            npu->deinit(npu);
        }
        if(request_handler)
        {
            kthread_stop(request_handler);
        }
    }

#if IS_STANDALONE    
    if (atomic_dec_return(&dxdev_refcnt) == 0)
#endif    
    {
        class_destroy(drv->dev_class);
        unregister_chrdev_region(drv->dev_num, drv->num_devices);
    }
    pr_debug( "%s done.\n", __func__);
}
