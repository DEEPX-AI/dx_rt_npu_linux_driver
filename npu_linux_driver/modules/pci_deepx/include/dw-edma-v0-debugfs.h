// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef _DW_EDMA_V0_DEBUG_FS_H
#define _DW_EDMA_V0_DEBUG_FS_H

#include "dx_edma.h"

#ifdef CONFIG_DEBUG_FS
void dw_edma_v0_debugfs_on(struct dw_edma_chip *chip);
void dw_edma_v0_debugfs_off(struct dw_edma_chip *chip);
#else
static inline void dw_edma_v0_debugfs_on(struct dw_edma_chip *chip)
{
}

static inline void dw_edma_v0_debugfs_off(struct dw_edma_chip *chip)
{
}
#endif /* CONFIG_DEBUG_FS */

#endif /* _DW_EDMA_V0_DEBUG_FS_H */
