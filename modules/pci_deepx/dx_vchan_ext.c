// SPDX-License-Identifier: GPL-2.0
/*
 * DEEPX MODIFIED: vchan_tx_desc_free extension for kernel 4.4.
 *
 * When CONFIG_DMA_VIRTUAL_CHANNELS=y the kernel provides its own virt-dma.c
 * (compiled into vmlinux) so the driver's virt-dma.c is NOT compiled.
 * However, the kernel 4.4 built-in virt-dma.c does not include
 * vchan_tx_desc_free (it was added in a later mainline release), so
 * the symbol is missing for this out-of-tree module.
 *
 * Provide the exact same implementation here, compiled into dx_dma.ko
 * only when CONFIG_DMA_VIRTUAL_CHANNELS=y (see Kbuild).
 */

#include "include/virt-dma.h"

/* to_virt_desc is a file-local helper in virt-dma.c; replicate it here. */
static inline struct virt_dma_desc *to_virt_desc(struct dma_async_tx_descriptor *tx)
{
	return container_of(tx, struct virt_dma_desc, tx);
}

/**
 * vchan_tx_desc_free - free a reusable descriptor
 * @tx: the transfer
 *
 * This function frees a previously allocated reusable descriptor. The only
 * other way is to clear the DMA_CTRL_REUSE flag and submit one last time the
 * transfer.
 *
 * Returns 1 upon success (matches the mainline implementation).
 */
int vchan_tx_desc_free(struct dma_async_tx_descriptor *tx)
{
	struct virt_dma_chan *vc = to_virt_chan(tx->chan);
	struct virt_dma_desc *vd = to_virt_desc(tx);
	unsigned long flags;

	spin_lock_irqsave(&vc->lock, flags);
	list_del(&vd->node);
	spin_unlock_irqrestore(&vc->lock, flags);

	dev_dbg(vc->chan.device->dev, "vchan %p: free vdesc %p\n", vc, vd);
	vc->desc_free(vd);
	return 1;
}
EXPORT_SYMBOL_GPL(vchan_tx_desc_free);
