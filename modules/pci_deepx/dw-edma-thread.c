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
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/wait.h>

#include "dw-edma-core.h"
#include "dw-edma-v0-core.h"

#include "dw-edma-thread.h"
#include "dx_util.h"
#include "dx_sgdma_cdev.h"
#include "dx_lib.h"
#include "dx_message.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/task.h>
#endif

#define EDMA_CHANNEL_NAME			"dma%uchan%u"

static u32 timeout = 3000;
module_param(timeout, uint, 0644);
MODULE_PARM_DESC(timeout, "Transfer timeout in msec");

static struct dw_edma_info test_info[MAX_DEV_NUM][EDMA_MAX_WR_CH][EDMA_CH_END];

/* Global mutex to prevent concurrent channel allocation */
static DEFINE_MUTEX(dma_allocation_mutex);

static void dw_edma_callback(void *arg)
{
	struct dw_edma_info *info = arg;
	dbg_tfr("[%s]\n", __func__);
	dx_pcie_end_profile(PCIE_ISR_EXEC_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

	dx_pcie_start_profile(PCIE_WAKEUP_LATENCY_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	info->dma_done.done = true;
	wake_up(info->dma_done.wait);
}

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
	int orig_nents = 0;
	enum dma_transfer_direction	direction = !info->cb->write ? DMA_DEV_TO_MEM : DMA_MEM_TO_DEV;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	int nice = task_nice(current);
	int policy = current->policy;
#endif
#ifdef DMA_PERF_MEASURE
	ktime_t dma_trans_t;
#endif
	long ret;
	int start_epoch;

	dbg_tfr("[%s] Start!!\n", __func__);

#ifdef DMA_PERF_MEASURE
	get_start_time(&dma_trans_t);
#endif

	if (chan == NULL) {
		pr_err("[%s] DMA channel Null point error! (dev_n=%d, npu_id=%d, write=%d)\n",
			__func__, info->dev_n,
			info->cb ? info->cb->npu_id : -1,
			info->cb ? info->cb->write : -1);
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

	/* Set SG Table */
	sgt = &(cb->sgt);
	sg = &cb->sgt.sgl[0];

	/*
	 * Configures DMA channel according to the direction
	 *  - flags
	 *  - source and destination addresses
	 */
	dx_pcie_start_profile(PCIE_DMA_MAP_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	/* Save original nents before dma_map_sg may coalesce entries.
	 * dma_unmap_sg must receive the same nents passed to dma_map_sg
	 * (i.e., the original count), otherwise cache maintenance
	 * (invalidation for DMA_FROM_DEVICE) will be incomplete,
	 * causing stale L1 cache lines (64-byte corruption on ARM). */
	orig_nents = sgt->nents;
	if (cb->pre_mapped) {
		/* Buffer already mapped (e.g. dma_alloc_coherent kernel buf).
		 * sg_dma_address/sg_dma_len are already set - skip dma_map_sg
		 * to avoid double-mapping and address overwrite. */
		dbg_tfr("%s: pre_mapped buffer, skip dma_map_sg\n",
			dma_chan_name(chan));
		if (direction == DMA_DEV_TO_MEM) {
			sconf.src_addr = cb->ep_addr;
			sconf.dst_addr = sg_dma_address(sg);
		} else {
			sconf.src_addr = sg_dma_address(sg);
			sconf.dst_addr = cb->ep_addr;
		}
		dw_chan->set_desc = true;
	} else if (direction == DMA_DEV_TO_MEM) {
		/* DMA_DEV_TO_MEM - WRITE - DMA_FROM_DEVICE */
		dbg_tfr("%s: DMA_DEV_TO_MEM - WRITE - DMA_FROM_DEVICE\n",
			dma_chan_name(chan));
		sgt->nents = dma_map_sg(dev, sgt->sgl, orig_nents, DMA_FROM_DEVICE);
		if (!sgt->nents) {
			pr_err("sg table mapped fail(DMA_FROM_DEVICE)\n");
			goto err_alloc_descs;
		}
		/* Endpoint memory */
		sconf.src_addr = cb->ep_addr;
		/* CPU memory */
		sconf.dst_addr = sg_dma_address(sg);
		dw_chan->set_desc = true;
	} else {
		/* DMA_MEM_TO_DEV - READ - DMA_TO_DEVICE */
		dbg_tfr("%s: DMA_MEM_TO_DEV - READ - DMA_TO_DEVICE\n",
			dma_chan_name(chan));
		sgt->nents = dma_map_sg(dev, sgt->sgl, orig_nents, DMA_TO_DEVICE);
		if (!sgt->nents) {
			pr_err("sg table mapped fail(DMA_TO_DEVICE)\n");
			goto err_alloc_descs;
		}

		/* CPU memory */
		sconf.src_addr = sg_dma_address(sg);
		/* Endpoint memory */
		sconf.dst_addr = cb->ep_addr;
		dw_chan->set_desc = true;
	}
	dx_pcie_end_profile(PCIE_DMA_MAP_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	if (sgt->nents == 1) {
		cb->is_llm = false;
	}
	dw_chan->is_llm = cb->is_llm;

	sconf.direction = DMA_TRANS_NONE; /* remote DMA (Device <-> Host Memory) */

	dmaengine_slave_config(chan, &sconf);
	dbg_tfr("%s: addr(physical) src=%pa, dst=%pa\n",
		dma_chan_name(chan), &sconf.src_addr, &sconf.dst_addr);

	/*
	 * Prepare the DMA channel for the transfer
	 *  - provide scatter-gather list
	 *  - configure to trigger an interrupt after the transfer
	 */
	dx_pcie_start_profile(PCIE_DMA_PREP_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
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
	dw_chan->hw_err = false;
	dw_chan->aborted = false;

	/* Capture recovery epoch — used after every wait to detect if
	 * a recovery happened during this transfer.  If the epoch changed,
	 * the channel has already been cleaned up by recovery and we must
	 * exit immediately without calling dmaengine_terminate_all() to
	 * avoid corrupting new post-recovery transfers on this channel. */
	start_epoch = atomic_read(&dw_chan->chip->dw->recovery_epoch);

	txdesc->callback = dw_edma_callback;
	txdesc->callback_param = info;
	cookie = dmaengine_submit(txdesc);
	if (dma_submit_error(cookie)) {
		dev_err(dev, "%s: dma_submit_error\n", dma_chan_name(chan));
		f_sbt_cnt++;
		goto err_stats;
	}
	dx_pcie_end_profile(PCIE_DMA_PREP_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

#ifdef DMA_PERF_MEASURE
	/* send pointer to measure a performace */
	dw_chan->chip->dw->irq[0].data[info->cb->npu_id][info->cb->write] = info;
#endif

	/* Publish waitqueue so recovery (dx_pcie_reset_dma_channels) can
	 * wake this thread immediately instead of waiting for timeout. */
	WRITE_ONCE(dw_chan->transfer_wq, &info->done_wait);

	/* ── Admission gate ──
	 * Block new DMA submissions while recovery/SBR is in progress.
	 * Without this gate, a concurrent process can ring the doorbell
	 * during SBR, causing the transfer to be silently lost (the HW
	 * is being reset).  Also re-check recovery_epoch in case recovery
	 * started between our epoch capture and this point.
	 *
	 * smp_rmb() pairs with smp_mb() in dx_pcie_reset_dma_channels()
	 * to ensure we see the latest sbr_in_progress/recovery_epoch
	 * stores on ARM64. */
	smp_rmb();
	if (atomic_read(&dw_chan->chip->dw->sbr_in_progress) ||
	    atomic_read(&dw_chan->chip->dw->recovery_epoch) != start_epoch) {
		dev_warn(dev, "%s: recovery/SBR in progress, rejecting transfer\n",
			 dma_chan_name(chan));
		WRITE_ONCE(dw_chan->transfer_wq, NULL);
		cb->result = -EIO;
		goto err_stats;
	}

	/* Start DMA transfer - trigger a doorbell of dma */
	dx_pcie_start_profile(PCIE_DMA_XFER_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	dma_async_issue_pending(chan);


	/* Thread waits here for transfer completion or exists by timeout */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	if (policy == SCHED_NORMAL)
		sched_set_fifo(current);
#endif

	ret = wait_event_interruptible_timeout(info->done_wait,
		done->done || dw_chan->hw_err,
		msecs_to_jiffies(timeout));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	if (policy == SCHED_NORMAL)
		sched_set_normal(current, nice);
#endif
	dx_pcie_end_profile(PCIE_WAKEUP_LATENCY_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

	/* ── Recovery guard ──
	 * If a recovery happened while we were waiting, the channel was
	 * already cleaned up by dx_pcie_reset_dma_channels().  Exit
	 * immediately — calling dmaengine_terminate_all() would corrupt
	 * new post-recovery transfers (set EDMA_REQ_STOP or clear
	 * configured on a channel the new transfer is using). */
	if (atomic_read(&dw_chan->chip->dw->recovery_epoch) != start_epoch) {
		cb->result = -EIO;
		goto err_stats;
	}

	/* Check DMA transfer status and act upon it  */
	status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
	if (dw_chan->hw_err) {
		dev_err(dev, "%s: channel in irrecoverable HW error (CS=2), failing transfer\n",
			dma_chan_name(chan));
		/*
		 * Two terminate_all calls needed (same as abort path):
		 *   1st: sets EDMA_REQ_STOP (channel was BUSY)
		 *   2nd: enters EDMA_REQ_STOP branch â drains desc_completed
		 *         and all other descriptor lists.
		 * Without the 2nd call, the stale vchan_complete tasklet from
		 * the original transfer fires into the shared info struct,
		 * setting done->done=true for the NEXT transfer â that transfer
		 * sees done->done=true but its cookie was never completed â
		 * "completion busy status=1" error.
		 */
		dmaengine_terminate_all(chan);  /* 1st: EDMA_REQ_STOP */
		dmaengine_terminate_all(chan);  /* 2nd: drain descriptors */
		cb->result = -EIO;
		/* No event here — hw_err is set by recovery path which
		 * already handles user-space coordination. */
	} else if (ret == -ERESTARTSYS) {
		dev_info(dev, "%s: signal received, requesting graceful DMA stop\n",
			 dma_chan_name(chan));
		dmaengine_terminate_all(chan);

		/* Re-wait (non-interruptible) for the current chunk to finish */
		ret = wait_event_timeout(info->done_wait,
			done->done || dw_chan->hw_err,
			msecs_to_jiffies(timeout));

		/* Recovery guard for re-wait */
		if (atomic_read(&dw_chan->chip->dw->recovery_epoch) != start_epoch) {
			cb->result = -EIO;
			goto err_stats;
		}

		if (ret == 0 && !done->done) {
			dev_warn(dev, "%s: graceful stop timed out, forcing cleanup\n",
				 dma_chan_name(chan));
			/* Second terminate_all call → force cleanup path */
			dmaengine_terminate_all(chan);
			cb->result = -ETIMEDOUT;
		} else {
			dev_info(dev, "%s: DMA stopped gracefully\n",
				 dma_chan_name(chan));
			cb->result = 0;
		}
	} else if (!done->done) {
		u32 cs = dw_edma_v0_core_ch_status_raw(dw_chan);
		f_tm_cnt++;

		if (cs == 2) {
			/*
			 * CS=2: PCIe error stopped the channel.  Abort MSI
			 * never arrived (HW limitation).
			 * ch_soft_reset (called by 2nd terminate_all) performs
			 * engine_en=0->1 to clear HW CS=2 state.
			 *
			 * NOTE: engine_en cycle resets ALL channels on this
			 * direction.  CS bits [6:5] are read-only HW status;
			 * there is no per-channel-only HW recovery path.
			 */
			dev_warn(dev, "%s: timeout (CS=2), attempting soft reset\n",
				 dma_chan_name(chan));
			dmaengine_terminate_all(chan);  /* 1st: EDMA_REQ_STOP */
			dmaengine_terminate_all(chan);  /* 2nd: ch_soft_reset -> IDLE */

			/* Verify HW actually recovered */
			cs = dw_edma_v0_core_ch_status_raw(dw_chan);
			if (cs == 0 || cs == 3) {
				dev_info(dev, "%s: channel recovered (CS=%u)\n",
					 dma_chan_name(chan), cs);
				cb->result = -EIO;
				/* Channel is usable again - no user-space event */
			} else {
				dev_err(dev, "%s: soft reset FAILED (CS=%u), user-space recovery needed\n",
					dma_chan_name(chan), cs);
				cb->result = -EIO;
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
				{
					struct dw_edma *dw = dw_chan->chip->dw;
					dx_pcie_enqueue_event_response(dw->idx,
						ERR_PCIE_DMA_CH_FAIL(dw_chan->id));
				}
#endif
			}
		} else {
			/*
			 * CS!=2: channel stuck or in unknown state.
			 * Only one terminate_all (EDMA_REQ_STOP) here;
			 * full recovery deferred to user-space.
			 */
			{
				struct dw_edma *dw = dw_chan->chip->dw;
				u32 done_sts = dw_edma_v0_core_status_done_int(
					dw, dw_chan->dir);
				u32 abort_sts = dw_edma_v0_core_status_abort_int(
					dw, dw_chan->dir);

				dev_err(dev, "%s: timeout (CS=%u) "
					"done_int=0x%x abort_int=0x%x\n",
					dma_chan_name(chan), cs,
					done_sts, abort_sts);
			}
			dmaengine_terminate_all(chan);
			cb->result = -ETIMEDOUT;
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
			{
				struct dw_edma *dw = dw_chan->chip->dw;
				dx_pcie_enqueue_event_response(dw->idx,
					ERR_PCIE_DMA_CH_FAIL(dw_chan->id));
			}
#endif
		}
	} else if (dw_chan->aborted) {
		/*
		 * Abort ISR completed the descriptor (done->done = true)
		 * but the transfer data is incomplete/corrupt.
		 * Abort ISR already sent enriched event to user-space
		 * (dx_pcie_enqueue_abort_event), so no duplicate here.
		 *
		 * Two terminate_all calls needed:
		 *   1st: sets EDMA_REQ_STOP
		 *   2nd: sees EDMA_REQ_STOP + CS=2, triggers ch_soft_reset
		 *         (engine_en cycle) to clear the HW error state.
		 * Without the 2nd call, channel stays in CS=2 permanently
		 * and all subsequent transfers fail with "skipping doorbell".
		 */
		dev_err(dev, "%s: transfer aborted by HW (CS=2)\n",
			dma_chan_name(chan));
		dw_chan->aborted = false;
		dmaengine_terminate_all(chan);  /* 1st: EDMA_REQ_STOP */
		dmaengine_terminate_all(chan);  /* 2nd: ch_soft_reset */
		cb->result = -EIO;
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

	/* Unmap scatter gather mapping.
	 * Use orig_nents (the count passed to dma_map_sg), not the
	 * coalesced sgt->nents returned by dma_map_sg, to ensure
	 * every original page receives proper cache maintenance.
	 * Skip unmap for pre-mapped buffers (dma_alloc_coherent). */
	dx_pcie_start_profile(PCIE_POST_PROCESS_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
	if (!cb->pre_mapped && orig_nents > 0) {
		if (direction == DMA_DEV_TO_MEM) {
			dma_unmap_sg(dev, sgt->sgl, orig_nents, DMA_FROM_DEVICE);
		} else {
			dma_unmap_sg(dev, sgt->sgl, orig_nents, DMA_TO_DEVICE);
		}
	}
	dx_pcie_end_profile(PCIE_POST_PROCESS_T, cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

err_alloc_descs:
	WRITE_ONCE(dw_chan->transfer_wq, NULL);	/* Unpublish waitqueue */
	sg_free_table(sgt);
	info->done = true; 

	return cb->result; // Return 0 on success, negative error code on failure
}

/* Optimized channel lookup with device ID pre-check */
static bool dw_edma_ch_filter_optimized(struct dma_chan *chan, void *filter_data)
{
	struct {
		int expected_dev_id;
		int expected_npu_id;
		char expected_name[32];
	} *filter_info = filter_data;

	/* Quick device ID check first */
	if (chan->device->dev_id != filter_info->expected_dev_id) {
		return false;
	}

	/* Then name comparison */
	if (strcmp(dma_chan_name(chan), filter_info->expected_name) == 0) {
		dbg_tfr("DMA channel match found: %s (dev_id: %d)\n", 
			dma_chan_name(chan), chan->device->dev_id);
		return true;
	}
	return false;
}

int dw_edma_dma_allocation(int dma_ch_id, int npu_id, struct dma_chan **_chan)
{
	dma_cap_mask_t mask;
	struct {
		int expected_dev_id;
		int expected_npu_id;
		char expected_name[32];
	} filter_info;
	int ret = 0;

#ifndef CONFIG_CMA_SIZE_MBYTES
	dbg_tfr("CMA not present/activated! Contiguous Memory may fail to be allocated\n");
#endif
	/* Validate NPU ID range - only 0~3 are supported */
	if (npu_id >= 4) {
		pr_err("Invalid NPU ID: %d (only 0~3 supported)\n", npu_id);
		return -EINVAL;
	}

	/* Protect concurrent allocation attempts with mutex */
	mutex_lock(&dma_allocation_mutex);

	/* Search dma channel */
	dbg_tfr("[DMA_ALLOC] dma%dchan%d allocation request\n", dma_ch_id, npu_id);

	if (!(*_chan)) {
		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);      /*Scatter Gather Mode*/
		dma_cap_set(DMA_CYCLIC, mask);     /*Cyclic Mode*/
		dma_cap_set(DMA_PRIVATE, mask);
		dma_cap_set(DMA_INTERLEAVE, mask);

		/* Try optimized filter first */
		filter_info.expected_dev_id = dma_ch_id;
		filter_info.expected_npu_id = npu_id;
		snprintf(filter_info.expected_name, sizeof(filter_info.expected_name),
			EDMA_CHANNEL_NAME, dma_ch_id, npu_id);

		(*_chan) = dma_request_channel(mask, dw_edma_ch_filter_optimized,
					&filter_info);
		if (*_chan) {
			dbg_tfr("[DMA_ALLOC] SUCCESS: dma%dchan%d allocated\n", dma_ch_id, npu_id);
		} else {
			pr_err("[DMA_ALLOC] FAILED: dma%dchan%d not found\n", dma_ch_id, npu_id);
			ret = -ENODEV;
		}
	}

	mutex_unlock(&dma_allocation_mutex);
	return ret;
}

void dw_edma_dma_deallocation(struct dma_chan **_chan)
{
	mutex_lock(&dma_allocation_mutex);

	if ((*_chan) != NULL && (*_chan)->client_count > 0) {
		dma_release_channel((*_chan));
		(*_chan) = NULL;
		dbg_tfr("DMA channel released\n");
	}

	mutex_unlock(&dma_allocation_mutex);
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
		dx_pcie_start_profile(PCIE_KERNEL_DMA_TOTAL_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);
		ret = dw_edma_sg_process(info, dma_ch);
		dx_pcie_end_profile(PCIE_KERNEL_DMA_TOTAL_T, info->cb->len, info->dev_n, info->cb->npu_id, info->cb->write);

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
			mutex_unlock(&info->lock);
		}
	}
}
