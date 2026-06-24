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
#include "dx_util.h"

#ifdef DX_DEBUG_ENABLE /*DEEPX MODIFIED*/
	#ifdef dev_vdbg
		#undef dev_vdbg
		#define dev_vdbg		dev_err
	#endif
#endif

/*
 * Step 3: descriptor staging is now per-channel (chan->desc_buf, 1MB each).
 * The legacy `allow_desc_dynamic_alloc` module param controlled a runtime
 * 1MB kmalloc fallback for the old global pool — no longer needed.
 */

/* ---------------- Helper Functions ---------------- */

static int dw_edma_alloc_dma_mem(struct device *dev, struct dx_edma_region *region, bool *is_buddy)
{
	/* 1. Try CMA first */
	region->vaddr = (void __force __iomem *)dma_alloc_coherent(dev, region->sz, &region->paddr, GFP_KERNEL | __GFP_NOWARN);
	if (region->vaddr) {
		*is_buddy = false;
		dev_vdbg(dev, "[MEM][DMA][CMA] Alloc: paddr=%pad, sz=%zu\n", &region->paddr, region->sz);
		return 0;
	}

	/* 2. Fallback to Buddy Allocator */
	{
		int order = get_order(region->sz);
		region->vaddr = (void __force __iomem *)__get_free_pages(GFP_KERNEL | __GFP_COMP | __GFP_ZERO | __GFP_RETRY_MAYFAIL, order);
		
		if (!region->vaddr)
			return -ENOMEM;

		region->paddr = dma_map_single(dev, (void __force *)region->vaddr, region->sz, DMA_BIDIRECTIONAL);
		if (dma_mapping_error(dev, region->paddr)) {
			free_pages((unsigned long)(void __force *)region->vaddr, order);
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
		free_pages((unsigned long)(void __force *)region->vaddr, get_order(region->sz));
	} else {
		dma_free_coherent(dev, region->sz, (void __force *)region->vaddr, region->paddr);
	}
	region->vaddr = NULL;
}

static void dw_edma_cleanup_work(struct work_struct *work)
{
	struct dw_edma *dw = container_of(work, struct dw_edma, cleanup_work);
	struct dw_edma_chunk *chunk, *tmp;
	LIST_HEAD(cleanup_list);
	unsigned long flags;

	spin_lock_irqsave(&dw->cleanup_lock, flags);
	list_splice_tail_init(&dw->cleanup_chunks, &cleanup_list);
	spin_unlock_irqrestore(&dw->cleanup_lock, flags);

	list_for_each_entry_safe(chunk, tmp, &cleanup_list, list) {
		list_del(&chunk->list);
		dw_edma_free_burst(chunk);
		kfree(chunk);
	}
}

void dw_edma_defer_chunk_free(struct dw_edma_chan *chan,
				      struct dw_edma_chunk *chunk)
{
	struct dw_edma *dw;
	unsigned long flags;

	if (!chan || !chunk || !chan->chip)
		return;

	dw = chan->chip->dw;
	if (!dw)
		return;

	spin_lock_irqsave(&dw->cleanup_lock, flags);
	list_add_tail(&chunk->list, &dw->cleanup_chunks);
	spin_unlock_irqrestore(&dw->cleanup_lock, flags);

	schedule_work(&dw->cleanup_work);
}
EXPORT_SYMBOL_GPL(dw_edma_defer_chunk_free);

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

struct dw_edma_burst *dw_edma_alloc_burst(struct dw_edma_chunk *chunk)
{
	struct dw_edma_chan *chan = chunk->chan;
	struct dw_edma *dw = chan->chip->dw;
	struct dw_edma_burst *burst;

	burst = dw_edma_alloc_burst_from_pool(dw);
	if (!burst) {
		/* Dynamic fallback (GFP_KERNEL) — not safe in atomic context */
		if (in_interrupt() || in_atomic()) {
			pr_err_ratelimited("[POOL] burst alloc failed in atomic (free=%d)\n",
					   dw->burst_free_cnt);
			return NULL;
		}
		burst = dw_edma_alloc_burst_dynamic(dw);
		if (!burst) {
			pr_err("[POOL] burst alloc failed: pool free=%d, dynamic OOM\n",
			       dw->burst_free_cnt);
			return NULL;
		}
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
			pr_err_ratelimited("BUG: Burst %p is in pool but from_pool is false! Fixing...\n", burst);
			burst->from_pool = true;
		}
	}

	if (burst->from_pool) {
		spin_lock_irqsave(&dw->pool_lock, flags);
		if (likely(dw->burst_free_cnt < EDMA_GLOBAL_BURST_POOL_SIZE))
			dw->burst_free_list[dw->burst_free_cnt++] = (burst - dw->burst_pool);
		spin_unlock_irqrestore(&dw->pool_lock, flags);
		// dev_vdbg(chan->chip->dev, "[MEM][BURST][POOL] Free: idx=%ld addr=%p\n", (burst - dw->burst_pool), burst);
	} else {
		dev_vdbg(chan->chip->dev, "[MEM][BURST][DYNAMIC] Free: addr=%p\n", burst);
		kfree(burst);
	}
}
EXPORT_SYMBOL_GPL(dw_edma_free_single_burst);

