/* SPDX-License-Identifier: GPL-2.0 */
/*
 * __poll_t and the kernel-side EPOLL* constants were introduced in mainline
 * 4.16.  On older kernels such as 4.4 the .poll fop returns a plain
 * "unsigned int" and uses the POLL* names instead.  Map them so the driver can
 * use the modern spelling unconditionally.
 */
#ifndef DX_POLL_COMPAT_H
#define DX_POLL_COMPAT_H

#include <linux/version.h>
#include <linux/poll.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))

typedef unsigned int __poll_t;

#ifndef EPOLLIN
#define EPOLLIN		POLLIN
#endif
#ifndef EPOLLRDNORM
#define EPOLLRDNORM	POLLRDNORM
#endif
#ifndef EPOLLERR
#define EPOLLERR	POLLERR
#endif

#endif /* < 4.16 */

#endif /* DX_POLL_COMPAT_H */
