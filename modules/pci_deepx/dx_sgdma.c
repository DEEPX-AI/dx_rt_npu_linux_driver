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
#include <linux/version.h>

#include <linux/scatterlist.h>
#include <asm/cacheflush.h>
#include <linux/delay.h>
#include <linux/pci.h>

#include "dx_sgdma_cdev.h"
#include "dx_lib.h"
#include "dw-edma-thread.h"
#include "dw-edma-v0-core.h"
#include "dx_util.h"
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
#include "dx_pcie_api.h"
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(KVM_KERNEL_MAJ, KVM_KERNEL_MIN, KVM_KERNEL_PAT))
#include <linux/vmalloc.h>
#endif

static void char_sgdma_unmap_user_buf(struct dx_dma_io_cb *cb, bool write)
{
	int i;

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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(KVM_KERNEL_MAJ, KVM_KERNEL_MIN, KVM_KERNEL_PAT))
	kvfree(cb->pages);
#else
	kfree(cb->pages)
#endif
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

	dx_pcie_start_profile(PCIE_SG_ALLOC_T, 0, dev_n, dma_n, write);
	if (sg_alloc_table(sgt, pages_nr, GFP_KERNEL)) {
		pr_err("sgl OOM.\n");
		return -ENOMEM;
	}
	dx_pcie_end_profile(PCIE_SG_ALLOC_T, 0, dev_n, dma_n, write);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(KVM_KERNEL_MAJ, KVM_KERNEL_MIN, KVM_KERNEL_PAT))
	cb->pages = kvmalloc_array(pages_nr, sizeof(struct page *), GFP_KERNEL);
#else
	cb->pages = kcalloc(pages_nr, sizeof(struct page *), GFP_KERNEL);
#endif
	if (!cb->pages) {
		pr_err("pages OOM.\n");
		rv = -ENOMEM;
		goto err_out;
	}
	dx_pcie_start_profile(PCIE_USER_MAP_T, 0, dev_n, dma_n, write);

	/* get physical pages from user pages */
	rv = get_user_pages_fast((unsigned long)buf, pages_nr, !write,
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

	}
	dx_pcie_end_profile(PCIE_USER_MAP_T, 0, dev_n, dma_n, write);

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

	dx_pcie_start_profile(PCIE_TOTAL_TIME_T, count, dw->idx, npu_id, 1);

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
	cb.is_llm  = true;

	rv = char_sgdma_map_user_buf_to_sgl(&cb, cb.write, dw->idx, npu_id);
	if (rv < 0) {
		pr_err("Fail create sgdma map(wr)\n");
		return rv;
	}
	/*Transfer*/
	if (!dw->rd_dma_chan[npu_id]) {
		pr_err("[%s] dev %d: rd_dma_chan[%d] is NULL! (ref_count=%d, wr_ch_cnt=%u)\n",
			__func__, dw->idx, npu_id, dw->ref_count, dw->wr_ch_cnt);
		char_sgdma_unmap_user_buf(&cb, cb.write);
		return -ENODEV;
	}
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

	dx_pcie_end_profile(PCIE_TOTAL_TIME_T, count, dw->idx, npu_id, 1);
	return ret;
}

static int char_sgdma_map_kernel_buf_to_sgl(struct sg_table *sgt, void *cpu_addr, dma_addr_t dma_addr, size_t size)
{
    struct scatterlist *sg;
    int ret;

    // Allocate scatter-gather table with 1 entry, since the buffer is physically contiguous
    ret = sg_alloc_table(sgt, 1, GFP_KERNEL);
    if (ret) {
        pr_err("sg_alloc_table failed with error %d\n", ret);
        return ret;
    }

    sg = sgt->sgl;
    // Set the single entry in scatter-gather list
    sg_set_page(sg, virt_to_page(cpu_addr), size, offset_in_page(cpu_addr));
    sg_dma_address(sg) = dma_addr;
    sg_dma_len(sg) = size;

    return 0;
}