void dw_edma_free_burst_batch(struct dw_edma_chan *chan,
			      struct dw_edma_burst **bursts, u32 count)
{
	struct dw_edma *dw = chan->chip->dw;
	struct dw_edma_burst *burst, *_next;
	unsigned long flags;
	u32 i;
	LIST_HEAD(dynamic_list);

	if (!bursts || !count)
		return;

	spin_lock_irqsave(&dw->pool_lock, flags);
	for (i = 0; i < count; i++) {
		burst = bursts[i];
		if (!burst)
			continue;

		if (unlikely(is_in_burst_pool(chan, burst) && !burst->from_pool)) {
			pr_err_ratelimited("BUG: Burst %p is in pool but from_pool is false! Fixing...\n",
					   burst);
			burst->from_pool = true;
		}

		if (burst->from_pool &&
		    likely(dw->burst_free_cnt < EDMA_GLOBAL_BURST_POOL_SIZE)) {
			dw->burst_free_list[dw->burst_free_cnt++] =
				(burst - dw->burst_pool);
		} else if (!burst->from_pool) {
			INIT_LIST_HEAD(&burst->list);
			list_add(&burst->list, &dynamic_list);
		}
		bursts[i] = NULL;
	}
	spin_unlock_irqrestore(&dw->pool_lock, flags);

	list_for_each_entry_safe(burst, _next, &dynamic_list, list) {
		list_del(&burst->list);
		kfree(burst);
	}
}
EXPORT_SYMBOL_GPL(dw_edma_free_burst_batch);

