// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef __DW_EDMA_THREAD_H__
#define __DW_EDMA_THREAD_H__

#include <linux/compiler.h>

#define MAX_DEV_NUM		16

enum channel_id {
	EDMA_CH_WR = 0,
	EDMA_CH_RD,
	EDMA_CH_END
};

struct dw_edma_done {
	bool					done;
	wait_queue_head_t		*wait;
};

struct dw_edma_info {
	struct list_head		channels;
	struct mutex			lock;
	bool					init;
	struct dx_dma_io_cb		*cb;
	int						dev_n;
	struct dma_chan 		*dma_chan;
	ssize_t 				host_buf_addr;
	ssize_t 				ep_buf_addr;
	ssize_t					size;
	bool run_test;
	wait_queue_head_t		done_wait;
	struct dw_edma_done		dma_done;
	bool					done;
};

extern int dw_edma_run(struct dx_dma_io_cb * cb, struct dma_chan *dma_ch, int dev_n, int ch);
extern int dw_edma_dma_allocation(int dma_ch_id, int npu_id, struct dma_chan **_chan);
extern void dw_edma_dma_deallocation(struct dma_chan **_chan);

extern int dw_edma_thread_init(int dev_n);
extern void dw_edma_thread_exit(int dev_n);
extern void dw_edma_thread_probe(void);

#endif /* __DW_EDMA_THREAD_H__ */
