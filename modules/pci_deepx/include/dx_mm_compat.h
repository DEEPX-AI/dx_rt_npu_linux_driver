/* SPDX-License-Identifier: GPL-2.0 */
/*
 * kvmalloc()/kvzalloc()/kvmalloc_array() (mainline 4.12) for older kernels
 * such as 4.4.  These provide the kmalloc-then-vmalloc fallback used by the
 * driver's large per-channel allocations.  kvfree() is already backported in
 * the target 4.4 kernel, so it is intentionally not redefined here.
 */
#ifndef __DX_MM_COMPAT_H
#define __DX_MM_COMPAT_H

#include <linux/version.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
static inline void *kvmalloc(size_t size, gfp_t flags)
{
	void *p = kmalloc(size, flags | __GFP_NOWARN);

	if (!p)
		p = vmalloc(size);
	return p;
}

static inline void *kvzalloc(size_t size, gfp_t flags)
{
	void *p = kzalloc(size, flags | __GFP_NOWARN);

	if (!p)
		p = vzalloc(size);
	return p;
}

static inline void *kvmalloc_array(size_t n, size_t size, gfp_t flags)
{
	if (size != 0 && n > SIZE_MAX / size)
		return NULL;
	return kvmalloc(n * size, flags);
}
#endif /* < 4.12 */

/*
 * __GFP_RETRY_MAYFAIL (mainline 4.13) is the renamed successor of the older
 * __GFP_REPEAT flag.  Map it back so buddy-allocator fallbacks keep the same
 * "try harder but allow failure" semantics on kernel 4.4.
 */
#ifndef __GFP_RETRY_MAYFAIL
#define __GFP_RETRY_MAYFAIL __GFP_REPEAT
#endif

#endif /* __DX_MM_COMPAT_H */
