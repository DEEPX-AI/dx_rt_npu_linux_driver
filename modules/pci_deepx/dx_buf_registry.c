// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Persistent user-buffer registry — see include/dx_buf_registry.h.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/version.h>
#include <linux/sched.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/signal.h>	/* split out of linux/sched.h in 4.11 */
#include <linux/sched/mm.h>
#endif
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>

#include "dx_buf_registry.h"
#include "dx_util.h"
#include "dx_gup_compat.h"
#include "dx_mm_compat.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(KVM_KERNEL_MAJ, KVM_KERNEL_MIN, KVM_KERNEL_PAT))
#include <linux/vmalloc.h>
#define dx_reg_page_array_alloc(n) \
	kvmalloc_array((n), sizeof(struct page *), GFP_KERNEL)
#define dx_reg_page_array_free(p)	kvfree(p)
#else
#define dx_reg_page_array_alloc(n) \
	kcalloc((n), sizeof(struct page *), GFP_KERNEL)
#define dx_reg_page_array_free(p)	kfree(p)
#endif

struct dx_reg_buf {
	struct list_head	node;
	void			*owner;
	/* Held with mmgrab(): the fd may outlive the registering process (fork,
	 * SCM_RIGHTS), and teardown still has to uncharge mm->pinned_vm. */
	struct mm_struct	*mm;
	struct device		*dma_dev;
	unsigned long		va;
	size_t			len;
	int			dev_id;
	bool			write;
	unsigned int		pages_nr;
	struct page		**pages;
	struct sg_table		sgt;	/* nents: post-map, orig_nents: page count */
	/* Identity witness: if the user frees the buffer and the VA is reused,
	 * the first page no longer resolves to this one. */
	struct page		*first_page;
	atomic_t		inflight;
};

/*
 * A spinlock, not a mutex: this is taken on every transfer from several
 * channels at once.  A contended mutex sleeps, and a sleep here costs a full
 * scheduler wakeup — on a host with deep C-states that is milliseconds, which
 * is far more than the whole DMA it is guarding.
 */
static LIST_HEAD(dx_reg_list);
static DEFINE_SPINLOCK(dx_reg_lock);

/* Well above any real tensor; RLIMIT_MEMLOCK bounds the per-process total. */
#define DX_REG_MAX_LEN		(1UL << 30)

/*
 * RLIMIT_MEMLOCK caps pages per process, not entries and not the system total.
 * Two things still need bounding: dx_reg_find() walks the list under a lock
 * taken on every transfer, so one process registering thousands of small
 * buffers would slow down everyone else's DMA; and FOLL_LONGTERM pins block
 * compaction, so the host-wide footprint must not be left to userspace.
 */
#define DX_REG_MAX_PER_OWNER	256

static unsigned long dx_reg_max_pinned_mb = 1024;
module_param(dx_reg_max_pinned_mb, ulong, 0644);
MODULE_PARM_DESC(dx_reg_max_pinned_mb,
		 "system-wide cap on registered (long-term pinned) memory, in MiB");

static unsigned long dx_reg_pinned_pages;	/* under dx_reg_lock */

static void dx_reg_memlock_uncharge(struct mm_struct *mm, unsigned long npages)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
	atomic64_sub(npages, &mm->pinned_vm);
#else
	down_write(&mm->mmap_sem);
	mm->pinned_vm -= npages;
	up_write(&mm->mmap_sem);
#endif
}

static int dx_reg_memlock_charge(struct mm_struct *mm, unsigned long npages)
{
	unsigned long limit = rlimit(RLIMIT_MEMLOCK) >> PAGE_SHIFT;
	unsigned long cur;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
	cur = atomic64_add_return(npages, &mm->pinned_vm);
#else
	down_write(&mm->mmap_sem);
	mm->pinned_vm += npages;
	cur = mm->pinned_vm;
	up_write(&mm->mmap_sem);
#endif
	if (cur > limit && !capable(CAP_IPC_LOCK)) {
		dx_reg_memlock_uncharge(mm, npages);
		pr_err("dx_buf: RLIMIT_MEMLOCK exceeded (%lu > %lu pages)\n",
		       cur, limit);
		return -ENOMEM;
	}
	return 0;
}

