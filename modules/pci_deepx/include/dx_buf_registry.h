/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Persistent user-buffer registry.
 *
 * Normally every read()/write() pins the user pages, builds an sg_table and
 * calls dma_map_sg(), then tears all of it down again.  On a 2.3 MB C2H
 * transfer that is ~300-500us of the ~1.3ms total, dominated by IOMMU IOVA
 * allocation (a 576-page mapping is far above the IOVA rcache ceiling, so
 * every map takes the alloc_iova() rbtree slow path).
 *
 * A registered buffer pays that cost exactly once.  Every later transfer
 * reuses the cached sg_table and only performs cache maintenance.
 */

#ifndef DX_BUF_REGISTRY_H
#define DX_BUF_REGISTRY_H

#include <linux/types.h>
#include <linux/scatterlist.h>

struct device;
struct dx_reg_buf;

/**
 * dx_buf_registry_add - pin and DMA-map a user buffer for repeated use
 * @dma_dev:  device the transfers will be issued on (must match the one used
 *            by dma_map_sg() on the normal path, or the IOVA domain differs)
 * @dev_id:   DeepX device index
 * @va:       user virtual address
 * @len:      length in bytes
 * @write:    true for H2C (device reads), false for C2H (device writes)
 * @owner:    opaque per-fd token; every entry is torn down when its owner goes
 *
 * Return: 0, or -EEXIST / -EINVAL / -ENOMEM / -EFAULT, or -ENOSPC when @owner
 * has hit the per-fd registration limit.
 */
int dx_buf_registry_add(struct device *dma_dev, int dev_id, void __user *va,
			size_t len, bool write, void *owner);

int dx_buf_registry_del(int dev_id, void __user *va, bool write, void *owner);

/* Tear down every entry belonging to @owner. Waits for in-flight transfers. */
void dx_buf_registry_del_owner(void *owner);

/*
 * Tear down every entry on @dev_id. Must be called from the PCIe remove path
 * while the DMA device is still alive: entries cache a struct device * for
 * dma_unmap_sg(), which would dangle once the channels are torn down.
 */
void dx_buf_registry_del_device(int dev_id);

/**
 * dx_buf_registry_acquire - look up a registered buffer for one transfer
 * @sgt_out: receives a by-value copy of the cached table on success
 * @reg_out: receives the entry; must be passed to dx_buf_registry_release()
 *
 * On success the entry is marked in-flight so it cannot be unregistered
 * underneath the transfer.
 *
 * Return: 0 on hit, -ENOENT if not registered (caller falls back to the
 * per-transfer pin path), -EFAULT if registered but the mapping is stale
 * (user freed the buffer without unregistering it).
 */
int dx_buf_registry_acquire(int dev_id, const void __user *va, size_t len,
			    bool write, struct sg_table *sgt_out,
			    struct dx_reg_buf **reg_out);

void dx_buf_registry_release(struct dx_reg_buf *reg);

#endif /* DX_BUF_REGISTRY_H */
