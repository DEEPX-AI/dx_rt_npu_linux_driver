// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef _DW_EDMA_CORE_H
#define _DW_EDMA_CORE_H

#include <linux/msi.h>
#include <linux/pci.h>

#include "dx_edma.h"
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
#include "dx_message.h"
#endif

//#include "../virt-dma.h"	//DEEPX MODIFIED
#include "virt-dma.h"		//DEEPX MODIFIED
#include "dx_cdev.h"

#define EDMA_LL_SZ						24
#define EDMA_MAX_WR_CH					8
#define EDMA_MAX_RD_CH					8

#define USER_NUM_MAX					4
#define USER_IRQ_NUMS					16

// #define SRAM_DESC_TABLE

enum dw_edma_dir {
	EDMA_DIR_WRITE = 0,
	EDMA_DIR_READ
};

enum dw_edma_engine {
	EDMA_ENG_DIS = 0,
	EDMA_ENG_EN
};

enum dx_edma_map_format {
	DX_DMA_MF_EDMA_LEGACY = 0x0,
	DX_DMA_MF_EDMA_UNROLL = 0x1,
	DX_DMA_MF_HDMA_COMPAT = 0x5
};

enum dw_edma_request {
	EDMA_REQ_NONE = 0,
	EDMA_REQ_STOP,
	EDMA_REQ_PAUSE
};

enum dw_edma_status {
	EDMA_ST_IDLE = 0,
	EDMA_ST_PAUSE,
	EDMA_ST_BUSY
};

enum dw_edma_xfer_type {
	EDMA_XFER_SCATTER_GATHER = 0,
	EDMA_XFER_CYCLIC,
	EDMA_XFER_INTERLEAVED
};

enum dw_iatu_inb_usage {
    IATU_INB_DMA_DESC  = 0,
    IATU_INB_USER0     = 1,
    IATU_INB_USER1     = 2,
    IATU_INB_USER2     = 3,
	IATU_INB_MAX       = 4
};

struct dw_edma_chan;
struct dw_edma_chunk;

struct dw_edma_burst {
	struct list_head		list;
	u64				sar;
	u64				dar;
	u32				sz;
};

struct dx_edma_region {
	phys_addr_t			paddr;
	void				__iomem *vaddr;
	size_t				sz;
};

struct dx_iatu_inbound {
	u8 mode;
	u64 base_addr;
	u64 tgt_addr;
	u64 size;
	u32 idx;
	u8 bar_no;
};

struct dx_user_region {
	phys_addr_t			paddr;
	phys_addr_t			ep_addr;
	void				__iomem *vaddr;
	size_t				sz;
	u32					bar_num;
};

struct dw_edma_chunk {
	struct list_head		list;
	struct dw_edma_chan		*chan;
	struct dw_edma_burst	*burst;

	u32						bursts_alloc;
	u8						cb;
	struct dx_edma_region	ll_region;	/* Linked list */
};

struct dw_edma_desc {
	struct virt_dma_desc	vd;
	struct dw_edma_chan		*chan;
	struct dw_edma_chunk	*chunk;

	u32						chunks_alloc;

	u32						alloc_sz;
	u32						xfer_sz;
};

struct dw_edma_chan {
	struct virt_dma_chan	vc;
	struct dw_edma_chip		*chip;
	int				id;
	enum dw_edma_dir		dir;

	u32						ll_max;

	struct msi_msg			msi;

	enum dw_edma_request	request;
	enum dw_edma_status		status;
	u8						configured;
	u8						set_desc; /* check to send description table */
	u8 						en_lie;   /* Generate a local interrupt to start npu */
	bool					is_llm;   /* Enable Linked List Mode */

	struct dma_slave_config	config;
};

struct dx_dma_user_irq {
	// struct xdma_dev *xdev;		/* parent device */
	struct dw_edma		*dw;
	u8					user_idx;			/* 0 ~ 15 */
	u8					events_irq;			/* accumulated IRQs */
	spinlock_t			events_lock;		/* lock to safely update events_irq */
	wait_queue_head_t	events_wq;	/* wait queue to sync waiting threads */
	irq_handler_t		handler;
	char				name[40];

	void *dev;
};

struct dx_edma_irq {
	struct msi_msg			msi;
	u32						wr_mask;
	u32						rd_mask;
	struct dw_edma			*dw;
	void					*data[EDMA_MAX_WR_CH][2];	/* user datas [dma channel number][write/read] */

