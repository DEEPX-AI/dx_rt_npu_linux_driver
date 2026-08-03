/* SPDX-License-Identifier: GPL-2.0 */
/*
 * debugfs helpers for kernels before debugfs_create_file_unsafe() (4.7) and
 * DEFINE_DEBUGFS_ATTRIBUTE() (4.5).  Maps the modern API used by the driver
 * onto the legacy equivalents so the same source builds on 4.4 through 6.x.
 */
#ifndef __DX_DEBUGFS_COMPAT_H
#define __DX_DEBUGFS_COMPAT_H

#include <linux/version.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0)
/*
 * debugfs_create_file_unsafe() appeared in 4.7.  The "unsafe" variant only
 * skips the open-time file_operations proxy used for removal-race protection,
 * so the regular helper is a correct (slightly safer) fallback.
 */
#define debugfs_create_file_unsafe(name, mode, parent, data, fops) \
	debugfs_create_file((name), (mode), (parent), (data), (fops))
#endif

#ifndef DEFINE_DEBUGFS_ATTRIBUTE
/*
 * DEFINE_DEBUGFS_ATTRIBUTE() appeared in 4.5.  DEFINE_SIMPLE_ATTRIBUTE()
 * builds an equivalent file_operations for the regular debugfs_create_file()
 * path, with the same get/set/format signature.
 */
#define DEFINE_DEBUGFS_ATTRIBUTE(__fops, __get, __set, __fmt) \
	DEFINE_SIMPLE_ATTRIBUTE(__fops, __get, __set, __fmt)
#endif

#endif /* __DX_DEBUGFS_COMPAT_H */
