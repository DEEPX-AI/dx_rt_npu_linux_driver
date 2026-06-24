/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver memory management
 */

#ifndef _DW_EDMA_MEM_H_
#define _DW_EDMA_MEM_H_

#include "dw-edma-core.h"

int dw_edma_mem_init(struct dw_edma *dw);
void dw_edma_mem_deinit(struct dw_edma *dw);

struct dw_edma_chunk *dw_edma_alloc_chunk(struct dw_edma_desc *desc);
void dw_edma_free_chunk(struct dw_edma_desc *desc);
void dw_edma_defer_chunk_free(struct dw_edma_chan *chan,
				      struct dw_edma_chunk *chunk);

struct dw_edma_burst *dw_edma_alloc_burst(struct dw_edma_chunk *chunk);
void dw_edma_free_burst(struct dw_edma_chunk *chunk);
void dw_edma_free_single_burst(struct dw_edma_chan *chan, struct dw_edma_burst *burst);
int dw_edma_alloc_burst_batch(struct dw_edma_chunk *chunk, u32 count,
			      struct dw_edma_burst **out);
void dw_edma_free_burst_batch(struct dw_edma_chan *chan,
			      struct dw_edma_burst **bursts, u32 count);

struct dw_edma_desc *dw_edma_alloc_desc(struct dw_edma_chan *chan);
void dw_edma_free_desc(struct dw_edma_desc *desc);

struct dw_edma_chunk *dw_edma_alloc_unlinked_chunk(struct dw_edma_chan *chan);
void dw_edma_free_unlinked_chunk(struct dw_edma_chan *chan,
				 struct dw_edma_chunk *chunk);

bool is_in_burst_pool(struct dw_edma_chan *chan, struct dw_edma_burst *burst);

/*
 * Per-channel descriptor buffer (1MB) — allocated at probe, freed at remove.
 * Helper RD channels (ch2/3) are skipped — they don't need a host buffer.
 * Returns 0 on success, -ENOMEM on failure.  Idempotent: re-calling on a
 * channel that already has a desc_buf is a no-op (safe across SBR/recovery).
 */
int dx_dma_chan_alloc_desc_buf(struct dw_edma_chan *chan);
void dx_dma_chan_free_desc_buf(struct dw_edma_chan *chan);

#endif /* _DW_EDMA_MEM_H_ */