	struct dx_dma_user_irq	user_irq;	/* user IRQ management for multi interrupt */
	struct dx_dma_user_irq	user_irqs[USER_IRQ_NUMS];	/* user IRQ management for one interrupt */
};

struct dx_edma_core_ops {
	int	(*irq_vector)(struct device *dev, unsigned int nr);
};
struct dw_edma {
	struct list_head		list_head; /* Detect multi-device */

	char					name[20];
	char					irq_name[EDMA_MAX_WR_CH][20];
	struct pci_dev			*pdev;
	struct dx_dma_pci_dev	*xpdev;
	u8						idx;

	struct dma_device		wr_edma;
	u16						wr_ch_cnt;

	struct dma_device		rd_edma;
	u16						rd_ch_cnt;

	struct dx_edma_region		rg_region;	/* Registers */
	struct dx_edma_region		ll_region_wr[EDMA_MAX_WR_CH];
	struct dx_edma_region		ll_region_rd[EDMA_MAX_RD_CH];
	struct dx_edma_region		dt_region_wr[EDMA_MAX_WR_CH]; /* not used */
	struct dx_edma_region		dt_region_rd[EDMA_MAX_RD_CH]; /* not used */

	u16							dx_ver;			/* DXNN Version */
	u16							user_bar_cnt;
	struct dx_user_region		npu_region[USER_NUM_MAX];
	u64							dma_desc_base_bar;
	u64							dma_desc_base_addr;
	u64							dma_desc_size;
	u32							dma_desc_bar_num;

	struct dx_edma_region		iatu_region;
	struct dx_iatu_inbound		iatu_inb[IATU_INB_MAX];

	struct dx_edma_irq			*irq;
	int							dma_irqs; /* number of edma irqs */
	int							nr_irqs;
	u16							event_irq_idx;	/* Error index of MSI vector table */
	
	enum dx_edma_map_format		mf;

	struct dw_edma_chan			*chan;
	int							rd_dma_id;	/* host - read  : DMA_WR */
	int							wr_dma_id;	/* host - write : DMA_RD */
	struct dma_chan 			*rd_dma_chan[EDMA_MAX_RD_CH];	/* DMA_READ */
	struct dma_chan 			*wr_dma_chan[EDMA_MAX_WR_CH];	/* DMA_WRITE */
	bool 						init_completed;
	int 						ref_count;	/* external module reference count */

	/* DXNN V2 only */
	u16							rd_dma_chan_list[EDMA_MAX_RD_CH]; /* 1 : used, 0: not-used */
	u16							wr_dma_chan_list[EDMA_MAX_WR_CH]; /* 1 : used, 0: not-used */
	struct mutex				rd_lock;
	struct mutex				wr_lock;
	struct dx_pcie_msg			*dx_msg;

	const struct dx_edma_core_ops	*ops;

	raw_spinlock_t				lock;		/* Only for legacy */

	/* Device Specific Datas */
	u64							download_region;
	u32							download_size;
	u64							booting_region[2];
#ifdef CONFIG_DEBUG_FS
	struct dentry			*debugfs;
#endif /* CONFIG_DEBUG_FS */
};

struct dw_edma_sg {
	struct scatterlist		*sgl;
	unsigned int			len;
};

struct dw_edma_cyclic {
	dma_addr_t			paddr;
	size_t				len;
	size_t				cnt;
};

struct dw_edma_transfer {
	struct dma_chan			*dchan;
	union dw_edma_xfer {
		struct dw_edma_sg		sg;
		struct dw_edma_cyclic		cyclic;
		struct dma_interleaved_template *il;
	} xfer;
	enum dma_transfer_direction	direction;
	unsigned long			flags;
	enum dw_edma_xfer_type		type;
};

static inline
struct dw_edma_chan *vc2dw_edma_chan(struct virt_dma_chan *vc)
{
	return container_of(vc, struct dw_edma_chan, vc);
}

static inline
struct dw_edma_chan *dchan2dw_edma_chan(struct dma_chan *dchan)
{
	return vc2dw_edma_chan(to_virt_chan(dchan));
}

int dx_dma_probe(struct dw_edma_chip *chip);
int dx_dma_remove(struct dw_edma_chip *chip);

void set_user_irq_vec_table(struct dw_edma *dw);
int get_nr_user_irqs(void);
int get_pos_user_irqs(int event_id);
bool check_event_id(int event_id);

#endif /* _DW_EDMA_CORE_H */