static ssize_t dx_sgdma_write_kernel(struct dw_edma *dw, char *buf, u64 pos, dma_addr_t dma_addr, size_t count, int npu_id, bool npu_run)
{
	struct dx_dma_io_cb cb;
	size_t ret;
	int rv;

	dx_pcie_start_profile(PCIE_TOTAL_TIME_T, count, dw->idx, npu_id, 1);

	dbg_sg("[W] Dev#%d, buf 0x%p,%llu, pos 0x%llx, npu_id:%d\n",
		dw->idx, buf, (u64)count, pos, npu_id);

	if (!dw) {
		pr_err("[%s] priv pointer open error!(NULL)\n", __func__);
		return 0;
	}

	/*Check transfer align - TODO*/

	memset(&cb, 0, sizeof(struct dx_dma_io_cb));
	cb.len     = count;
	cb.ep_addr = pos;
	cb.write   = true;
	cb.dma_ch_id = dw->wr_dma_id;
	cb.npu_id  = npu_id;
	cb.npu_run = npu_run;
	cb.is_llm  = false;	/* TODO */
	cb.pre_mapped = true;	/* Kernel buf from dma_alloc_coherent, already has DMA addr */

	/* Allocate scatter-gather table */
	char_sgdma_map_kernel_buf_to_sgl(&cb.sgt, buf, dma_addr, cb.len);

	/*Transfer*/
	if (!dw->rd_dma_chan[npu_id]) {
		pr_err("[%s] dev %d: rd_dma_chan[%d] is NULL! (ref_count=%d, wr_ch_cnt=%u)\n",
			__func__, dw->idx, npu_id, dw->ref_count, dw->wr_ch_cnt);
		return -ENODEV;
	}
	rv = dw_edma_run(&cb, dw->rd_dma_chan[npu_id], dw->idx, 0);
	/* need to dealloc function */

	/*check result*/
	if (rv == 0) {
		if(cb.result)
			ret = -ERESTARTSYS;
		else
			ret = count;
	} else {
		ret = rv;
	}

	dx_pcie_end_profile(PCIE_TOTAL_TIME_T, count, dw->idx, npu_id, 1);
	return ret;
}