void dw_edma_free_burst(struct dw_edma_chunk *chunk)
{
	struct dw_edma_burst *child, *_next;
	struct dw_edma_chan *chan;
	struct dw_edma *dw;
	unsigned long flags;
	LIST_HEAD(dynamic_list);

	if (!chunk)
		return;

	chan = chunk->chan;
	dw = chan->chip->dw;

	if (!chunk->burst)
		return;

	dev_vdbg(chan->chip->dev, "[MEM][BURST] Free Start: chunk %p count=%d\n", chunk, chunk->bursts_alloc + 1);

	/* Batch free: single spinlock for all pool bursts */
	spin_lock_irqsave(&dw->pool_lock, flags);

	/* Free all the list elements */
	list_for_each_entry_safe(child, _next, &chunk->burst->list, list) {
		list_del(&child->list);
		if (unlikely(is_in_burst_pool(chan, child) && !child->from_pool)) {
			pr_err_ratelimited("BUG: Burst %p is in pool but from_pool is false! Fixing...\n",
					   child);
			child->from_pool = true;
		}
		if (child->from_pool &&
		    likely(dw->burst_free_cnt < EDMA_GLOBAL_BURST_POOL_SIZE)) {
			dw->burst_free_list[dw->burst_free_cnt++] = (child - dw->burst_pool);
		} else if (!child->from_pool) {
			list_add(&child->list, &dynamic_list);
		}
		chunk->bursts_alloc--;
	}

	/* Free the list head */
	if (unlikely(is_in_burst_pool(chan, chunk->burst) && !chunk->burst->from_pool)) {
		pr_err_ratelimited("BUG: Burst %p is in pool but from_pool is false! Fixing...\n",
				   chunk->burst);
		chunk->burst->from_pool = true;
	}
	if (chunk->burst->from_pool &&
	    likely(dw->burst_free_cnt < EDMA_GLOBAL_BURST_POOL_SIZE)) {
		dw->burst_free_list[dw->burst_free_cnt++] = (chunk->burst - dw->burst_pool);
	} else if (!chunk->burst->from_pool) {
		list_add(&chunk->burst->list, &dynamic_list);
	}

	spin_unlock_irqrestore(&dw->pool_lock, flags);

	/* Free dynamic bursts outside the spinlock */
	list_for_each_entry_safe(child, _next, &dynamic_list, list) {
		list_del(&child->list);
		kfree(child);
	}

	chunk->burst = NULL;
}
EXPORT_SYMBOL_GPL(dw_edma_free_burst);

int dw_edma_alloc_burst_batch(struct dw_edma_chunk *chunk, u32 count,
			     struct dw_edma_burst **out)
{
	struct dw_edma *dw = chunk->chan->chip->dw;
	unsigned long flags;
	u32 allocated = 0;
	int idx;
	ktime_t alloc_start;
	bool perf_enabled = READ_ONCE(g_perf_enabled);

	/*
	 * Batch alloc: acquire pool_lock once for all requested bursts.
	 * This reduces ISR overhead from ~43K spinlock round-trips to 1
	 * when refilling a full chunk's worth of bursts.
	 */
	if (perf_enabled)
		get_start_time(&alloc_start);
	spin_lock_irqsave(&dw->pool_lock, flags);
	while (allocated < count && dw->burst_free_cnt > 0) {
		idx = dw->burst_free_list[--dw->burst_free_cnt];
		out[allocated] = &dw->burst_pool[idx];
		out[allocated]->from_pool = true;
		allocated++;
	}
	spin_unlock_irqrestore(&dw->pool_lock, flags);
	if (perf_enabled)
		dx_pcie_perf_record_pool_alloc(dw->idx, count, allocated,
			get_elapsed_time_ns(alloc_start));

	/*
	 * Pool ownership is initialized here, not by callers.  Field initialization
	 * stays in the SG population loops to avoid a separate cache-thrashing pass
	 * over the burst working set.
	 */

	return allocated;
}
EXPORT_SYMBOL_GPL(dw_edma_alloc_burst_batch);

/*
 * Step 2: Allocate chunk metadata (small kmalloc) and bind it to the
 * channel's pre-allocated 1MB desc_buf.  Reuse-safe because:
 *  - HW reads descriptors only from device LL SRAM after precopy_lli /
 *    xfer_llm_desc completes.
 *  - vc.lock + desc->shadow_state machine serialize writers/readers of
 *    chan->desc_buf.
 *
 * Helper RD channels (ch2/3) must never reach this — they have no desc_buf.
 */
static struct dw_edma_chunk *dw_edma_alloc_chunk_for_chan(struct dw_edma_chan *chan)
{
	struct dw_edma_chunk *chunk;

	if (WARN_ON_ONCE(!chan->desc_buf.vaddr)) {
		pr_err_ratelimited("[CHUNK] alloc on channel without desc_buf (helper?)\n");
		return NULL;
	}

