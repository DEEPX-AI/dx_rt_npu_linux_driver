// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver memory management
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#include "dw-edma-core.h"
#include "dw-edma-mem.h"
#include "dx_lib.h"

#ifdef DX_DEBUG_ENABLE /*DEEPX MODIFIED*/
	#ifdef dev_vdbg
		#undef dev_vdbg
		#define dev_vdbg		dev_err
	#endif
#endif

/* ---------------- Helper Functions ---------------- */

static int dw_edma_alloc_dma_mem(struct device *dev, struct dx_edma_region *region, bool *is_buddy)
{
	/* 1. Try CMA first */
	region->vaddr = dma_alloc_coherent(dev, region->sz, &region->paddr, GFP_KERNEL | __GFP_NOWARN);
	if (region->vaddr) {
		*is_buddy = false;
		dev_vdbg(dev, "[MEM][DMA][CMA] Alloc: paddr=%pad, sz=%zu\n", &region->paddr, region->sz);
		return 0;
	}

	/* 2. Fallback to Buddy Allocator */
	{
		int order = get_order(region->sz);
		region->vaddr = (void *)__get_free_pages(GFP_KERNEL | __GFP_COMP | __GFP_ZERO | __GFP_RETRY_MAYFAIL, order);
		
		if (!region->vaddr)
			return -ENOMEM;

		region->paddr = dma_map_single(dev, region->vaddr, region->sz, DMA_BIDIRECTIONAL);
		if (dma_mapping_error(dev, region->paddr)) {
			free_pages((unsigned long)region->vaddr, order);
			region->vaddr = NULL;
			return -ENOMEM;
		}
		
		*is_buddy = true;
		dev_vdbg(dev, "[MEM][DMA][BUDDY] Alloc: paddr=%pad, sz=%zu\n", &region->paddr, region->sz);
	}
	
	return 0;
}

static void dw_edma_free_dma_mem(struct device *dev, struct dx_edma_region *region, bool is_buddy)
{
	if (!region->vaddr)
		return;

	dev_vdbg(dev, "[MEM][DMA][%s] Free: paddr=%pad, sz=%zu\n", is_buddy ? "BUDDY" : "CMA", &region->paddr, region->sz);

	if (is_buddy) {
		dma_unmap_single(dev, region->paddr, region->sz, DMA_BIDIRECTIONAL);
		free_pages((unsigned long)region->vaddr, get_order(region->sz));
	} else {
		dma_free_coherent(dev, region->sz, region->vaddr, region->paddr);
	}
	region->vaddr = NULL;
}

void dw_edma_desc_cleanup_work(struct work_struct *work);

static struct dw_edma_chunk *dw_edma_alloc_chunk_from_pool(struct dw_edma *dw)
{
	struct dw_edma_chunk *chunk;
	struct dx_edma_region saved_region;
	unsigned long flags;
	int idx;

	spin_lock_irqsave(&dw->pool_lock, flags);
	if (dw->chunk_free_cnt > 0) {
		idx = dw->chunk_free_list[--dw->chunk_free_cnt];
		chunk = &dw->chunk_pool[idx];
		
		saved_region = chunk->host_region;
		spin_unlock_irqrestore(&dw->pool_lock, flags);
		
		memset(chunk, 0, sizeof(*chunk));
		chunk->host_region = saved_region;
		chunk->from_pool = true;
		dev_vdbg(&dw->pdev->dev, "[MEM][CHUNK][POOL] Alloc: idx=%d addr=%p buf_paddr=%pad buf_vaddr=%p\n", idx, chunk, &chunk->host_region.paddr, chunk->host_region.vaddr);
		return chunk;
	}
	spin_unlock_irqrestore(&dw->pool_lock, flags);
	return NULL;
}

static struct dw_edma_chunk *dw_edma_alloc_chunk_dynamic(struct dw_edma *dw)
{
	struct dw_edma_chunk *chunk;

	chunk = kzalloc(sizeof(*chunk), GFP_KERNEL);
	if (unlikely(!chunk)) {
		pr_err("chunk kernel memory alloc fail!\n");
		return NULL;
	}
	chunk->from_pool = false;

	chunk->host_region.sz = EDMA_CHUNK_SIZE;
	
