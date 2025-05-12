// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/wait.h>

#include "dw-edma-core.h"
#include <linux/times.h>
#include <linux/time.h>

#include "dw-edma-thread.h"
#include "dx_util.h"
#include "dx_sgdma_cdev.h"
#include "dx_lib.h"

#include <linux/compiler.h>
#include <linux/ratelimit.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/task.h>
#endif

// #define DMA_S_POLLING_MODE

#define EDMA_TEST_MAX_THREADS_CHANNEL		8
#define EDMA_CHANNEL_NAME			"dma%uchan%u"

static u32 timeout = 3000;
module_param(timeout, uint, 0644);
MODULE_PARM_DESC(timeout, "Transfer timeout in msec");

static struct dw_edma_info test_info[MAX_DEV_NUM][EDMA_MAX_WR_CH][EDMA_CH_END];

static void dw_edma_callback(void *arg)
{
	struct dw_edma_info *info = arg;
	dbg_tfr("[%s]\n", __func__);
	dx_pcie_end_profile(PCIE_INT_CB_CALL_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

	dx_pcie_start_profile(PCIE_CB_TO_WAKE_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	info->dma_done.done = true;
	wake_up_interruptible(info->dma_done.wait);
}
#if 1
static bool dw_edma_check_desc_table_set(struct dw_edma_info *info, ssize_t host_addr, ssize_t ep_addr, ssize_t size)
{
	bool ret = true;
	if ((info->host_buf_addr == host_addr) &&
		(info->ep_buf_addr == ep_addr) &&
		(info->size == size) &&
		(size < 174756*1024)) { /* TODO - remove hard fixed condition*/
		ret = false;
	}
	return ret;
}
#endif

static int dw_edma_sg_process(struct dw_edma_info *info,
				    struct dma_chan *chan)
{
	struct dma_async_tx_descriptor *txdesc;
	struct dw_edma_done *done;
	struct dma_slave_config	sconf;
	enum dma_status status;
	struct scatterlist *sg;
	struct sg_table	*sgt;
	dma_cookie_t cookie;
	struct device *dev;
	u32 f_prp_cnt = 0;
	u32 f_sbt_cnt = 0;
	u32 f_cpl_err = 0;
	u32 f_cpl_bsy = 0;
	u32 f_tm_cnt = 0;
	struct dx_dma_io_cb *cb;
	struct dw_edma_chan *dw_chan;
	enum dma_transfer_direction	direction = !info->cb->write ? DMA_DEV_TO_MEM : DMA_MEM_TO_DEV;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	int nice = task_nice(current);
	int policy = current->policy;
#endif
#ifdef DMA_PERF_MEASURE
	ktime_t dma_trans_t;
#endif

	dbg_tfr("[%s] Start!!\n", __func__);

#ifdef DMA_PERF_MEASURE
	get_start_time(&dma_trans_t);
#endif

	if (chan == NULL) {
		pr_err("[%s] DMA channel Null point error!", __func__);
		return -1;
	}

	info->done = false;
	info->dma_done.wait = &info->done_wait;
	init_waitqueue_head(&info->done_wait);

	done = &info->dma_done;
	dev = chan->device->dev;
	cb = info->cb;
	dw_chan = dchan2dw_edma_chan(chan);


	if (info->cb->npu_run) {
		dw_chan->en_lie = true;
	} else {
		dw_chan->en_lie = false;
	}
	dbg_tfr("%s: local interrupt is %s\n",
			dma_chan_name(chan), 
			dw_chan->en_lie ? "enable" : "disable");

	// pr_err("[DMA] task nice:%d, cpu:%d, policy:%d, prio:%d\n",
	// 	task_nice(current), task_cpu(current), current->policy, current->prio);

	/* Set SG Table */
	sgt = &(cb->sgt);
	sg = &cb->sgt.sgl[0];

	/*
	 * Configures DMA channel according to the direction
	 *  - flags
	 *  - source and destination addresses
	 */
	if (direction == DMA_DEV_TO_MEM) {
		/* DMA_DEV_TO_MEM - WRITE - DMA_FROM_DEVICE */
		dbg_tfr("%s: DMA_DEV_TO_MEM - WRITE - DMA_FROM_DEVICE\n",
			dma_chan_name(chan));
		sgt->nents = dma_map_sg(dev, sgt->sgl, sgt->nents, DMA_FROM_DEVICE);
		if (!sgt->nents) {
			pr_err("sg table mapped fail(DMA_FROM_DEVICE)\n");
			goto err_alloc_descs;
		}
		/* Endpoint memory */
		sconf.src_addr = cb->ep_addr;
		/* CPU memory */
		sconf.dst_addr = sg_dma_address(sg);
#ifdef SRAM_DESC_TABLE
		dw_chan->set_desc = true;
#else
		dw_chan->set_desc = dw_edma_check_desc_table_set(info, sconf.dst_addr, sconf.src_addr, cb->len);
#endif
	} else {
		/* DMA_MEM_TO_DEV - READ - DMA_TO_DEVICE */
		dbg_tfr("%s: DMA_MEM_TO_DEV - READ - DMA_TO_DEVICE\n",
			dma_chan_name(chan));
		sgt->nents = dma_map_sg(dev, sgt->sgl, sgt->nents, DMA_TO_DEVICE);
		if (!sgt->nents) {
			pr_err("sg table mapped fail(DMA_TO_DEVICE)\n");
			goto err_alloc_descs;
		}

		/* CPU memory */
		sconf.src_addr = sg_dma_address(sg);
		/* Endpoint memory */
		sconf.dst_addr = cb->ep_addr;
		// sconf.dst_addr = dt_region->paddr;
#ifdef SRAM_DESC_TABLE
		dw_chan->set_desc = true;
#else
		dw_chan->set_desc = dw_edma_check_desc_table_set(info, sconf.src_addr, sconf.dst_addr, cb->len);
#endif
	}
	if (sgt->nents == 1) {
		cb->is_llm = false;
	}
	dw_chan->is_llm = cb->is_llm;
	// pr_info("DMA mapped: sg[dir:%d] -> phys: %llx, dma: %llx\n",
	// 	direction, (unsigned long long)sg_phys(sg), (unsigned long long)sg_dma_address(sg));

	sconf.direction = DMA_TRANS_NONE; /* remote DMA (Device <-> Host Memory) */

	dmaengine_slave_config(chan, &sconf);
	dbg_tfr("%s: addr(physical) src=%pa, dst=%pa\n",
		dma_chan_name(chan), &sconf.src_addr, &sconf.dst_addr);

	/*
	 * Prepare the DMA channel for the transfer
	 *  - provide scatter-gather list
	 *  - configure to trigger an interrupt after the transfer
	 */
	txdesc = dmaengine_prep_slave_sg(chan, sgt->sgl, sgt->nents,
					 direction,
					 DMA_PREP_INTERRUPT);
	if (!txdesc) {
		dev_err(dev, "%s: dmaengine_prep_slave_sg\n",
			dma_chan_name(chan));
		f_prp_cnt++;
		goto err_stats;
	}

	done->done = false;
	txdesc->callback = dw_edma_callback;
	// txdesc->callback_param = done;
	txdesc->callback_param = info;
	cookie = dmaengine_submit(txdesc);
	if (dma_submit_error(cookie)) {
		dev_err(dev, "%s: dma_submit_error\n", dma_chan_name(chan));
		f_sbt_cnt++;
		goto err_stats;
	}

#ifdef DMA_PERF_MEASURE
	/* send pointer to measure a performace */
	dw_chan->chip->dw->irq[0].data[info->cb->npu_id][info->cb->write] = info;
#endif

	/* Start DMA transfer - trigger a doorbell of dma */
	dma_async_issue_pending(chan);

	dx_pcie_start_profile(PCIE_DATA_BW_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

	/* Thread waits here for transfer completion or exists by timeout */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	if (policy == SCHED_NORMAL)
		sched_set_fifo(current);
#endif

	wait_event_interruptible_timeout(info->done_wait,
		done->done,
		msecs_to_jiffies(timeout));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	if (policy == SCHED_NORMAL)
		sched_set_normal(current, nice);
#endif
	dx_pcie_end_profile(PCIE_CB_TO_WAKE_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

#ifdef DMA_PERF_MEASURE
	dev_err(dev, "[PERF] DMA Transer Elapsed Time(%s) @ %lld ns\n",
		(direction == DMA_DEV_TO_MEM) ? "WRITE" : "READ",
		get_elapsed_time_ns(dma_trans_t));
#endif

	/* Check DMA transfer status and act upon it  */
	status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
	if (!done->done) {
		f_tm_cnt++;
		dmaengine_terminate_all(chan);
		cb->result = -ETIMEDOUT;
	} else if (status != DMA_COMPLETE) {
		if (status == DMA_ERROR) {
			f_cpl_err++;
		} else {
			f_cpl_bsy++;
		}
		dmaengine_terminate_all(chan);
		cb->result = -EIO;
	} else {
		cb->result = 0;
	}

err_stats:
	if (f_prp_cnt || f_sbt_cnt || f_tm_cnt || f_cpl_err || f_cpl_bsy) {
		dev_err(dev, "%s: failed - dmaengine_prep_slave_sg=%u, dma_submit_error=%u, timeout=%u, completion error status=%u, completion busy status=%u\n",
			dma_chan_name(chan), f_prp_cnt, f_sbt_cnt,
			f_tm_cnt, f_cpl_err, f_cpl_bsy);
		if (cb->result == 0) cb->result = -EIO;
	}

	/* Unmap scatter gatter mapping */
	if (sgt->nents > 0) {
		if (direction == DMA_DEV_TO_MEM) {
			dma_unmap_sg(dev, sgt->sgl, sgt->nents, DMA_FROM_DEVICE);
		} else {
			dma_unmap_sg(dev, sgt->sgl, sgt->nents, DMA_TO_DEVICE);
		}
	}

	/* TODO - need a recovery code in case of dma errors. */

err_alloc_descs:
	sg_free_table(sgt);
	info->done = true; 

	return cb->result; // Return 0 on success, negative error code on failure
}

static bool dw_edma_ch_filter(struct dma_chan *chan, void *filter)
{
	dbg_tfr("Fitler for DMA Channel ID:%d(filter:%s, name:%s)\n",
		chan->device->dev_id, (char *)filter, dma_chan_name(chan));
	if (strcmp(dma_chan_name(chan), filter))
		return false;
	return true;
}

int dw_edma_dma_allocation(int dma_ch_id, int npu_id, struct dma_chan **_chan)
{
	dma_cap_mask_t mask;
	char filter[20];

#ifndef CONFIG_CMA_SIZE_MBYTES
	dbg_tfr("CMA not present/activated! Contiguous Memory may fail to be allocated\n");
#endif

	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);      /*Scatter Gather Mode*/
	dma_cap_set(DMA_CYCLIC, mask);     /*Cyclic Mode*/
	dma_cap_set(DMA_PRIVATE, mask);
	dma_cap_set(DMA_INTERLEAVE, mask);

	/* Search dma channel */
	dbg_tfr("chan pointer : 0x%p\n", (*_chan));
	if (!(*_chan)) {
		snprintf(filter, sizeof(filter),
			EDMA_CHANNEL_NAME, dma_ch_id, npu_id);
		(*_chan) = dma_request_channel(mask, dw_edma_ch_filter,
					filter);
		if (!(*_chan)) {
			return -ENODEV;
		}
		dbg_tfr("[DMA] Request dma channel!! (chan:%p, id:%d, name:%s)\n",
				(*_chan), (*_chan)->device->dev_id, dma_chan_name((*_chan)));
	} else {
		dbg_tfr("[DMA] Dma channel is already allocated (chan:%p, name:%s)\n",
			(*_chan), dma_chan_name((*_chan)));
	}
	return 0;
}

void dw_edma_dma_deallocation(struct dma_chan **_chan)
{
	if ((*_chan) != NULL && (*_chan)->client_count > 0) {
		dbg_tfr("[DMA] Release dma channel!! (chan:%p, name:%s, ref_c:%d)\n",
			(*_chan), dma_chan_name((*_chan)), (*_chan)->client_count);
		dma_release_channel((*_chan));
		(*_chan) = NULL;
	}
}

int dw_edma_run(struct dx_dma_io_cb * cb, struct dma_chan *dma_ch, int dev_n, int ch)
{
	struct dw_edma_info *info = &test_info[dev_n][cb->npu_id][ch];
	int ret = 0;

	mutex_lock(&info->lock);
	if (!info->done) {
		ret = -EBUSY;
		pr_err("DMA is running (BUSY:dev#%d, npu#%d, ch:%d) : %d\n", dev_n, cb->npu_id, ch, ret);
	} else {
		dbg_tfr("DMA is Ready to tranfser datas (dev#%d, npu#%d, ch:%d)\n", dev_n, cb->npu_id, ch);
		info->cb = cb;
		dx_pcie_start_profile(PCIE_THREAD_RUN_T, 0, info->dev_n, info->cb->npu_id, info->cb->write);
		ret = dw_edma_sg_process(info, dma_ch);
		dx_pcie_end_profile(PCIE_THREAD_RUN_T, 0, info->dev_n, info->cb->npu_id, info->cb->write);

	}
	mutex_unlock(&info->lock);

	return ret;
}

int dw_edma_thread_init(int dev_n)
{
	struct dw_edma_info *info;
	int i, j;

	for (i = 0; i < EDMA_MAX_WR_CH; i++) {
		for (j = 0; j < EDMA_CH_END; j++) {
			info = &test_info[dev_n][i][j];
			dbg_init("Thread Init for dev#%d npu#%d [info:%p]\n", dev_n, i, info);
			info->init = true;
			info->dev_n = dev_n;
			info->done = true;
		}
	}
	clear_pcie_profile_info(0, 0, 0, 0, 0);

	return 0;
}

void dw_edma_thread_probe(void)
{
	int i, j, k;
	for (i = 0; i < MAX_DEV_NUM; i++) {
		for (j = 0; j < EDMA_MAX_WR_CH; j++) {
			for (k = 0; k < EDMA_CH_END; k++) {
				INIT_LIST_HEAD(&test_info[i][j][k].channels);
				mutex_init(&test_info[i][j][k].lock);
			}
		}
	}
}

// static void dw_edma_del_channel(struct dw_edma_thread_chan *tchan)
// {
// 	struct dw_edma_thread *thread = tchan->thread;
// 	dbg_tfr("thread %s exited\n", thread->task->comm);
// 	kvfree(thread);
// 	tchan->thread = NULL;
// 	dmaengine_terminate_all(tchan->chan);
// 	kvfree(tchan);
// }

void dw_edma_thread_exit(int dev_n)
{
	struct dw_edma_info *info;
	int i, j;

	for (i = 0; i < EDMA_MAX_WR_CH; i++) {
		for (j = 0; j < EDMA_CH_END; j++) {
			info = &test_info[dev_n][i][j];
			mutex_lock(&info->lock);
			dbg_init("Thread Exit for dev#%d npu#%d [info:%p]\n", dev_n, i, info);
			info->init = false;
			/* TODO - A defense code is required so that the DMA can be forcibly terminated in case the DMA is not terminated.*/
			// dw_edma_thread_stop(info);
			mutex_unlock(&info->lock);
		}
	}
}
