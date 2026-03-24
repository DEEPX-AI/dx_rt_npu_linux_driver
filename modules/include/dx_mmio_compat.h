// SPDX-License-Identifier: GPL-2.0
/*
 * MMIO accessors
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#ifndef __DX_MMIO_COMPAT_H
#define __DX_MMIO_COMPAT_H

#include <linux/io.h>
#include <linux/stddef.h>
#include <linux/types.h>

/**
 * dx_memcpy_fromio32 - MMIO read using 32-bit access only
 * @dest: kernel buffer (destination)
 * @src:  MMIO address  (source, PCIe BAR)
 * @size: number of bytes to copy
 */
static inline void dx_memcpy_fromio32(void *dest,
                                      const volatile void __iomem *src,
                                      size_t size)
{
	u8 *d = dest;

	while (size >= 4 && IS_ALIGNED((uintptr_t)src, 4)) {
		*(u32 *)d = readl(src);
		d    += 4;
		src  += 4;
		size -= 4;
	}
	while (size > 0) {
		*d = readb(src);
		d++;
		src++;
		size--;
	}
}

/**
 * dx_memcpy_toio32 - MMIO write using 32-bit access only
 * @dest: MMIO address  (destination, PCIe BAR)
 * @src:  kernel buffer (source)
 * @size: number of bytes to copy
 */
static inline void dx_memcpy_toio32(volatile void __iomem *dest,
                                       const void *src, size_t size)
{
	const u8 *s = src;

	while (size >= 4 && IS_ALIGNED((uintptr_t)dest, 4)) {
		writel(*(const u32 *)s, dest);
		s    += 4;
		dest += 4;
		size -= 4;
	}
	while (size > 0) {
		writeb(*s, dest);
		s++;
		dest++;
		size--;
	}
}

/**
 * dx_memset_io32 - MMIO memset using 32-bit access only
 * @dest:  MMIO address (PCIe BAR)
 * @value: byte value to fill
 * @size:  number of bytes to set
 */
static inline void dx_memset_io32(volatile void __iomem *dest,
                                    int value, size_t size)
{
	u32 val32 = (u8)value;

	val32 |= val32 << 8;
	val32 |= val32 << 16;

	while (size >= 4 && IS_ALIGNED((uintptr_t)dest, 4)) {
		writel(val32, dest);
		dest += 4;
		size -= 4;
	}
	while (size > 0) {
		writeb((u8)value, dest);
		dest++;
		size--;
	}
}

/** Address of a struct field within an MMIO-mapped structure */
#define DX_FIELD_ADDR(base, type, field) \
	((void __iomem *)(base) + offsetof(type, field))

/** 32-bit field read/write */
#define dx_read32(base, type, field) \
	readl(DX_FIELD_ADDR(base, type, field))

#define dx_write32(base, type, field, val) \
	writel((val), DX_FIELD_ADDR(base, type, field))

/** 8-bit field read/write */
#define dx_read8(base, type, field) \
	readb(DX_FIELD_ADDR(base, type, field))

#define dx_write8(base, type, field, val) \
	writeb((val), DX_FIELD_ADDR(base, type, field))

#endif /* __DX_MMIO_COMPAT_H */
