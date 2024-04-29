// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef __DX_CDEV_CTRL_H__
#define __DX_CDEV_CTRL_H__

#include <linux/ioctl.h>
#include "dx_cdev.h"

/* Use 'x' as magic number */
#define DX_DMA_IOC_MAGIC	'x'
/* XL OpenCL X->58(ASCII), L->6C(ASCII), O->0 C->C L->6C(ASCII); */
#define DX_DMA_XCL_MAGIC 0X586C0C6C

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 *
 * _IO(type,nr)		    no arguments
 * _IOR(type,nr,datatype)   read data from driver
 * _IOW(type,nr.datatype)   write data to driver
 * _IORW(type,nr,datatype)  read/write data
 *
 * _IOC_DIR(nr)		    returns direction
 * _IOC_TYPE(nr)	    returns magic
 * _IOC_NR(nr)		    returns number
 * _IOC_SIZE(nr)	    returns size
 */

enum DX_DMA_IOC_TYPES {
	DX_DMA_IOC_NOP,
	DX_DMA_IOC_INFO,
	DX_DMA_IOC_OFFLINE,
	DX_DMA_IOC_ONLINE,
	DX_DMA_IOC_MAX
};

struct dx_dma_ioc_base {
	unsigned int magic;
	unsigned int command;
};

struct dx_dma_ioc_info {
	struct dx_dma_ioc_base	base;
	unsigned short		vendor;
	unsigned short		device;
	unsigned short		subsystem_vendor;
	unsigned short		subsystem_device;
	unsigned int		dma_engine_version;
	unsigned int		driver_version;
	unsigned long long	feature_id;
	unsigned short		domain;
	unsigned char		bus;
	unsigned char		dev;
	unsigned char		func;
};

/* IOCTL codes */
#define DX_DMA_IOCINFO		_IOWR(DX_DMA_IOC_MAGIC, DX_DMA_IOC_INFO, \
					struct dx_dma_ioc_info)
#define DX_DMA_IOCOFFLINE	_IO(DX_DMA_IOC_MAGIC, DX_DMA_IOC_OFFLINE)
#define DX_DMA_IOCONLINE	_IO(DX_DMA_IOC_MAGIC, DX_DMA_IOC_ONLINE)

#define IOCTL_XDMA_ADDRMODE_SET	_IOW('q', 4, int)
#define IOCTL_XDMA_ADDRMODE_GET	_IOR('q', 5, int)
#define IOCTL_XDMA_ALIGN_GET	_IOR('q', 6, int)

void dx_pcie_interrupt_err(int dev_id);
void dx_cdev_ctrl_init(struct dx_dma_cdev *xcdev);

#endif /* __DX_CDEV_CTRL_H__ */