ssize_t dx_sgdma_read_user(struct dw_edma *dw, char __user *buf, u64 pos, size_t count, int npu_id)
{
	const char __user *ubuf = buf;
	struct dx_dma_io_cb cb;
	size_t ret;
	int rv;

	dx_pcie_start_profile(PCIE_TOTAL_TIME_T, count, dw->idx, npu_id, 0);

	dbg_sg("[R] Dev#%d, buf 0x%p,%llu, pos 0x%llx, npu_id:%d\n",
		dw->idx, ubuf, (u64)count, pos, npu_id);

	/*Check transfer align - TODO*/

	memset(&cb, 0, sizeof(struct dx_dma_io_cb));
	cb.buf     = (char __user *)ubuf;
	cb.len     = count;
	cb.ep_addr = pos;
	cb.write   = false;
	cb.dma_ch_id = dw->rd_dma_id;
	cb.npu_id  = npu_id;
	cb.npu_run = false;
	cb.is_llm  = true;

	rv = char_sgdma_map_user_buf_to_sgl(&cb, cb.write, dw->idx, npu_id);
	if (rv < 0) {
		pr_err("Fail create sgdma map(rd)\n");
		return rv;
	}

	/*Transfer*/
	if (!dw->wr_dma_chan[npu_id]) {
		pr_err("[%s] dev %d: wr_dma_chan[%d] is NULL! (ref_count=%d, rd_ch_cnt=%u)\n",
			__func__, dw->idx, npu_id, dw->ref_count, dw->rd_ch_cnt);
		char_sgdma_unmap_user_buf(&cb, cb.write);
		return -ENODEV;
	}
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

	dx_pcie_end_profile(PCIE_TOTAL_TIME_T, count, dw->idx, npu_id, 0);
	return ret;
}
/**
 * dx_sgdma_write - Write user buffer datas to end-point
 * @src: Address of end point
 * @dest: User buffer address
 * @count: Transfer size
 * @dev_id: Device id
 * @dma_ch: DMA Channel id
 * @npu_run: Whether or not to run npu after transmission
 * @type: Where is the location of the source buffer[User/Kernel]
 * @dma_addr: dma address (only for kernel memory type)
 *return - Fail:negative, Pass:transfered size
**/
ssize_t dx_sgdma_write(char *dest, u64 src, size_t count, int dev_id, int dma_ch, bool npu_run, enum mem_type type, dma_addr_t dma_addr)
{
	ssize_t ret = 0;
	struct dw_edma *dw = dx_dev_list_get(dev_id);

	if (!dw) {
		pr_err("[ERR] not found deepx pcie struct for dev_id %d\n", dev_id);
		return -ENODEV;
	}
	if (type == USER_SPACE_BUF) {
		ret = dx_sgdma_write_user(dw, (char __user *)dest, src, count, dma_ch, npu_run);
	} else if (type == KERNEL_SPACE_BUF) {
		ret = dx_sgdma_write_kernel(dw, dest, src, dma_addr, count, dma_ch, npu_run);
	} else {
		pr_err("undefined type[%d]\n", type);
	}

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
 * @type: Where is the location of the source buffer[User/Kernel]
 *return - Fail:negative, Pass:transfered size
**/
ssize_t dx_sgdma_read(char *src, u64 dest, size_t count, int dev_id, int dma_ch, enum mem_type type)
{
	ssize_t ret = 0;
	struct dw_edma *dw = dx_dev_list_get(dev_id);

	if (!dw) {
		pr_err("[ERR] not found deepx pcie struct for dev_id %d\n", dev_id);
		return -ENODEV;
	}
	if(type == USER_SPACE_BUF) {
		ret = dx_sgdma_read_user(dw, (char __user *)src, dest, count, dma_ch);
	} else if (type == KERNEL_SPACE_BUF) {
		pr_err("currently not support kernel space buffer\n");
	} else {
		pr_err("undefined type[%d]\n", type);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(dx_sgdma_read);

/**
 * dx_pcie_reset_dma_channels - Terminate and reset all DMA channels
 * @dev_id: Device id
 *
 * Calls dmaengine_terminate_all on every allocated DMA channel to
 * halt any in-flight transfers, clear pending descriptors and return
 * each channel to the idle state.  Called during error recovery.
 *
 * Return: 0 on success, negative errno on failure.
 */
int dx_pcie_reset_dma_channels(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	int i, ret, err = 0;

	if (!dw) {
		pr_err("[ERR] not found deepx pcie struct for dev_id %d\n", dev_id);
		return -ENODEV;
	}

	pr_info("[%s] resetting DMA channels for dev %d\n", __func__, dev_id);

	/* Guard against use-after-free during concurrent PCI remove */
	if (!atomic_read(&dw->alive)) {
		pr_warn("[%s] dev %d: device removal in progress, skip reset\n",
			__func__, dev_id);
		return -ENODEV;
	}

	mutex_lock(&dw->wr_lock);
	if (dw->ref_count == 0) {
		bool stuck = false;

		for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
			u32 cs = dw_edma_v0_core_ch_status_raw(&dw->chan[i]);
			if (cs == 1 || cs == 2) {
				pr_warn("[%s] dev %d CH%d stuck (CS=%u) with ref_count=0\n",
					__func__, dev_id, i, cs);
				stuck = true;
			}
		}
		if (!stuck) {
			pr_info("[%s] dev %d no active DMA users and no stuck channels, skip\n",
				__func__, dev_id);
			mutex_unlock(&dw->wr_lock);
			return 0;
		}
		pr_warn("[%s] dev %d: no RT service users but channel(s) stuck, proceeding with recovery\n",
			__func__, dev_id);
	}

	/* Bump recovery epoch BEFORE setting hw_err / waking threads.
	 * Transfer threads capture the epoch at start and check it after
	 * waking — if the epoch changed, they bail out immediately
	 * without touching channel state, preventing stale threads from
	 * calling dmaengine_terminate_all() on channels that new
	 * post-recovery transfers may be using. */
	atomic_inc(&dw->recovery_epoch);
	atomic_set(&dw->sbr_in_progress, 1);

	/*
	 * Full memory barrier: ensure recovery_epoch and sbr_in_progress
	 * stores are globally visible before we set hw_err and read
	 * transfer_wq.  Pairs with smp_rmb() in dw_edma_sg_process().
	 * Required on ARM64 where atomic_set is not fully ordered.
	 */
	smp_mb();

	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		struct dw_edma_chan *chan = &dw->chan[i];
		wait_queue_head_t *wq;

		WRITE_ONCE(chan->hw_err, true);
		wq = READ_ONCE(chan->transfer_wq);
		if (wq)
			wake_up_interruptible(wq);
	}

	/* Brief delay to let in-flight DMA threads wake up and
	 * reach the epoch guard (err_stats path).  Without this,
	 * terminate_all below may race with a thread that is still
	 * in the wait_event but has not checked hw_err yet.  The
	 * vc.lock and epoch guard prevent corruption, but draining
	 * first results in a cleaner terminate (most channels will
	 * already be in the expected IDLE or STOP state). */
	msleep(1);
	/*
	 * Terminate all channels with two passes:
	 *   1st terminate_all: sets EDMA_REQ_STOP (or IDLE cleanup)
	 *   2nd terminate_all: if EDMA_REQ_STOP was set and channel is CS=2,
	 *                      triggers ch_soft_reset (engine_en cycle) to
	 *                      clear the HW error state.
	 * Both passes are needed because a single terminate_all only sets
	 * the STOP flag → it does not perform the actual HW reset.
	 */

	/* Write (host→device) channels */
	for (i = 0; i < dw->wr_ch_cnt; i++) {
		if (dw->wr_dma_chan[i]) {
			dmaengine_terminate_all(dw->wr_dma_chan[i]);
			ret = dmaengine_terminate_all(dw->wr_dma_chan[i]);
			if (ret) {
				pr_err("[%s] dev %d wr_ch %d terminate failed (%d)\n",
					__func__, dev_id, i, ret);
				if (!err) err = ret;
			}
		}
	}

	/* Read (device→host) channels */
	for (i = 0; i < dw->rd_ch_cnt; i++) {
		if (dw->rd_dma_chan[i]) {
			dmaengine_terminate_all(dw->rd_dma_chan[i]);
			ret = dmaengine_terminate_all(dw->rd_dma_chan[i]);
			if (ret) {
				pr_err("[%s] dev %d rd_ch %d terminate failed (%d)\n",
					__func__, dev_id, i, ret);
				if (!err) err = ret;
			}
		}
	}

	/*
	 * Safety drain: explicitly flush all vchan descriptor lists for
	 * every channel.  The 2× terminate_all above drains via the
	 * EDMA_REQ_STOP branch, but only if the channel enters that state.
	 * If a channel was already cleaned up to IDLE by its transfer
	 * thread (common with hw_err), terminate_all just clears
	 * configured and skips the drain.  Stale descriptors left in
	 * desc_completed cause the vchan_complete tasklet to fire a
	 * callback into the shared info struct of the NEXT transfer
	 * → "completion busy status=1" error.
	 */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		struct dw_edma_chan *chan = &dw->chan[i];
		struct virt_dma_chan *vc = &chan->vc;
		unsigned long drain_flags;
		LIST_HEAD(drain_head);

		spin_lock_irqsave(&vc->lock, drain_flags);
		list_splice_tail_init(&vc->desc_allocated, &drain_head);
		list_splice_tail_init(&vc->desc_submitted, &drain_head);
		list_splice_tail_init(&vc->desc_issued, &drain_head);
		list_splice_tail_init(&vc->desc_completed, &drain_head);
		chan->request = EDMA_REQ_NONE;
		chan->status = EDMA_ST_IDLE;
		chan->configured = false;
		spin_unlock_irqrestore(&vc->lock, drain_flags);

		if (!list_empty(&drain_head))
			vchan_dma_desc_free_list(vc, &drain_head);
	}

	/*
	 * If any channels are still stuck after terminate_all,
	 * perform PCIe SBR to reset the endpoint.
	 */
	{
		bool needs_sbr = false;

		for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
			u32 cs = dw_edma_v0_core_ch_status_raw(&dw->chan[i]);
			if (cs == 1 || cs == 2) {
				pr_warn("[%s] dev %d CH%d still stuck (CS=%u) after terminate\n",
					__func__, dev_id, i, cs);
				needs_sbr = true;
			}
		}

		if (needs_sbr) {
			int sbr_ret;

			pr_warn("[%s] dev %d: performing PCIe SBR\n",
				__func__, dev_id);
			sbr_ret = dw_edma_v0_core_pcie_reset(dw);
			if (sbr_ret) {
				pr_err("[%s] dev %d: PCIe SBR failed (%d)\n",
					__func__, dev_id, sbr_ret);
				if (!err)
					err = sbr_ret;
			} else {
				/*
				 * SBR succeeded: return 1 to tell the caller
				 * that FW was reset and cannot respond to
				 * messages until re-initialized.
				 */
				err = 1;
			}
		}

		/*
		 * Do NOT clear hw_err / aborted here.
		 * The DMA transfer thread checks hw_err to detect recovery
		 * wakeup.  Clearing it before the thread runs causes a race
		 * where the thread misses the signal and sleeps again until
		 * timeout.  Both flags are reset at the start of the next
		 * transfer (dw_edma_sg_process), which is the safe place.
		 */
	}

	/* Reset channel-in-use tracking so channels can be reused
	 * after recovery.  Without this, ch_in_use stays true forever
	 * and the channel appears permanently busy. */
	for (i = 0; i < dw->rd_ch_cnt; i++) {
		spin_lock(&dw->rd_dma_chan_locks[i].ch_lock);
		dw->rd_dma_chan_locks[i].ch_in_use = false;
		spin_unlock(&dw->rd_dma_chan_locks[i].ch_lock);
	}
	for (i = 0; i < dw->wr_ch_cnt; i++) {
		spin_lock(&dw->wr_dma_chan_locks[i].ch_lock);
		dw->wr_dma_chan_locks[i].ch_in_use = false;
		spin_unlock(&dw->wr_dma_chan_locks[i].ch_lock);
	}

	/* Allow new DMA submissions now that recovery is complete */
	atomic_set(&dw->sbr_in_progress, 0);
	mutex_unlock(&dw->wr_lock);

	pr_info("[%s] dev %d DMA channel reset %s\n",
		__func__, dev_id, err ? "had errors" : "done");
	return err;
}
EXPORT_SYMBOL_GPL(dx_pcie_reset_dma_channels);

