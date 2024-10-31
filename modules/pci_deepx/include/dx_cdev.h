// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef __DX_CHRDEV_H__
#define __DX_CHRDEV_H__

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/cdev.h>

#include "dw-edma-core.h"

#define DX_DMA_NODE_NAME	"dx_dma"
#define DX_DMA_MINOR_BASE (0)
#define DX_DMA_MINOR_COUNT (255)

#define MAGIC_ENGINE	0xEEEEEEEEUL
#define MAGIC_DEVICE	0xDDDDDDDDUL
#define MAGIC_CHAR		0xCCCCCCCCUL
#define MAGIC_BITSTREAM 0xBBBBBBBBUL


#define EDMA_CHANNEL_NUM_MAX      4
#define EDMA_EVENT_NUM_MAX        16

extern unsigned int desc_blen_max;
extern unsigned int h2c_timeout;
extern unsigned int c2h_timeout;


struct dx_dma_io_cb {
	char __user *buf;
	size_t len;
	void *private;
	unsigned int pages_nr;
	struct sg_table sgt;
	struct page **pages;
	int dma_ch_id;
	int npu_id;
	/** total data size */
	unsigned int count;
	/** MM only, DDR/BRAM memory addr */
	u64 ep_addr;
	/** write: if write to the device */
	// struct xdma_request_cb *req;
	u8 write:1;
	bool npu_run;
	// void (*io_done)(unsigned long cb_hndl, int err);
	char result; /* 0:PASS, -1:FAIL */
	bool is_llm; /* linked list mode */
};

struct dx_dma_cdev {
	unsigned long magic;	/* structure ID for sanity checks */
	struct dx_dma_pci_dev *xpdev;
	dev_t cdevno;			/* character device major:minor */
	struct cdev cdev;		/* character device embedded struct */
	int bar;				/* PCIe BAR for HW access, if needed */
	int npu_id;				/* NPU ID, if needed */
	bool write;				/* c2h chdev : true, h2c chdev : false*/
	unsigned long base;		/* bar access offset */
	struct dx_dma_user_irq *user_irq;	/* IRQ value, if needed */
	struct device *sys_device;	/* sysfs device */
	spinlock_t lock;
	int f_count;			/* file open count */
};

/* PCIe device specific book-keeping */
struct dx_dma_pci_dev {
	unsigned long magic;		/* structure ID for sanity checks */
	struct pci_dev *pdev;		/* pci device struct from probe() */
	struct dw_edma *dw;

	int major;		/* major number */
	int instance;		/* instance number */
	int user_max;
	int c2h_channel_max;
	int h2c_channel_max;

	unsigned int flags;

	/* character device structures */
	struct dx_dma_cdev ctrl_cdev;
	struct dx_dma_cdev sgdma_c2h_cdev[EDMA_CHANNEL_NUM_MAX];
	struct dx_dma_cdev sgdma_h2c_cdev[EDMA_CHANNEL_NUM_MAX];
	struct dx_dma_cdev events_cdev[EDMA_EVENT_NUM_MAX];

	struct dx_dma_cdev user_cdev[4];
	struct dx_dma_cdev bypass_c2h_cdev[EDMA_CHANNEL_NUM_MAX];
	struct dx_dma_cdev bypass_h2c_cdev[EDMA_CHANNEL_NUM_MAX];
	struct dx_dma_cdev bypass_cdev_base;

	void *data;
};

struct cdev_async_io {
	struct kiocb *iocb;
	struct dx_dma_io_cb *cb;
	bool write;
	bool cancel;
	int cmpl_cnt;
	int req_cnt;
	spinlock_t lock;
	struct work_struct wrk_itm;
	struct cdev_async_io *next;
	ssize_t res;
	ssize_t res2;
	int err_cnt;
};

int char_open(struct inode *inode, struct file *file);
int char_close(struct inode *inode, struct file *file);
int dx_cdev_check(const char *fname, struct dx_dma_cdev *xcdev, bool check_engine);
void dx_cdev_event_init(struct dx_dma_cdev *xcdev, u8 idx);

int xpdev_create_interfaces(struct dw_edma_chip *chip);
void xpdev_release_interfaces(struct dx_dma_pci_dev *xpdev);
int dx_cdev_init(void);
void dx_cdev_cleanup(void);

#endif /* __DX_CHRDEV_H__ */
