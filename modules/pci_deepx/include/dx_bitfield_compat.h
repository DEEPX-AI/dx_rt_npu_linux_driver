/* SPDX-License-Identifier: GPL-2.0 */
/*
 * FIELD_PREP / FIELD_GET for kernels without include/linux/bitfield.h (before ~5.1).
 */
#ifndef __DX_BITFIELD_COMPAT_H
#define __DX_BITFIELD_COMPAT_H

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
#include <linux/bitfield.h>
#else
/* 4.x has neither linux/bitfield.h nor linux/bits.h; provide FIELD_* locally. */

#ifndef __bf_shf
#define __bf_shf(x) (__builtin_ffsll(x) - 1)
#endif

#ifndef FIELD_PREP
#define FIELD_PREP(_mask, _val)						\
	({								\
		((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask);	\
	})
#endif

#ifndef FIELD_GET
#define FIELD_GET(_mask, _reg)						\
	({								\
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask));	\
	})
#endif

#endif /* LINUX_VERSION_CODE < 5.1 */

#endif /* __DX_BITFIELD_COMPAT_H */
