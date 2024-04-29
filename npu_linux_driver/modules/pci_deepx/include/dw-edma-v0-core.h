// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef _DW_EDMA_V0_CORE_H
#define _DW_EDMA_V0_CORE_H

#include "dx_edma.h"
#include "dw-edma-core.h"

/* eDMA management callbacks */
void dw_edma_v0_core_off(struct dw_edma *chan);
u16 dw_edma_v0_core_ch_count(struct dw_edma *chan, enum dw_edma_dir dir);
enum dma_status dw_edma_v0_core_ch_status(struct dw_edma_chan *chan);
void dw_edma_v0_core_clear_done_int(struct dw_edma_chan *chan);
void dw_edma_v0_core_clear_abort_int(struct dw_edma_chan *chan);
u32 dw_edma_v0_core_status_done_int(struct dw_edma *chan, enum dw_edma_dir dir);
u32 dw_edma_v0_core_status_abort_int(struct dw_edma *chan, enum dw_edma_dir dir);
void dw_edma_v0_core_start(struct dw_edma_chunk *chunk, bool first, bool set_desc);
int dw_edma_v0_core_device_config(struct dw_edma_chan *chan);
/* eDMA debug fs callbacks */
void dw_edma_v0_core_debugfs_on(struct dw_edma_chip *chip);
void dw_edma_v0_core_debugfs_off(struct dw_edma_chip *chip);
/* iATU */
void dw_iatu_config_inbound(struct dw_edma *dw, u8 mode, u64 base_addr, u64 tgt_addr, u32 size, u32 idx, u8 bar_no);
void dw_iatu_default_config_set(struct dw_edma *dw);
void dw_iatu_desc_region_check(struct dw_edma *dw, u64 addr, u64 size);

#endif /* _DW_EDMA_V0_CORE_H */