	if (dw_edma_alloc_dma_mem(&dw->pdev->dev, &chunk->host_region, &chunk->is_buddy)) {
		pr_err("host region alloc fail (CMA & Buddy)\n");
		kfree(chunk);
		return NULL;
	}

	if (chunk->is_buddy)
		pr_debug("Allocated chunk from cached contiguous memory\n");
	else
		pr_debug("Allocated chunk from non-cached contiguous memory\n");

	dev_vdbg(&dw->pdev->dev, "[MEM][CHUNK][DYNAMIC] Alloc\n");
	return chunk;
}

static struct dw_edma_burst *dw_edma_alloc_burst_from_pool(struct dw_edma *dw)
{
	struct dw_edma_burst *burst;
	unsigned long flags;
	int idx;

	spin_lock_irqsave(&dw->pool_lock, flags);
	if (dw->burst_free_cnt > 0) {
		idx = dw->burst_free_list[--dw->burst_free_cnt];
		burst = &dw->burst_pool[idx];
		spin_unlock_irqrestore(&dw->pool_lock, flags);
		memset(burst, 0, sizeof(*burst));
		burst->from_pool = true;
		// dev_vdbg(&dw->pdev->dev, "[MEM][BURST][POOL] Alloc: idx=%d addr=%p\n", idx, burst);
		return burst;
	}
	spin_unlock_irqrestore(&dw->pool_lock, flags);
	return NULL;
}

static struct dw_edma_burst *dw_edma_alloc_burst_dynamic(struct dw_edma *dw)
{
	struct dw_edma_burst *burst;

	burst = kzalloc(sizeof(*burst), GFP_KERNEL);
	if (unlikely(!burst)) {
		pr_err("burst kernel memory alloc fail!\n");
		return NULL;
	}
	burst->from_pool = false;
	dev_vdbg(&dw->pdev->dev, "[MEM][BURST][DYNAMIC] Alloc: addr=%p\n", burst);
	return burst;
}

static struct dw_edma_desc *dw_edma_alloc_desc_from_pool(struct dw_edma *dw)
{
	struct dw_edma_desc *desc;
	unsigned long flags;
	int idx;

	spin_lock_irqsave(&dw->pool_lock, flags);
	if (dw->desc_free_cnt > 0) {
		idx = dw->desc_free_list[--dw->desc_free_cnt];
		desc = &dw->desc_pool[idx];
		spin_unlock_irqrestore(&dw->pool_lock, flags);
		memset(desc, 0, sizeof(*desc));
		desc->from_pool = true;
		dev_vdbg(&dw->pdev->dev, "[MEM][DESC][POOL] Alloc: idx=%d addr=%p\n", idx, desc);
		return desc;
	}
	spin_unlock_irqrestore(&dw->pool_lock, flags);
	return NULL;
}

static struct dw_edma_desc *dw_edma_alloc_desc_dynamic(struct dw_edma *dw)
{
	struct dw_edma_desc *desc;

	desc = kzalloc(sizeof(*desc), GFP_KERNEL);
	if (unlikely(!desc)) {
		pr_err("description kernel memory alloc fail!\n");
		return NULL;
	}
	desc->from_pool = false;
	dev_vdbg(&dw->pdev->dev, "[MEM][DESC][DYNAMIC] Alloc addr=%p\n", desc);
	return desc;
}

/* ---------------- Public API ---------------- */

bool is_in_burst_pool(struct dw_edma_chan *chan, struct dw_edma_burst *burst)
{
	struct dw_edma *dw = chan->chip->dw;
	if (!dw->burst_pool) return false;
	return (burst >= dw->burst_pool && burst < (dw->burst_pool + EDMA_GLOBAL_BURST_POOL_SIZE));
}
EXPORT_SYMBOL_GPL(is_in_burst_pool);

bool is_in_chunk_pool(struct dw_edma_chan *chan, struct dw_edma_chunk *chunk)
{
	struct dw_edma *dw = chan->chip->dw;
	if (!dw->chunk_pool) return false;
	return (chunk >= dw->chunk_pool && chunk < (dw->chunk_pool + EDMA_GLOBAL_CHUNK_POOL_SIZE));
}
EXPORT_SYMBOL_GPL(is_in_chunk_pool);

