/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sysfs_emit() landed in mainline 5.10.  On older kernels such as 4.4, sysfs
 * show() handlers wrote into the PAGE_SIZE buffer directly via scnprintf().
 * Provide an equivalent so the driver can use sysfs_emit() unconditionally.
 */
#ifndef DX_SYSFS_COMPAT_H
#define DX_SYSFS_COMPAT_H

#include <linux/version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))

#include <linux/kernel.h>
#include <linux/mm.h>		/* PAGE_SIZE */

#ifndef sysfs_emit
#define sysfs_emit(buf, fmt, ...) \
	scnprintf(buf, PAGE_SIZE, fmt, ##__VA_ARGS__)
#endif

#endif /* < 5.10 */

#endif /* DX_SYSFS_COMPAT_H */