/**
 * dx_sgdma_init - PCIe SGDMA initilization
 * @dev_id: Device id
**/
void dx_sgdma_init(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	int i, ret;

	if (!dw) {
		pr_err("[ERR] not found deepx pcie struct for dev_id %d\n", dev_id);
		return;
	}

	if (!atomic_read(&dw->alive)) {
		pr_warn("[%s] dev %d: device removal in progress, skip init\n",
			__func__, dev_id);
		return;
	}

	mutex_lock(&dw->wr_lock);
	if (dw->ref_count == 0) {
		for (i=0; i<dw->rd_ch_cnt; i++) {
			ret = dw_edma_dma_allocation(dw->rd_dma_id, i, &dw->wr_dma_chan[i]);
			if (ret || !dw->wr_dma_chan[i]) {
				pr_err("[%s] dev %d: wr_dma_chan[%d] alloc FAILED (ret=%d)\n",
					__func__, dev_id, i, ret);
			}
		}
		for (i=0; i<dw->wr_ch_cnt; i++) {
			ret = dw_edma_dma_allocation(dw->wr_dma_id, i, &dw->rd_dma_chan[i]);
			if (ret || !dw->rd_dma_chan[i]) {
				pr_err("[%s] dev %d: rd_dma_chan[%d] alloc FAILED (ret=%d)\n",
					__func__, dev_id, i, ret);
			}
		}
	}
	dw->ref_count++;
	pr_debug("[%s] dev %d: ref_count=%d\n", __func__, dev_id, dw->ref_count);
	mutex_unlock(&dw->wr_lock);
}
EXPORT_SYMBOL_GPL(dx_sgdma_init);