struct dw_edma_burst *dw_edma_alloc_burst(struct dw_edma_chunk *chunk)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	struct dw_edma_burst *burst;

	burst = dw_edma_alloc_burst_from_pool(dw);
	if (!burst) {
		burst = dw_edma_alloc_burst_dynamic(dw);
		if (!burst) return NULL;
	}

	INIT_LIST_HEAD(&burst->list);
	if (chunk->burst) {
		/* Create and add new element into the linked list */
		chunk->bursts_alloc++;
		list_add_tail(&burst->list, &chunk->burst->list);
	} else {
		/* List head */
		chunk->bursts_alloc = 0;
		chunk->burst = burst;
	}

	return burst;
}
EXPORT_SYMBOL_GPL(dw_edma_alloc_burst);

void dw_edma_free_single_burst(struct dw_edma_chan *chan, struct dw_edma_burst *burst)
{
	struct dw_edma *dw = chan->chip->dw;
	unsigned long flags;

	if (is_in_burst_pool(chan, burst)) {
		if (!burst->from_pool) {
			pr_err("BUG: Burst %p is in pool but from_pool is false! Fixing...\n", burst);
			burst->from_pool = true;
		}
	}

	if (burst->from_pool) {
		spin_lock_irqsave(&dw->pool_lock, flags);
		dw->burst_free_list[dw->burst_free_cnt++] = (burst - dw->burst_pool);
		spin_unlock_irqrestore(&dw->pool_lock, flags);
		// dev_vdbg(chan->chip->dev, "[MEM][BURST][POOL] Free: idx=%ld addr=%p\n", (burst - dw->burst_pool), burst);
	} else {
		dev_vdbg(chan->chip->dev, "[MEM][BURST][DYNAMIC] Free: addr=%p\n", burst);
		kfree(burst);
	}
}
EXPORT_SYMBOL_GPL(dw_edma_free_single_burst);

void dw_edma_free_burst(struct dw_edma_chunk *chunk)
{
	struct dw_edma_burst *child, *_next;
	struct dw_edma_chan *chan;

	if (!chunk)
		return;

	chan = chunk->chan;

	if (!chunk->burst)
		return;

	dev_vdbg(chan->chip->dev, "[MEM][BURST] Free Start: chunk %p count=%d\n", chunk, chunk->bursts_alloc + 1);

	/* Remove all the list elements */
	list_for_each_entry_safe(child, _next, &chunk->burst->list, list) {
		list_del(&child->list);
		dw_edma_free_single_burst(chan, child);
		chunk->bursts_alloc--;
	}

	/* Remove the list head */
	if (chunk->burst) {
		dw_edma_free_single_burst(chan, chunk->burst);
		chunk->burst = NULL;
	}
}
EXPORT_SYMBOL_GPL(dw_edma_free_burst);

struct dw_edma_chunk *dw_edma_alloc_chunk(struct dw_edma_desc *desc)
{
	struct dw_edma_chan *chan = desc->chan;
	struct dw_edma *dw = chan->chip->dw;
	struct dw_edma_chunk *chunk;
	unsigned long flags;

	chunk = dw_edma_alloc_chunk_from_pool(dw);
	if (!chunk) {
		chunk = dw_edma_alloc_chunk_dynamic(dw);
		if (!chunk) return NULL;
	}

	INIT_LIST_HEAD(&chunk->list);
	chunk->chan = chan;
	/* Toggling change bit (CB) in each chunk, this is a mechanism to
	 * inform the eDMA HW block that this is a new linked list ready
	 * to be consumed.
	 *  - Odd chunks originate CB equal to 0
	 *  - Even chunks originate CB equal to 1
	 */
	chunk->cb = !(desc->chunks_alloc % 2);
	if (chan->dir == EDMA_DIR_WRITE) {
		chunk->ll_region.paddr = dw->ll_region_wr[chan->id].paddr;
		chunk->ll_region.vaddr = dw->ll_region_wr[chan->id].vaddr;
		chunk->ll_region.sz = dw->ll_region_wr[chan->id].sz;
	} else {
		chunk->ll_region.paddr = dw->ll_region_rd[chan->id].paddr;
		chunk->ll_region.vaddr = dw->ll_region_rd[chan->id].vaddr;
		chunk->ll_region.sz = dw->ll_region_rd[chan->id].sz;
	}