/* Caller holds dx_reg_lock. */
static struct dx_reg_buf *dx_reg_find(struct mm_struct *mm, int dev_id,
				      unsigned long va, bool write)
{
	struct dx_reg_buf *reg;

	list_for_each_entry(reg, &dx_reg_list, node) {
		if (reg->mm == mm && reg->dev_id == dev_id &&
		    reg->va == va && reg->write == write)
			return reg;
	}
	return NULL;
}

/* Caller holds dx_reg_lock and has ensured inflight == 0. Only unlinks; the
 * teardown itself sleeps (unpin takes lock_page) so it must run unlocked. */
static void dx_reg_unlink(struct dx_reg_buf *reg)
{
	list_del(&reg->node);
	dx_reg_pinned_pages -= reg->pages_nr;
}

/* Caller holds dx_reg_lock. */
static int dx_reg_check_limits(void *owner, unsigned int pages_nr)
{
	unsigned long max_pages = dx_reg_max_pinned_mb << (20 - PAGE_SHIFT);
	struct dx_reg_buf *reg;
	unsigned int owned = 0;

	if (dx_reg_pinned_pages + pages_nr > max_pages) {
		pr_err("dx_buf: registry full (%lu + %u > %lu pages); raise dx_reg_max_pinned_mb\n",
		       dx_reg_pinned_pages, pages_nr, max_pages);
		return -ENOMEM;
	}

	list_for_each_entry(reg, &dx_reg_list, node) {
		if (reg->owner == owner && ++owned >= DX_REG_MAX_PER_OWNER) {
			pr_err("dx_buf: owner %p already holds %u registrations\n",
			       owner, owned);
			return -ENOSPC;
		}
	}
	return 0;
}

