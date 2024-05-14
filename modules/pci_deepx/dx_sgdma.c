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
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
#include "dx_pcie_api.h"
#endif

enum mem_type {
	USER_SPACE_BUF   = 0,
	KERNEL_SPACE_BUF = 1,
};

static void char_sgdma_unmap_user_buf(struct dx_dma_io_cb *cb, bool write)
{
	int i;

	// sg_free_table(&cb->sgt);

	if (!cb->pages || !cb->pages_nr)
		return;

	for (i = 0; i < cb->pages_nr; i++) {
		if (cb->pages[i]) {
			if (!write)
				set_page_dirty_lock(cb->pages[i]);
			put_page(cb->pages[i]);
		} else
			break;
	}

	if (i != cb->pages_nr)
		pr_info("sgl pages %d/%u.\n", i, cb->pages_nr);

	kfree(cb->pages);
	cb->pages = NULL;
}

static int char_sgdma_map_user_buf_to_sgl(struct dx_dma_io_cb *cb, bool write, int dev_n, int dma_n)
{
	struct sg_table *sgt = &cb->sgt;
	unsigned long len = cb->len;
	void __user *buf = cb->buf;
	struct scatterlist *sg;
	unsigned int pages_nr = (((unsigned long)buf + len + PAGE_SIZE - 1) -
				 ((unsigned long)buf & PAGE_MASK))
				>> PAGE_SHIFT;
	int i;
	int rv;

	if (pages_nr == 0)
		return -EINVAL;

	dx_pcie_start_profile(PCIE_SG_TABLE_ALLOC_T, 0, dev_n, dma_n, write);
	if (sg_alloc_table(sgt, pages_nr, GFP_KERNEL)) {
		pr_err("sgl OOM.\n");
		return -ENOMEM;
	}
	dx_pcie_end_profile(PCIE_SG_TABLE_ALLOC_T, 0, dev_n, dma_n, write);

	cb->pages = kcalloc(pages_nr, sizeof(struct page *), GFP_KERNEL);
	if (!cb->pages) {
		pr_err("pages OOM.\n");
		rv = -ENOMEM;
		goto err_out;
	}
	dx_pcie_start_profile(PCIE_USER_PG_TO_PHY_MAPPING_T, 0, dev_n, dma_n, write);

	/* get physical pages from user pages */
	rv = get_user_pages_fast((unsigned long)buf, pages_nr, 1/* write */,
				cb->pages);

	/* No pages were pinned */
	if (rv < 0) {
		pr_err("unable to pin down %u user pages, %d.\n",
			pages_nr, rv);
		goto err_out;
	}
	/* Less pages pinned than wanted */
	if (rv != pages_nr) {
		pr_err("unable to pin down all %u user pages, %d.\n",
			pages_nr, rv);
		cb->pages_nr = rv;
		rv = -EFAULT;
		goto err_out;
	}

	for (i = 1; i < pages_nr; i++) {
		if (cb->pages[i - 1] == cb->pages[i]) {
			pr_err("duplicate pages, %d, %d.\n",
				i - 1, i);
			rv = -EFAULT;
			cb->pages_nr = pages_nr;
			goto err_out;
		}
	}

	sg = sgt->sgl;
	for (i = 0; i < pages_nr; i++, sg = sg_next(sg)) {
		unsigned int offset = offset_in_page(buf);
		unsigned int nbytes =
			min_t(unsigned int, PAGE_SIZE - offset, len);

		flush_dcache_page(cb->pages[i]);
		sg_set_page(sg, cb->pages[i], nbytes, offset);

		buf += nbytes;
		len -= nbytes;

		// pr_info("SG#%d Address:0x%llx, Page:%x, nbytes:%x\n",
		// 	i, sg_dma_address(sg), sg->page_link, nbytes);
	}
	dx_pcie_end_profile(PCIE_USER_PG_TO_PHY_MAPPING_T, 0, dev_n, dma_n, write);

	if (len) {
		pr_err("Invalid user buffer length. Cannot map to sgl\n");
		return -EINVAL;
	}
	cb->pages_nr = pages_nr;

	return 0;

err_out:
	char_sgdma_unmap_user_buf(cb, write);

	return rv;
}

