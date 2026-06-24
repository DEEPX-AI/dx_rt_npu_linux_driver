/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __DX_DMA_SYSFS_H
#define __DX_DMA_SYSFS_H

struct pci_driver;

void dx_dma_sysfs_create(struct pci_driver *drv);
void dx_dma_sysfs_remove(struct pci_driver *drv);

#endif /* __DX_DMA_SYSFS_H */