	if (desc->chunk) {
		/* Create and add new element into the linked list */
		if (!dw_edma_alloc_burst(chunk)) {
			pr_err("burst alloc fail!!\n");
			if (chunk->from_pool) {
				spin_lock_irqsave(&dw->pool_lock, flags);
				dw->chunk_free_list[dw->chunk_free_cnt++] = (chunk - dw->chunk_pool);
				spin_unlock_irqrestore(&dw->pool_lock, flags);
			} else {
				dw_edma_free_dma_mem(chan->chip->dev, &chunk->host_region, chunk->is_buddy);
				kfree(chunk);
			}
			return NULL;
		}
		desc->chunks_alloc++;
		list_add_tail(&chunk->list, &desc->chunk->list);
		dev_vdbg(chan->chip->dev, "[MEM][CHUNK] Link: child %p -> desc %p\n", chunk, desc);
	} else {
		/* List head */
		chunk->burst = NULL;
		desc->chunks_alloc = 0;
		desc->chunk = chunk;
		dev_vdbg(chan->chip->dev, "[MEM][CHUNK] Link: head %p -> desc %p\n", chunk, desc);
	}
	dbg_tfr("[DMA_CH] List is created (ch num:%d)\n", desc->chunks_alloc);

	return chunk;
}
EXPORT_SYMBOL_GPL(dw_edma_alloc_chunk);

void dw_edma_free_chunk(struct dw_edma_desc *desc)
{
	struct dw_edma_chunk *child, *_next;
	struct dw_edma_chan *chan = desc->chan;
	struct dw_edma *dw = chan->chip->dw;
	unsigned long flags;
	bool in_atomic = in_interrupt() || in_atomic();

	if (!desc->chunk)
		return;

	dev_vdbg(chan->chip->dev, "[MEM][CHUNK] Free Start: desc %p (head %p) count=%d\n", desc, desc->chunk, desc->chunks_alloc + 1);

	/* Remove all the list elements */
	list_for_each_entry_safe(child, _next, &desc->chunk->list, list) {
		dw_edma_free_burst(child);

		if (is_in_chunk_pool(chan, child)) {
			if (!child->from_pool) {
				pr_err("BUG: Chunk %p is in pool but from_pool is false! Fixing...\n", child);
				child->from_pool = true;
			}
		}

		list_del(&child->list);

		/* Free DMA memory based on context */
		if (child->from_pool) {
			/* Pool chunk: just return to pool (no DMA free needed) */
			spin_lock_irqsave(&dw->pool_lock, flags);
			dw->chunk_free_list[dw->chunk_free_cnt++] = (child - dw->chunk_pool);
			spin_unlock_irqrestore(&dw->pool_lock, flags);
			dev_vdbg(chan->chip->dev, "[MEM][CHUNK][POOL] Free: idx=%ld addr=%p buf_paddr=%pad buf_vaddr=%p\n", (child - dw->chunk_pool), child, &child->host_region.paddr, child->host_region.vaddr);
		} else {
			/* Non-pool chunk */
			if (child->host_region.vaddr) {
				if (in_atomic) {
					/* Atomic context (interrupt/tasklet): DO NOT free DMA memory
					 * It will be freed in dw_edma_device_terminate_all() (process context)
					 * Mark vaddr as NULL to prevent double-free */
					pr_debug_ratelimited("Skipping DMA free in atomic context, will free in terminate_all\n");
					/* Keep vaddr intact - it will be freed in terminate_all */
				} else {
					/* Process context: safe to free DMA memory directly */
					dw_edma_free_dma_mem(chan->chip->dev, &child->host_region, child->is_buddy);
				}
			}
			dev_vdbg(chan->chip->dev, "[MEM][CHUNK][DYNAMIC] Free: addr=%p\n", child);
			kfree(child);
		}
	}

	/* Remove the list head */
	if (is_in_chunk_pool(chan, desc->chunk)) {
		if (!desc->chunk->from_pool) {
			pr_err("BUG: Chunk head %p is in pool but from_pool is false! Fixing...\n", desc->chunk);
			desc->chunk->from_pool = true;
		}
	}

	dw_edma_free_burst(desc->chunk);

	dev_vdbg(chan->chip->dev, "[MEM][CHUNK] Free Head: %p\n", desc->chunk);

	/* Free DMA memory for head chunk based on context */
	if (desc->chunk->from_pool) {
		/* Pool chunk: return to pool */
		spin_lock_irqsave(&dw->pool_lock, flags);
		dw->chunk_free_list[dw->chunk_free_cnt++] = (desc->chunk - dw->chunk_pool);
		spin_unlock_irqrestore(&dw->pool_lock, flags);
		dev_vdbg(chan->chip->dev, "[MEM][CHUNK][POOL] Free: idx=%ld addr=%p buf_paddr=%pad buf_vaddr=%p\n", (desc->chunk - dw->chunk_pool), desc->chunk, &desc->chunk->host_region.paddr, desc->chunk->host_region.vaddr);
	} else {
		/* Non-pool chunk */
		if (desc->chunk->host_region.vaddr) {
			if (in_atomic) {
				/* Atomic context: DO NOT free, will be freed in terminate_all */
				pr_debug_ratelimited("Skipping head DMA free in atomic context\n");
				/* Keep vaddr intact */
			} else {
				/* Process context: safe to free */
				dw_edma_free_dma_mem(chan->chip->dev, &desc->chunk->host_region, desc->chunk->is_buddy);
			}
		}
		dev_vdbg(chan->chip->dev, "[MEM][CHUNK][DYNAMIC] Free: addr=%p\n", desc->chunk);
		kfree(desc->chunk);
	}
	desc->chunk = NULL;
}
EXPORT_SYMBOL_GPL(dw_edma_free_chunk);

