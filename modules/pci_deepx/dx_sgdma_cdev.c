// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <linux/scatterlist.h>
#include <asm/cacheflush.h>
#include <linux/delay.h>

#include "dx_sgdma_cdev.h"
#include "dx_lib.h"
#include "dw-edma-thread.h"
#include "dw-edma-v0-core.h"
#include "dx_util.h"

#define IOCTL_PRINT 1

int dx_sgdma_cdev_open(struct inode *inode, struct file *filp);
int dx_sgdma_cdev_release(struct inode *inode, struct file *filp);
ssize_t dx_sgdma_cdev_read(struct file *filp, char __user *buf, size_t count, loff_t *pos);
ssize_t dx_sgdma_cdev_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos);
long dx_sgdma_cdev_ioctl(struct file *filp, unsigned int cmd, unsigned long data);
static loff_t dx_sgdma_cdev_llseek(struct file *file, loff_t off, int whence);

/*
 * character device file operations for SG DMA engine
 */
static loff_t dx_sgdma_cdev_llseek(struct file *file, loff_t off, int whence)
{
	loff_t newpos = 0;

	switch (whence) {
	case 0: /* SEEK_SET */
		newpos = off;
		break;
	case 1: /* SEEK_CUR */
		newpos = file->f_pos + off;
		break;
	case 2: /* SEEK_END, @TODO should work from end of address space */
		newpos = UINT_MAX + off;
		break;
	default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0)
		return -EINVAL;
	file->f_pos = newpos;
	dbg_sg("%s: pos=0x%llx\n", __func__, (signed long long)newpos);

#if 0
	pr_err("0x%p, off %lld, whence %d -> pos %lld.\n",
		file, (signed long long)off, whence, (signed long long)off);
#endif

	return newpos;
}

/* when open() is called */
int dx_sgdma_cdev_open(struct inode *inode, struct file *filp) {
	struct dx_dma_cdev *xcdev;
	struct dw_edma *dw;

	char_open(inode, filp);
	xcdev = (struct dx_dma_cdev *)filp->private_data;
	dw = xcdev->xpdev->dw;
	xcdev->f_count++;

	dbg_sg("[%s][%s][%s] Device_#%d NPU_#%d called!(count:%d)\n", __func__,
		!xcdev->write ? "W" : "R",
		xcdev->sys_device->kobj.name,
		dw->idx, xcdev->npu_id,
		xcdev->f_count);
	/* TODO - Check device busy */

	/* DMA Channel allocation */
	if (xcdev->write) {
		dw_edma_dma_allocation(dw->rd_dma_id, xcdev->npu_id, &dw->wr_dma_chan[xcdev->npu_id]);
	} else {
		dw_edma_dma_allocation(dw->wr_dma_id, xcdev->npu_id, &dw->rd_dma_chan[xcdev->npu_id]);
	}

	return 0;
}

/* when last close() is called */
int dx_sgdma_cdev_release(struct inode *inode, struct file *filp) {
	struct dx_dma_cdev *xcdev = (struct dx_dma_cdev *)filp->private_data;
	struct dw_edma *dw = xcdev->xpdev->dw;
	xcdev->f_count--;

	dbg_sg("[%s][%s][%s] Device_#%d NPU_#%d called!(count:%d)\n", __func__,
		!xcdev->write ? "W" : "R",
		xcdev->sys_device->kobj.name,
		dw->idx, xcdev->npu_id,
		xcdev->f_count);

	// ret = dw_edma_get(dw->idx, xcdev->npu_id);
	if (xcdev->f_count == 0) {
		if (xcdev->write)
			dw_edma_dma_deallocation(&dw->wr_dma_chan[xcdev->npu_id]);
		else
			dw_edma_dma_deallocation(&dw->rd_dma_chan[xcdev->npu_id]);
	}

	return 0;
}

long dx_sgdma_cdev_ioctl(struct file *filp, unsigned int cmd, unsigned long data) {
	switch (cmd) {
	case IOCTL_PRINT:
		printk(KERN_ALERT "[%s] IOCTL_PRINT called!", __func__);
		break;
	default:
		printk(KERN_ALERT "[%s] unknown command!", __func__);
		break;
	}

	return 0;
}

/* Copy userspace buffer to kernel buffer  */
ssize_t dx_sgdma_cdev_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos) {
	struct dx_dma_cdev *xcdev = (struct dx_dma_cdev *)filp->private_data;
	struct dw_edma *dw = xcdev->xpdev->dw;
	size_t ret;

	ret = dx_sgdma_write_user(dw, (char *)buf, (u64)*pos, count, xcdev->npu_id, false);

	return ret;
}

/* copy kernel buffer to userspace buffer, saved by write */
ssize_t dx_sgdma_cdev_read(struct file *filp, char __user *buf, size_t count, loff_t *pos) {
	struct dx_dma_cdev *xcdev = (struct dx_dma_cdev *)filp->private_data;
	struct dw_edma *dw = xcdev->xpdev->dw;
	size_t ret;

	ret = dx_sgdma_read_user(dw, (char *)buf, (u64)*pos, count, xcdev->npu_id);

	return ret;
}

static const struct file_operations sgdma_fops = {
  .owner           = THIS_MODULE,
  .read            = dx_sgdma_cdev_read,
  .write           = dx_sgdma_cdev_write,
  .open            = dx_sgdma_cdev_open,
  .release         = dx_sgdma_cdev_release,
  .unlocked_ioctl  = dx_sgdma_cdev_ioctl,
  .llseek          = dx_sgdma_cdev_llseek,
};

void dx_cdev_sgdma_init(struct dx_dma_cdev *xcdev)
{
	cdev_init(&xcdev->cdev, &sgdma_fops);
}