ssize_t dx_sgdma_write_user(struct dw_edma *dw, char __user *buf, u64 pos, size_t count, int npu_id, bool npu_run)
{
	const char __user *ubuf = buf;
	struct dx_dma_io_cb cb;
	size_t ret;
	int rv;

	dx_pcie_start_profile(PCIE_KERNEL_EXEC_T, count, dw->idx, npu_id, 1);

	dbg_sg("[W] Dev#%d, buf 0x%p,%llu, pos 0x%llx, npu_id:%d\n",
		dw->idx, buf, (u64)count, pos, npu_id);

	if (!dw) {
		pr_err("[%s] priv pointer open error!(NULL)\n", __func__);
		return 0;
	}

	/*Check transfer align - TODO*/

	memset(&cb, 0, sizeof(struct dx_dma_io_cb));
	cb.buf     = (char __user *)ubuf;
	cb.len     = count;
	cb.ep_addr = pos;
	cb.write   = true;
	cb.dma_ch_id = dw->wr_dma_id;
	cb.npu_id  = npu_id;
	cb.npu_run = npu_run;

	rv = char_sgdma_map_user_buf_to_sgl(&cb, cb.write, dw->idx, npu_id);
	if (rv < 0)
		return rv;
	/*Transfer*/
	rv = dw_edma_run(&cb, dw->rd_dma_chan[npu_id], dw->idx, 0);
	char_sgdma_unmap_user_buf(&cb, cb.write);

	/*check result*/
	if (rv == 0) {
		if(cb.result)
			ret = -ERESTARTSYS;
		else
			ret = count;
	} else {
		ret = rv;
	}

	dx_pcie_end_profile(PCIE_KERNEL_EXEC_T, count, dw->idx, npu_id, 1);
	return ret;
}

ssize_t dx_sgdma_read_user(struct dw_edma *dw, char __user *buf, u64 pos, size_t count, int npu_id)
{
	const char __user *ubuf = buf;
	struct dx_dma_io_cb cb;
	size_t ret;
	int rv;

	dx_pcie_start_profile(PCIE_KERNEL_EXEC_T, count, dw->idx, npu_id, 0);

	dbg_sg("[R] Dev#%d, buf 0x%p,%llu, pos 0x%llx, npu_id:%d\n",
		dw->idx, ubuf, (u64)count, pos, npu_id);

	/*Check transfer align - TODO*/

	memset(&cb, 0, sizeof(struct dx_dma_io_cb));
	cb.buf = (char __user *)ubuf;
	cb.len = count;
	cb.ep_addr = pos;
	cb.write = false;
	cb.dma_ch_id = dw->rd_dma_id;
	cb.npu_id = npu_id;
	cb.npu_run = false;

	rv = char_sgdma_map_user_buf_to_sgl(&cb, cb.write, dw->idx, npu_id);
	if (rv < 0)
		return rv;

	/*Transfer*/
	rv = dw_edma_run(&cb, dw->wr_dma_chan[npu_id], dw->idx, 1);

	char_sgdma_unmap_user_buf(&cb, cb.write);

	/*check result*/
	if (rv == 0) {
		if(cb.result)
			ret = -ERESTARTSYS;
		else
			ret = count;
	} else {
		ret = rv;
	}

	dx_pcie_end_profile(PCIE_KERNEL_EXEC_T, count, dw->idx, npu_id, 0);
	return ret;
}
#if 0
static int dx_sgdma_sched_channel_push(struct dw_edma *dw, bool write)
{
	struct mutex *lock = (write) ? &dw->wr_lock : &dw->rd_lock;
	u16 *list = (write) ? dw->wr_dma_chan_list : dw->rd_dma_chan_list;
	u32 chan_nums = (write) ? dw->wr_ch_cnt : dw->rd_ch_cnt;
	u32 dma_chan_id = 0;
	int i;

	mutex_lock(lock);
	for(i = 0; i < chan_nums; i++) {
		if (list[i] == 0) {
			list[i] = 1;
			break;
		}
	}
	if (i == chan_nums) {
		pr_err("All dma channel is already busy\n");
		dma_chan_id = -1;
	} else {
		dma_chan_id = i;
		dbg_sg("Selected dma channel %d\n", dma_chan_id);
	}
	mutex_unlock(lock);

	return dma_chan_id;
}