	chunk = kzalloc(sizeof(*chunk), GFP_KERNEL);
	if (!chunk) {
		pr_err_ratelimited("[CHUNK] metadata kmalloc fail\n");
		return NULL;
	}

	chunk->host_region = chan->desc_buf;	/* struct copy: vaddr/paddr/sz */

	dev_vdbg(chan->chip->dev,
		 "[MEM][CHUNK][CHAN] Alloc: chan=%s%d chunk=%p host_paddr=%pad\n",
		 chan->dir == EDMA_DIR_WRITE ? "WR" : "RD", chan->id,
		 chunk, &chunk->host_region.paddr);
	return chunk;
}

struct dw_edma_chunk *dw_edma_alloc_chunk(struct dw_edma_desc *desc)
{
	struct dw_edma_chan *chan = desc->chan;
	struct dw_edma *dw = chan->chip->dw;
	struct dw_edma_chunk *chunk;

	chunk = dw_edma_alloc_chunk_for_chan(chan);
	if (!chunk)
		return NULL;

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
			/*
			 * Step 2: chunk metadata is kmalloc'd, host_region is
			 * borrowed from chan->desc_buf — never free host_region.
			 */
			kfree(chunk);
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

	if (!desc->chunk)
		return;

	dev_vdbg(chan->chip->dev, "[MEM][CHUNK] Free Start: desc %p (head %p) count=%d\n", desc, desc->chunk, desc->chunks_alloc + 1);

	/* Remove all the list elements */
	list_for_each_entry_safe(child, _next, &desc->chunk->list, list) {
		dw_edma_free_burst(child);
		list_del(&child->list);
		/*
		 * Step 4: chunks always borrow host_region from chan->desc_buf;
		 * the per-channel buffer is freed at remove() time, never here.
		 */
		dev_vdbg(chan->chip->dev, "[MEM][CHUNK] Free child: addr=%p\n", child);
		kfree(child);
	}

	/* Remove the list head */
	if (desc->chunk == &desc->chunk_head) {
		dw_edma_free_burst(desc->chunk);
		desc->chunk = NULL;
		return;
	}

	dw_edma_free_burst(desc->chunk);

	dev_vdbg(chan->chip->dev, "[MEM][CHUNK] Free Head: %p\n", desc->chunk);

	kfree(desc->chunk);
	desc->chunk = NULL;
}
EXPORT_SYMBOL_GPL(dw_edma_free_chunk);

struct dw_edma_desc *dw_edma_alloc_desc(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	struct dw_edma_desc *desc;

	desc = dw_edma_alloc_desc_from_pool(dw);
	if (!desc) {
		desc = dw_edma_alloc_desc_dynamic(dw);
		if (!desc) return NULL;
	}

	desc->chan = chan;

	/* Embedded list head: do not consume a 1MB descriptor staging chunk just
	 * to anchor desc->chunk->list.  Child chunks below are the only buffers
	 * that own host_region memory. */
	memset(&desc->chunk_head, 0, sizeof(desc->chunk_head));
	INIT_LIST_HEAD(&desc->chunk_head.list);
	desc->chunk_head.chan = chan;
	desc->chunk = &desc->chunk_head;
	desc->chunks_alloc = 0;

	return desc;
}
EXPORT_SYMBOL_GPL(dw_edma_alloc_desc);

