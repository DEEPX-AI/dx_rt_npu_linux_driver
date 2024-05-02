// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef DX_EDMA_H
#define DX_EDMA_H

#include <linux/types.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
#include <linux/dma/edma.h>
#else
#include <linux/device.h>
#include <linux/dmaengine.h>

struct dw_edma;

/**
 * struct dw_edma_chip - representation of DesignWare eDMA controller hardware
 * @dev:		 struct device of the eDMA controller
 * @id:			 instance ID
 * @irq:		 irq line
 * @dw:			 struct dw_edma that is filed by dw_edma_probe()
 */
struct dw_edma_chip {
	struct device		*dev;
	int			id;
	int			irq;
	struct dw_edma		*dw;
};
#endif /*(LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))*/

#endif /* DX_EDMA_H */