static void dx_sgdma_sched_channel_pop(struct dw_edma *dw, int write, int dma_chan_id)
{
	struct mutex *lock = (write) ? &dw->wr_lock : &dw->rd_lock;
	u16 *list = (write) ? dw->wr_dma_chan_list : dw->rd_dma_chan_list;
	u32 chan_nums = (write) ? dw->wr_ch_cnt : dw->rd_ch_cnt;

	mutex_lock(lock);
	if (dma_chan_id < chan_nums) {
		list[dma_chan_id] = 0;
		dbg_sg("Released dma channel %d\n", dma_chan_id);
	} else {
		pr_err("Channel Transfer done channel id is not matched(%d)\n", dma_chan_id);
	}
	mutex_unlock(lock);
}

void dx_sgdma_sched_channel_init(struct dw_edma *dw)
{
	mutex_init(&dw->rd_lock);
	mutex_init(&dw->wr_lock);
}
#endif

/**
 * dx_sgdma_write - Write user buffer datas to end-point
 * @src: Address of end point
 * @dest: User buffer address
 * @count: Transfer size
 * @dev_id: Device id
 * @dma_ch: DMA Channel id
 * @mem_type: Where is the location of the source buffer[User/Kernel]
 * @npu_run: Whether or not to run npu after transmission
 *return - Fail:negative, Pass:transfered size
**/
ssize_t dx_sgdma_write(char *dest, u64 src, size_t count, int dev_id, int dma_ch, bool npu_run)
{
	enum mem_type type = USER_SPACE_BUF;
	ssize_t ret = 0;
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	// int chan_id = dx_sgdma_sched_channel_push(dw, true);
	if (type == USER_SPACE_BUF) {
		ret = dx_sgdma_write_user(dw, (char __user *)dest, src, count, dma_ch, npu_run);
	} else if (type == KERNEL_SPACE_BUF) {
		/*TODO*/
	} else {
		pr_err("undefined type[%d]\n", type);
	}
	// dx_sgdma_sched_channel_pop(dw, true, chan_id);

	return ret;
}
EXPORT_SYMBOL_GPL(dx_sgdma_write);

/**
 * dx_sgdma_read - Read end-point datas to user buffers
 * @dest: Address of end point
 * @src: User buffer address
 * @count: Transfer size
 * @dev_id: Device id
 * @dma_ch: DMA Channel id
 * @mem_type: Where is the location of the source buffer[User/Kernel]
 *return - Fail:negative, Pass:transfered size
**/
ssize_t dx_sgdma_read(char *src, u64 dest, size_t count, int dev_id, int dma_ch)
{
	enum mem_type type = USER_SPACE_BUF;
	ssize_t ret = 0;
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	// int chan_id = dx_sgdma_sched_channel_push(dw, true);
	if(type == USER_SPACE_BUF) {
		ret = dx_sgdma_read_user(dw, (char __user *)src, dest, count, dma_ch);
	} else if (type == KERNEL_SPACE_BUF) {
		/*TODO*/
	} else {
		pr_err("undefined type[%d]\n", type);
	}
	// dx_sgdma_sched_channel_pop(dw, true, chan_id);

	return ret;
}
EXPORT_SYMBOL_GPL(dx_sgdma_read);

/**
 * dx_sgdma_init - PCIe SGDMA initilization
 * @dev_id: Device id
**/
void dx_sgdma_init(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	int i;
	if (dw) {
		for (i=0; i<dw->rd_ch_cnt; i++) {
			dw_edma_dma_allocation(dw->rd_dma_id, i, &dw->wr_dma_chan[i]);
		}
		for (i=0; i<dw->wr_ch_cnt; i++) {
			dw_edma_dma_allocation(dw->wr_dma_id, i, &dw->rd_dma_chan[i]);
		}
	}
}
EXPORT_SYMBOL_GPL(dx_sgdma_init);

/**
 * dx_sgdma_init - PCIe SGDMA deinitilization
 * @dev_id: Device id
**/
void dx_sgdma_deinit(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	int i;
	if (dw) {
		for (i=0; i<dw->wr_ch_cnt; i++) {
			dw_edma_dma_deallocation(&dw->wr_dma_chan[i]);
		}
		for (i=0; i<dw->rd_ch_cnt; i++) {
			dw_edma_dma_deallocation(&dw->rd_dma_chan[i]);
		}
	}
}
EXPORT_SYMBOL_GPL(dx_sgdma_deinit);

MODULE_LICENSE("GPL");
