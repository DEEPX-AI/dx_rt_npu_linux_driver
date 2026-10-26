/* SPDX-License-Identifier: GPL-2.0 */
/*
 * FOLL_PIN (pin_user_pages_fast / unpin_user_pages_dirty_lock) landed in
 * mainline 5.6.  It is the API the kernel mandates for pages handed to a DMA
 * engine: unlike the plain refcount taken by get_user_pages_fast(), a FOLL_PIN
 * reference makes page_maybe_dma_pinned() true, so writeback, page migration
 * and fork/COW know the page is under device DMA and handle it correctly.
 *
 * On older kernels fall back to the get_user_pages_fast() sequence, which is
 * the best that can be done there.
 */
#ifndef DX_GUP_COMPAT_H
#define DX_GUP_COMPAT_H

#include <linux/version.h>
#include <linux/mm.h>
#include <linux/pagemap.h>

/* FOLL_LONGTERM only exists from 5.0. */
#ifndef FOLL_LONGTERM
#define FOLL_LONGTERM 0
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0))

/* Mirrors the *_fast signature (int nr_pages / int return), not the plain
 * pin_user_pages() one, which takes unsigned long and returns long. */
static inline int dx_pin_user_pages_fast(unsigned long start, int nr_pages,
					 unsigned int gup_flags,
					 struct page **pages)
{
	/* get_user_pages_fast() rejects FOLL_LONGTERM before 5.6; the callers
	 * only lose the CMA/ZONE_MOVABLE migration hint, not correctness. */
	gup_flags &= ~FOLL_LONGTERM;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))
	/* Third argument was a plain "int write" before 4.20. */
	return get_user_pages_fast(start, nr_pages,
				   !!(gup_flags & FOLL_WRITE), pages);
#else
	return get_user_pages_fast(start, nr_pages, gup_flags, pages);
#endif
}

static inline void dx_unpin_user_pages_dirty_lock(struct page **pages,
						  unsigned long npages,
						  bool make_dirty)
{
	unsigned long i;

	for (i = 0; i < npages; i++) {
		struct page *page = pages[i];

		/* Callers pass the count get_user_pages_fast() returned, so the
		 * array has no holes; skip rather than break so a violated
		 * assumption cannot strand the remaining references. */
		if (!page)
			continue;
		/* set_page_dirty_lock() unconditionally takes lock_page(), which can
		 * block on khugepaged/compaction/reclaim; upstream skips it when the
		 * page is already dirty. */
		if (make_dirty && !PageDirty(page))
			set_page_dirty_lock(page);
		put_page(page);
	}
}

#else /* >= 5.6 */

#define dx_pin_user_pages_fast		pin_user_pages_fast
#define dx_unpin_user_pages_dirty_lock	unpin_user_pages_dirty_lock

#endif

#endif /* DX_GUP_COMPAT_H */