/**
 * dx_sgdma_deinit - PCIe SGDMA deinitilization
 * @dev_id: Device id
**/
void dx_sgdma_deinit(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	int i;

	if (!dw) {
		pr_err("[ERR] not found deepx pcie struct for dev_id %d\n", dev_id);
		return;
	}

	/* Guard against use-after-free: if PCI remove is in progress
	 * (or already completed), dw memory may be freed imminently.
	 * Do not touch dw->wr_lock in that case. */
	if (!atomic_read(&dw->alive)) {
		pr_warn("[%s] dev %d: device removal in progress, skip deinit\n",
			__func__, dev_id);
		return;
	}

	mutex_lock(&dw->wr_lock);
	pr_debug("[%s] dev %d: ref_count=%d (before decrement)\n",
		__func__, dev_id, dw->ref_count);
	if (dw->ref_count == 1) {
		for (i=0; i<dw->wr_ch_cnt; i++) {
			dw_edma_dma_deallocation(&dw->wr_dma_chan[i]);
		}
		for (i=0; i<dw->rd_ch_cnt; i++) {
			dw_edma_dma_deallocation(&dw->rd_dma_chan[i]);
		}
	}
	dw->ref_count--;
	mutex_unlock(&dw->wr_lock);
}
EXPORT_SYMBOL_GPL(dx_sgdma_deinit);

MODULE_LICENSE("GPL");