void dw_edma_free_desc(struct dw_edma_desc *desc)
{
	struct dw_edma_chan *chan = desc->chan;
	struct dw_edma *dw = chan->chip->dw;
	unsigned long flags;

	/* Cancel and free shadow pre-build if active or already initialized.
	 * shadow_work can set SHADOW_IDLE before the callback returns; always
	 * synchronize initialized work so descriptor memory is not freed while
	 * the worker is still unwinding. */
	if (desc->shadow_work_initialized) {
		if (desc->shadow_state != SHADOW_IDLE)
			desc->shadow_state = SHADOW_CANCELLED;
		cancel_work_sync(&desc->shadow_work);
		desc->shadow_work_initialized = false;
	}
	if (desc->shadow_chunk) {
		dw_edma_free_unlinked_chunk(desc->chan, desc->shadow_chunk);
		desc->shadow_chunk = NULL;
	}
	if (desc->shadow_next_chunk) {
		dw_edma_free_unlinked_chunk(desc->chan, desc->shadow_next_chunk);
		desc->shadow_next_chunk = NULL;
	}
	if (desc->lazy_work_initialized) {
		desc->lazy_refill_state = LAZY_REFILL_IDLE;
		cancel_work_sync(&desc->lazy_work);
		desc->lazy_work_initialized = false;
	}

	dw_edma_free_chunk(desc);
	
	if (desc->from_pool) {
		spin_lock_irqsave(&dw->pool_lock, flags);
		if (likely(dw->desc_free_cnt < EDMA_GLOBAL_DESC_POOL_SIZE))
			dw->desc_free_list[dw->desc_free_cnt++] = (desc - dw->desc_pool);
		spin_unlock_irqrestore(&dw->pool_lock, flags);
		dev_vdbg(&dw->pdev->dev, "[MEM][DESC][POOL] Free: idx=%ld addr=%p\n", (desc - dw->desc_pool), desc);
	} else {
		dev_vdbg(&dw->pdev->dev, "[MEM][DESC][DYNAMIC] Free: addr=%p\n", desc);
		kfree(desc);
	}
}
EXPORT_SYMBOL_GPL(dw_edma_free_desc);

/*
 * Allocate a standalone chunk not linked to any descriptor.
 * Used for shadow pre-build: the WQ populates this chunk while current DMA runs.
 */
struct dw_edma_chunk *dw_edma_alloc_unlinked_chunk(struct dw_edma_chan *chan)
{
	struct dw_edma *dw = chan->chip->dw;
	struct dw_edma_chunk *chunk;

	chunk = dw_edma_alloc_chunk_for_chan(chan);
	if (!chunk)
		return NULL;

	INIT_LIST_HEAD(&chunk->list);
	chunk->chan = chan;
	chunk->burst = NULL;
	chunk->bursts_alloc = 0;
	chunk->cb = 0;

	/* Set up ll_region from channel's BAR0 region */
	if (chan->dir == EDMA_DIR_WRITE) {
		chunk->ll_region.paddr = dw->ll_region_wr[chan->id].paddr;
		chunk->ll_region.vaddr = dw->ll_region_wr[chan->id].vaddr;
		chunk->ll_region.sz = dw->ll_region_wr[chan->id].sz;
	} else {
		chunk->ll_region.paddr = dw->ll_region_rd[chan->id].paddr;
		chunk->ll_region.vaddr = dw->ll_region_rd[chan->id].vaddr;
		chunk->ll_region.sz = dw->ll_region_rd[chan->id].sz;
	}

	return chunk;
}
EXPORT_SYMBOL_GPL(dw_edma_alloc_unlinked_chunk);

/*
 * Free a standalone (unlinked) chunk: free all bursts, then return
 * the chunk itself to the pool or free dynamically.
 */
void dw_edma_free_unlinked_chunk(struct dw_edma_chan *chan,
				 struct dw_edma_chunk *chunk)
{
	if (!chunk)
		return;

	dw_edma_free_burst(chunk);
	/*
	 * Step 4: host_region is always borrowed from chan->desc_buf.
	 * Never free it; only kfree the metadata.
	 */
	kfree(chunk);
}
EXPORT_SYMBOL_GPL(dw_edma_free_unlinked_chunk);

/*
 * Helper-channel detection: RD ch2/3 are dedicated to device-to-device
 * LLI copies and never own a host descriptor buffer.
 */
