/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __DX_DMA_SYSFS_H
#define __DX_DMA_SYSFS_H

struct pci_driver;
struct pci_dev;

void dx_dma_sysfs_create(struct pci_driver *drv);
void dx_dma_sysfs_remove(struct pci_driver *drv);
int dx_dma_sysfs_device_create(struct pci_dev *pdev);
void dx_dma_sysfs_device_remove(struct pci_dev *pdev);
void dx_dma_sysfs_device_release(struct pci_dev *pdev);

#endif /* __DX_DMA_SYSFS_H */
