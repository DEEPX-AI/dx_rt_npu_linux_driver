// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef __DX_DMA_VERSION_H__
#define __DX_DMA_VERSION_H__

#define DRV_MODULE_VERSION      \
	__stringify(PCIE_VERSION_MAJOR) "." \
	__stringify(PCIE_VERSION_MINOR) "." \
	__stringify(PCIE_VERSION_PATCH)

#define DRV_MOD_VERSION_NUMBER  \
	((PCIE_VERSION_MAJOR)*1000 + (PCIE_VERSION_MINOR)*100 + PCIE_VERSION_PATCH)

#endif /* ifndef __DX_DMA_VERSION_H__ */