static void dx_reg_free(struct dx_reg_buf *reg)
{
	dma_unmap_sg(reg->dma_dev, reg->sgt.sgl, reg->sgt.orig_nents,
		     reg->write ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
	sg_free_table(&reg->sgt);

	/* C2H buffers were written by the device, so they must be re-dirtied. */
	dx_unpin_user_pages_dirty_lock(reg->pages, reg->pages_nr, !reg->write);
	dx_reg_page_array_free(reg->pages);

	dx_reg_memlock_uncharge(reg->mm, reg->pages_nr);
	mmdrop(reg->mm);
	kfree(reg);
}

int dx_buf_registry_add(struct device *dma_dev, int dev_id, void __user *va,
			size_t len, bool write, void *owner)
{
	unsigned long start = (unsigned long)va;
	struct dx_reg_buf *reg;
	struct scatterlist *sg;
	unsigned int pages_nr;
	size_t remaining = len;
	unsigned long cursor = start;
	int i, rv, mapped;

	if (!dma_dev || !va || !len || !owner || !current->mm)
		return -EINVAL;

	/* @len comes straight from userspace: reject a wrapping range and cap the
	 * page count so it always fits the int nr_pages of pin_user_pages_fast(). */
	if (len > DX_REG_MAX_LEN || len > ULONG_MAX - start)
		return -EINVAL;

	pages_nr = (PAGE_ALIGN(start + len) - (start & PAGE_MASK)) >> PAGE_SHIFT;
	if (!pages_nr)
		return -EINVAL;

	reg = kzalloc(sizeof(*reg), GFP_KERNEL);
	if (!reg)
		return -ENOMEM;

	reg->pages = dx_reg_page_array_alloc(pages_nr);
	if (!reg->pages) {
		kfree(reg);
		return -ENOMEM;
	}

	rv = dx_reg_memlock_charge(current->mm, pages_nr);
	if (rv)
		goto err_pages;

	/* FOLL_LONGTERM: the pin outlives this syscall, so movable/CMA pages
	 * must be migrated out now rather than blocking compaction forever.
	 *
	 * FOLL_WRITE is used for both directions, unlike the per-transfer path
	 * which only sets it for device-to-host. A buffer is normally registered
	 * right after allocation, before userspace has written a single byte, so
	 * a read-only fault on an untouched anonymous page resolves to the shared
	 * zero page. The pin would then latch that zero page while the first
	 * userspace write silently COWs the mapping onto fresh pages, leaving the
	 * cached sg_table pointing at memory the process no longer uses. Faulting
	 * the pages in writable here materialises the final pages up front, which
	 * is exactly the setup cost this registry exists to hoist out of the hot
	 * path. Pinning host-to-card buffers writable is harmless: the device
	 * only reads them.
	 */
	rv = dx_pin_user_pages_fast(start, pages_nr, FOLL_WRITE | FOLL_LONGTERM,
				    reg->pages);
	if (rv < 0) {
		pr_err("dx_buf: pin failed for %u pages, %d\n", pages_nr, rv);
		goto err_memlock;
	}
	if (rv != pages_nr) {
		pr_err("dx_buf: pinned only %d of %u pages\n", rv, pages_nr);
		reg->pages_nr = rv;
		rv = -EFAULT;
		goto err_pin;
	}
	reg->pages_nr = pages_nr;

	if (sg_alloc_table(&reg->sgt, pages_nr, GFP_KERNEL)) {
		rv = -ENOMEM;
		goto err_pin;
	}

	sg = reg->sgt.sgl;
	for (i = 0; i < pages_nr; i++, sg = sg_next(sg)) {
		unsigned int offset = offset_in_page(cursor);
		unsigned int nbytes =
			min_t(unsigned int, PAGE_SIZE - offset, remaining);

		flush_dcache_page(reg->pages[i]);
		sg_set_page(sg, reg->pages[i], nbytes, offset);
		cursor += nbytes;
		remaining -= nbytes;
	}
	if (remaining) {
		pr_err("dx_buf: length %zu does not match %u pages\n",
		       len, pages_nr);
		rv = -EINVAL;
		goto err_sgt;
	}

	mapped = dma_map_sg(dma_dev, reg->sgt.sgl, reg->sgt.orig_nents,
			    write ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
	if (!mapped) {
		pr_err("dx_buf: dma_map_sg failed\n");
		rv = -ENOMEM;
		goto err_sgt;
	}
	reg->sgt.nents = mapped;

	reg->owner	= owner;
	reg->mm		= current->mm;
	reg->dma_dev	= dma_dev;
	reg->va		= start;
	reg->len	= len;
	reg->dev_id	= dev_id;
	reg->write	= write;
	reg->first_page	= reg->pages[0];

	spin_lock(&dx_reg_lock);
	if (dx_reg_find(current->mm, dev_id, start, write)) {
		spin_unlock(&dx_reg_lock);
		rv = -EEXIST;
		goto err_map;
	}
	rv = dx_reg_check_limits(owner, pages_nr);
	if (rv) {
		spin_unlock(&dx_reg_lock);
		goto err_map;
	}
	mmgrab(reg->mm);
	list_add_tail(&reg->node, &dx_reg_list);
	dx_reg_pinned_pages += pages_nr;
	spin_unlock(&dx_reg_lock);

	pr_debug("dx_buf: dev %d registered %s va 0x%lx len %zu (%u pages -> %u sg) mm %p\n",
		 dev_id, write ? "H2C" : "C2H", start, len, pages_nr, mapped,
		 current->mm);
	return 0;

err_map:
	dma_unmap_sg(dma_dev, reg->sgt.sgl, reg->sgt.orig_nents,
		     write ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
err_sgt:
	sg_free_table(&reg->sgt);
err_pin:
	if (reg->pages_nr)
		dx_unpin_user_pages_dirty_lock(reg->pages, reg->pages_nr, false);
err_memlock:
	dx_reg_memlock_uncharge(current->mm, pages_nr);
err_pages:
	dx_reg_page_array_free(reg->pages);
	kfree(reg);
	return rv;
}

int dx_buf_registry_del(int dev_id, void __user *va, bool write, void *owner)
{
	struct dx_reg_buf *reg;

	if (!current->mm)
		return -EINVAL;

	spin_lock(&dx_reg_lock);
	reg = dx_reg_find(current->mm, dev_id, (unsigned long)va, write);
	if (!reg || reg->owner != owner) {
		spin_unlock(&dx_reg_lock);
		return -ENOENT;
	}
	if (atomic_read(&reg->inflight)) {
		spin_unlock(&dx_reg_lock);
		pr_err("dx_buf: unregister of va 0x%lx while a transfer is in flight\n",
		       (unsigned long)va);
		return -EBUSY;
	}
	dx_reg_unlink(reg);
	spin_unlock(&dx_reg_lock);

	dx_reg_free(reg);
	return 0;
}

/* @owner NULL matches any owner, @dev_id < 0 matches any device. */
static void dx_reg_purge(void *owner, int dev_id)
{
	struct dx_reg_buf *reg, *tmp;
	unsigned int retries = 0;
	LIST_HEAD(doomed);
	bool busy;

	do {
		busy = false;
		spin_lock(&dx_reg_lock);
		list_for_each_entry_safe(reg, tmp, &dx_reg_list, node) {
			if (owner && reg->owner != owner)
				continue;
			if (dev_id >= 0 && reg->dev_id != dev_id)
				continue;
			if (atomic_read(&reg->inflight)) {
				busy = true;
				continue;
			}
			dx_reg_unlink(reg);
			list_add_tail(&reg->node, &doomed);
		}
		spin_unlock(&dx_reg_lock);

		list_for_each_entry_safe(reg, tmp, &doomed, node) {
			list_del(&reg->node);
			dx_reg_free(reg);
		}

		if (!busy)
			break;
		/* Transfers are bounded by the DMA timeout, so this drains. */
		msleep(20);
	} while (++retries < 500);

	if (busy)
		pr_err("dx_buf: owner %p dev %d still has in-flight buffers after 10s; leaking them rather than freeing pages under DMA\n",
		       owner, dev_id);
}

void dx_buf_registry_del_owner(void *owner)
{
	if (owner)
		dx_reg_purge(owner, -1);
}

void dx_buf_registry_del_device(int dev_id)
{
	if (dev_id >= 0)
		dx_reg_purge(NULL, dev_id);
}

int dx_buf_registry_acquire(int dev_id, const void __user *va, size_t len,
			    bool write, struct sg_table *sgt_out,
			    struct dx_reg_buf **reg_out)
{
	unsigned long start = (unsigned long)va;
	struct dx_reg_buf *reg;
	struct page *probe = NULL;
	int rv;

	if (!current->mm)
		return -ENOENT;

	spin_lock(&dx_reg_lock);
	reg = dx_reg_find(current->mm, dev_id, start, write);
	if (!reg || reg->len != len) {
		spin_unlock(&dx_reg_lock);
		pr_debug("dx_buf: MISS dev %d %s va 0x%lx len %zu mm %p (%s)\n",
			 dev_id, write ? "H2C" : "C2H", start, len, current->mm,
			 reg ? "len mismatch" : "no entry");
		return -ENOENT;
	}
	atomic_inc(&reg->inflight);
	*sgt_out = reg->sgt;
	spin_unlock(&dx_reg_lock);

	/*
	 * The kernel is not told when userspace free()s a registered buffer, so
	 * verify the VA still resolves to the pages we pinned.  One fast-path
	 * GUP on a single page; sub-microsecond against the ~400us being saved.
	 */
	rv = get_user_pages_fast(start, 1, 0, &probe);
	if (rv != 1) {
		pr_err("dx_buf: registered va 0x%lx is no longer mapped (%d) — buffer was freed without unregistering\n",
		       start, rv);
		rv = -EFAULT;
		goto err;
	}
	if (probe != reg->first_page) {
		put_page(probe);
		pr_err("dx_buf: registered va 0x%lx now resolves to a different page — buffer was freed without unregistering\n",
		       start);
		rv = -EFAULT;
		goto err;
	}
	put_page(probe);

	*reg_out = reg;
	return 0;

err:
	dx_buf_registry_release(reg);
	return rv;
}

/* Lock-free: the caller owns an in-flight count, so the entry cannot go away. */
void dx_buf_registry_release(struct dx_reg_buf *reg)
{
	if (reg)
		atomic_dec(&reg->inflight);
}