struct dw_edma_desc *dw_edma_alloc_desc(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	struct dw_edma_desc *desc;
	unsigned long flags;

	desc = dw_edma_alloc_desc_from_pool(dw);
	if (!desc) {
		desc = dw_edma_alloc_desc_dynamic(dw);
		if (!desc) return NULL;
	}

	desc->chan = chan;
	INIT_LIST_HEAD(&desc->pending_free_chunks);
	INIT_WORK(&desc->cleanup_work, dw_edma_desc_cleanup_work);
	if (!dw_edma_alloc_chunk(desc)) {
		if (desc->from_pool) {
			spin_lock_irqsave(&dw->pool_lock, flags);
			dw->desc_free_list[dw->desc_free_cnt++] = (desc - dw->desc_pool);
			spin_unlock_irqrestore(&dw->pool_lock, flags);
		} else {
			kfree(desc);
		}
		return NULL;
	}

	return desc;
}
EXPORT_SYMBOL_GPL(dw_edma_alloc_desc);

void dw_edma_free_desc(struct dw_edma_desc *desc)
{
	struct dw_edma_chan *chan = desc->chan;
	struct dw_edma *dw = chan->chip->dw;
	unsigned long flags;

	/* Logging Summary */
	{
		int c_pool = 0, c_dyn = 0;
		int b_pool = 0, b_dyn = 0;
		struct dw_edma_chunk *c;
		struct dw_edma_burst *b;
		
		if (desc->chunk) {
			if (desc->chunk->from_pool) c_pool++; else c_dyn++;
			if (desc->chunk->burst) {
				if (desc->chunk->burst->from_pool) b_pool++; else b_dyn++;
				list_for_each_entry(b, &desc->chunk->burst->list, list) {
					if (b->from_pool) b_pool++; else b_dyn++;
				}
			}
			list_for_each_entry(c, &desc->chunk->list, list) {
				if (c->from_pool) c_pool++; else c_dyn++;
				if (c->burst) {
					if (c->burst->from_pool) b_pool++; else b_dyn++;
					list_for_each_entry(b, &c->burst->list, list) {
						if (b->from_pool) b_pool++; else b_dyn++;
					}
				}
			}
		}
		dev_vdbg(&dw->pdev->dev, "[MEM][DESC] Summary: Chunks(P:%d/D:%d) Bursts(P:%d/D:%d)\n", c_pool, c_dyn, b_pool, b_dyn);
	}

	/* Cancel any pending cleanup work */
	cancel_work_sync(&desc->cleanup_work);
	
	/* Manually trigger cleanup if there are pending chunks */
	if (!list_empty(&desc->pending_free_chunks)) {
		dw_edma_desc_cleanup_work(&desc->cleanup_work);
	}

	dw_edma_free_chunk(desc);
	
	if (desc->from_pool) {
		spin_lock_irqsave(&dw->pool_lock, flags);
		dw->desc_free_list[dw->desc_free_cnt++] = (desc - dw->desc_pool);
		spin_unlock_irqrestore(&dw->pool_lock, flags);
		dev_vdbg(&dw->pdev->dev, "[MEM][DESC][POOL] Free: idx=%ld addr=%p\n", (desc - dw->desc_pool), desc);
	} else {
		dev_vdbg(&dw->pdev->dev, "[MEM][DESC][DYNAMIC] Free: addr=%p\n", desc);
		kfree(desc);
	}
}
EXPORT_SYMBOL_GPL(dw_edma_free_desc);

