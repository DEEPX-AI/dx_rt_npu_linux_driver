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
u32 dw_edma_v0_core_ch_status_raw(struct dw_edma_chan *chan);
int dw_edma_v0_core_ch_status_checked(struct dw_edma_chan *chan, u32 *cs);
u32 dw_edma_v0_core_ch_recover_abort(struct dw_edma_chan *chan);
int dw_edma_v0_core_ch_soft_reset(struct dw_edma_chan *chan);
void dw_edma_v0_core_engine_cycle(struct dw_edma_chan *chan);
int dw_edma_v0_core_pcie_reset(struct dw_edma *dw);
int dw_edma_v0_core_edma_restore(struct dw_edma *dw);
void dw_edma_v0_core_clear_done_int(struct dw_edma_chan *chan);
void dw_edma_v0_core_clear_abort_int(struct dw_edma_chan *chan);
u32 dw_edma_v0_core_status_done_int(struct dw_edma *chan, enum dw_edma_dir dir);
u32 dw_edma_v0_core_status_abort_int(struct dw_edma *chan, enum dw_edma_dir dir);
int dw_edma_v0_core_prepare_start(struct dw_edma_chunk *chunk, bool first, bool is_llm);
void dw_edma_v0_core_launch_prepared(struct dw_edma_chunk *chunk, bool is_llm);
void dw_edma_v0_core_cancel_prepared(struct dw_edma_chunk *chunk, bool is_llm);
int dw_edma_v0_core_prebuild_chunk(struct dw_edma_chunk *chunk);
bool dw_edma_v0_core_shadow_precopy_available(struct dw_edma *dw);
int dw_edma_v0_core_precopy_lli(struct dw_edma_chunk *chunk,
				       struct dw_edma_desc *desc);
int dw_edma_v0_core_launch_precopied(struct dw_edma_chunk *chunk);
int dw_edma_v0_core_device_config(struct dw_edma_chan *chan);
void dw_edma_v0_core_engine_disable(struct dw_edma_chan *chan);
void dw_edma_v0_core_engine_enable(struct dw_edma_chan *chan);
/* eDMA debug fs callbacks */
void dw_edma_v0_core_debugfs_on(struct dw_edma_chip *chip);
void dw_edma_v0_core_debugfs_off(struct dw_edma_chip *chip);
/* iATU */
void dw_iatu_config_inbound(struct dw_edma *dw, u8 mode, u64 base_addr, u64 tgt_addr, u32 size, u32 idx, u8 bar_no);
void dw_iatu_default_config_set(struct dw_edma *dw);

#endif /* _DW_EDMA_V0_CORE_H */