static bool dx_dma_chan_is_helper(const struct dw_edma_chan *chan)
{
	return chan->dir == EDMA_DIR_READ &&
	       chan->id >= DX_READ_HELPER_CH_FIRST &&
	       chan->id <= DX_READ_HELPER_CH_LAST;
}

int dx_dma_chan_alloc_desc_buf(struct dw_edma_chan *chan)
{
	struct device *dev;
	int ret;

	if (!chan || !chan->chip)
		return -EINVAL;

	if (dx_dma_chan_is_helper(chan)) {
		/* Helper channels never need a host descriptor buffer. */
		chan->desc_buf.vaddr = NULL;
		chan->desc_buf.paddr = 0;
		chan->desc_buf.sz = 0;
		chan->desc_buf_is_buddy = false;
		return 0;
	}

	if (chan->desc_buf.vaddr) {
		/* Already allocated — idempotent across recovery paths. */
		return 0;
	}

	dev = chan->chip->dev;
	chan->desc_buf.sz = EDMA_CHUNK_SIZE;
	ret = dw_edma_alloc_dma_mem(dev, &chan->desc_buf, &chan->desc_buf_is_buddy);
	if (ret) {
		dev_err(dev, "[CHAN][%s%d] Failed to allocate desc_buf (1MB): %d\n",
			chan->dir == EDMA_DIR_WRITE ? "WR" : "RD",
			chan->id, ret);
		chan->desc_buf.sz = 0;
		return ret;
	}

	dev_vdbg(dev, "[CHAN][%s%d] desc_buf alloc: vaddr=%p paddr=%pad sz=%zu buddy=%d\n",
		 chan->dir == EDMA_DIR_WRITE ? "WR" : "RD",
		 chan->id, chan->desc_buf.vaddr, &chan->desc_buf.paddr,
		 chan->desc_buf.sz, chan->desc_buf_is_buddy);
	return 0;
}
EXPORT_SYMBOL_GPL(dx_dma_chan_alloc_desc_buf);

void dx_dma_chan_free_desc_buf(struct dw_edma_chan *chan)
{
	if (!chan || !chan->chip)
		return;

	if (!chan->desc_buf.vaddr)
		return;

	dw_edma_free_dma_mem(chan->chip->dev, &chan->desc_buf,
			     chan->desc_buf_is_buddy);
	chan->desc_buf.vaddr = NULL;
	chan->desc_buf.paddr = 0;
	chan->desc_buf.sz = 0;
	chan->desc_buf_is_buddy = false;
}
EXPORT_SYMBOL_GPL(dx_dma_chan_free_desc_buf);

int dw_edma_mem_init(struct dw_edma *dw)
{
	struct device *dev = &dw->pdev->dev;
	int i;
	/* Initialize Global Memory Pools */
	spin_lock_init(&dw->pool_lock);
	spin_lock_init(&dw->cleanup_lock);
	INIT_LIST_HEAD(&dw->cleanup_chunks);
	INIT_WORK(&dw->cleanup_work, dw_edma_cleanup_work);

	/*
	 * Step 3: per-channel desc_buf (1MB × 6 user channels = 6MB) replaces
	 * the former 8-slot global chunk pool.  Channel-level allocation is
	 * performed by dx_dma_chan_alloc_desc_buf() during channel setup; see
	 * dw-edma-core.c.  No global chunk pool is needed.
	 */

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
		vfree(dw->burst_pool);
		vfree(dw->burst_free_list);
		return -ENOMEM;
	}

	return 0;
}

void dw_edma_mem_deinit(struct dw_edma *dw)
{
	struct device *dev = &dw->pdev->dev;

	dev_vdbg(dev, "[MEM][DEINIT] De-initializing memory pools\n");
	cancel_work_sync(&dw->cleanup_work);
	dw_edma_cleanup_work(&dw->cleanup_work);

	/*
	 * Step 3: chunk pool removed.  Per-channel desc_buf is freed via
	 * dx_dma_chan_free_desc_buf() in dx_dma_remove() / probe error path.
	 */

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