/* Descriptor-specific cleanup work (runs in process context) */
void dw_edma_desc_cleanup_work(struct work_struct *work)
{
	struct dw_edma_desc *desc = container_of(work, struct dw_edma_desc, cleanup_work);
	struct dw_edma_chunk *chunk, *tmp;
	struct device *dev = desc->chan->chip->dev;
	
	dev_vdbg(dev, "[MEM][DESC] Cleanup Work: desc=%p\n", desc);

	/* Free all non-pool chunks for this descriptor */
	list_for_each_entry_safe(chunk, tmp, &desc->pending_free_chunks, list) {
		if (chunk->host_region.vaddr) {
			dw_edma_free_dma_mem(dev, &chunk->host_region, chunk->is_buddy);
		}
		list_del(&chunk->list);
		kfree(chunk);
	}
	
	pr_debug("Freed non-pool chunks for descriptor %p\n", desc);
}
EXPORT_SYMBOL_GPL(dw_edma_desc_cleanup_work);

int dw_edma_mem_init(struct dw_edma *dw)
{
	struct device *dev = &dw->pdev->dev;
	int i;

	/* Initialize Global Memory Pools */
	spin_lock_init(&dw->pool_lock);

	/* 1. Allocate Global Chunk Pool (32MB CMA) */
	dw->chunk_pool = devm_kcalloc(dev, EDMA_GLOBAL_CHUNK_POOL_SIZE, sizeof(struct dw_edma_chunk), GFP_KERNEL);
	dev_vdbg(dev, "[MEM][INIT] chunk_pool addr=%p size=%zu\n", 
		dw->chunk_pool, 
		sizeof(struct dw_edma_chunk) * EDMA_GLOBAL_CHUNK_POOL_SIZE
	);
	dw->chunk_free_list = devm_kcalloc(dev, EDMA_GLOBAL_CHUNK_POOL_SIZE, sizeof(int), GFP_KERNEL);
	dev_vdbg(dev, "[MEM][INIT] chunk_free_list addr=%p size=%zu\n", 
		dw->chunk_free_list, 
		sizeof(int) * EDMA_GLOBAL_CHUNK_POOL_SIZE
	);
	if (dw->chunk_pool && dw->chunk_free_list) {
		dw->chunk_free_cnt = EDMA_GLOBAL_CHUNK_POOL_SIZE;
		for (i = 0; i < EDMA_GLOBAL_CHUNK_POOL_SIZE; i++) {
			dw->chunk_free_list[i] = i;
			dw->chunk_pool[i].host_region.sz = EDMA_CHUNK_SIZE;
			
			if (dw_edma_alloc_dma_mem(dev, &dw->chunk_pool[i].host_region, &dw->chunk_pool[i].is_buddy)) {
				dev_err(dev, "Failed to pre-allocate DMA buffer for global chunk pool %d (CMA & Buddy failed)\n", i);
				return -ENOMEM;
			}
			
			if (dw->chunk_pool[i].is_buddy)
				dev_vdbg(dev, "[MEM][INIT] Allocated chunk from cached contiguous memory\n");
			// else
			// 	dev_vdbg(dev, "[MEM][INIT] Allocated chunk from non-cached contiguous memory\n");
			
			dev_vdbg(dev, "[MEM][INIT] chunk_pool[%d] buddy=%d addr=%p size=%zu/%d\n", 
				dw->chunk_free_list[i], 
				dw->chunk_pool[i].is_buddy,
				dw->chunk_pool[i].host_region.vaddr,
				dw->chunk_pool[i].host_region.sz,
				EDMA_CHUNK_SIZE
			);
		}
	} else {
		return -ENOMEM;
	}

	/* 2. Allocate Global Burst Pool (~16MB RAM) */
	dw->burst_pool = vzalloc(sizeof(struct dw_edma_burst) * EDMA_GLOBAL_BURST_POOL_SIZE);
	dev_vdbg(dev, "[MEM][INIT] burst_pool addr=%p size=%zu\n", 
		dw->burst_pool, 
		sizeof(struct dw_edma_burst) * EDMA_GLOBAL_BURST_POOL_SIZE
	);
	dw->burst_free_list = vzalloc(sizeof(int) * EDMA_GLOBAL_BURST_POOL_SIZE);
	dev_vdbg(dev, "[MEM][INIT] burst_free_list addr=%p size=%zu\n", 
		dw->burst_free_list, 
		sizeof(int) * EDMA_GLOBAL_BURST_POOL_SIZE
	);
	if (dw->burst_pool && dw->burst_free_list) {
		dw->burst_free_cnt = EDMA_GLOBAL_BURST_POOL_SIZE;
		for (i = 0; i < EDMA_GLOBAL_BURST_POOL_SIZE; i++) {
			dw->burst_free_list[i] = i;
		}
	} else {
		if (dw->burst_pool) vfree(dw->burst_pool);
		if (dw->burst_free_list) vfree(dw->burst_free_list);
		return -ENOMEM;
	}

	/* 3. Allocate Global Desc Pool */
	dw->desc_pool = devm_kcalloc(dev, EDMA_GLOBAL_DESC_POOL_SIZE, sizeof(struct dw_edma_desc), GFP_KERNEL);
	dev_vdbg(dev, "[MEM][INIT] desc_pool addr=%p size=%zu\n", 
		dw->desc_pool, 
		sizeof(struct dw_edma_desc) * EDMA_GLOBAL_DESC_POOL_SIZE
	);
	dw->desc_free_list = devm_kcalloc(dev, EDMA_GLOBAL_DESC_POOL_SIZE, sizeof(int), GFP_KERNEL);
	dev_vdbg(dev, "[MEM][INIT] desc_free_list addr=%p size=%zu\n", 
		dw->desc_free_list, 
		sizeof(int) * EDMA_GLOBAL_DESC_POOL_SIZE
	);
	if (dw->desc_pool && dw->desc_free_list) {
		dw->desc_free_cnt = EDMA_GLOBAL_DESC_POOL_SIZE;
		for (i = 0; i < EDMA_GLOBAL_DESC_POOL_SIZE; i++) {
			dw->desc_free_list[i] = i;
		}
	} else {
		return -ENOMEM;
	}

	return 0;
}

void dw_edma_mem_deinit(struct dw_edma *dw)
{
	int i;
	struct device *dev = &dw->pdev->dev;

	dev_vdbg(dev, "[MEM][DEINIT] De-initializing memory pools\n");

	/* Free Global Chunk Pool DMA buffers */
	if (dw->chunk_pool) {
		for (i = 0; i < EDMA_GLOBAL_CHUNK_POOL_SIZE; i++) {
			if (dw->chunk_pool[i].host_region.vaddr) {
				dw_edma_free_dma_mem(dev, &dw->chunk_pool[i].host_region, dw->chunk_pool[i].is_buddy);
			}
		}
	}

	/* Free Global Burst Pool (vzalloc) */
	if (dw->burst_pool) {
		vfree(dw->burst_pool);
		dw->burst_pool = NULL;
	}
	if (dw->burst_free_list) {
		vfree(dw->burst_free_list);
		dw->burst_free_list = NULL;
	}
}
