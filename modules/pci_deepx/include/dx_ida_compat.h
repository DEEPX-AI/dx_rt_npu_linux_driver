/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ida_alloc_max() / ida_free() (mainline 4.19) for older kernels such as 4.4.
 * These wrap the legacy ida_simple_get()/ida_simple_remove() helpers.
 */
#ifndef DX_IDA_COMPAT_H
#define DX_IDA_COMPAT_H

#include <linux/version.h>
#include <linux/idr.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))

#include <linux/gfp.h>

static inline int ida_alloc_max(struct ida *ida, unsigned int max, gfp_t gfp)
{
	/* ida_simple_get() upper bound is exclusive; max is inclusive. */
	return ida_simple_get(ida, 0, max + 1, gfp);
}

static inline void ida_free(struct ida *ida, unsigned int id)
{
	ida_simple_remove(ida, id);
}

#endif /* < 4.19 */

#endif /* DX_IDA_COMPAT_H */
