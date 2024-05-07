// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#include <linux/device.h>
#include <linux/dmaengine.h>
#include <linux/module.h>
#include <linux/spinlock.h>

#include "virt-dma.h"

static struct virt_dma_desc *to_virt_desc(struct dma_async_tx_descriptor *tx)
{
	return container_of(tx, struct virt_dma_desc, tx);
}

dma_cookie_t vchan_tx_submit(struct dma_async_tx_descriptor *tx)
{
	struct virt_dma_chan *vc = to_virt_chan(tx->chan);
	struct virt_dma_desc *vd = to_virt_desc(tx);
	unsigned long flags;
	dma_cookie_t cookie;

	spin_lock_irqsave(&vc->lock, flags);
	cookie = dma_cookie_assign(tx);

	list_move_tail(&vd->node, &vc->desc_submitted);
	spin_unlock_irqrestore(&vc->lock, flags);

	dev_dbg(vc->chan.device->dev, "vchan %p: txd %p[%x]: submitted\n",
		vc, vd, cookie);

	return cookie;
}
EXPORT_SYMBOL_GPL(vchan_tx_submit);

/**
 * vchan_tx_desc_free - free a reusable descriptor
 * @tx: the transfer
 *
 * This function frees a previously allocated reusable descriptor. The only
 * other way is to clear the DMA_CTRL_REUSE flag and submit one last time the
 * transfer.
 *
 * Returns 0 upon success
 */
int vchan_tx_desc_free(struct dma_async_tx_descriptor *tx)
{
	struct virt_dma_chan *vc = to_virt_chan(tx->chan);
	struct virt_dma_desc *vd = to_virt_desc(tx);
	unsigned long flags;

	spin_lock_irqsave(&vc->lock, flags);
	list_del(&vd->node);
	spin_unlock_irqrestore(&vc->lock, flags);

	dev_dbg(vc->chan.device->dev, "vchan %p: txd %p[%x]: freeing\n",
		vc, vd, vd->tx.cookie);
	vc->desc_free(vd);
	return 0;
}
EXPORT_SYMBOL_GPL(vchan_tx_desc_free);

struct virt_dma_desc *vchan_find_desc(struct virt_dma_chan *vc,
	dma_cookie_t cookie)
{
	struct virt_dma_desc *vd;

	list_for_each_entry(vd, &vc->desc_issued, node)
		if (vd->tx.cookie == cookie)
			return vd;

	return NULL;
}
EXPORT_SYMBOL_GPL(vchan_find_desc);

/*
 * This tasklet handles the completion of a DMA descriptor by
 * calling its callback and freeing it.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static void vchan_complete(struct tasklet_struct *t)
{
	struct virt_dma_chan *vc = from_tasklet(vc, t, task);
#else
static void vchan_complete(unsigned long arg)
{
	struct virt_dma_chan *vc = (struct virt_dma_chan *)arg;
#endif
	struct virt_dma_desc *vd;
	struct dmaengine_desc_callback cb;
	LIST_HEAD(head);

	spin_lock_irq(&vc->lock);
	list_splice_tail_init(&vc->desc_completed, &head);
	vd = vc->cyclic;
	if (vd) {
		vc->cyclic = NULL;
		dmaengine_desc_get_callback(&vd->tx, &cb);
	} else {
		memset(&cb, 0, sizeof(cb));
	}
	spin_unlock_irq(&vc->lock);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	{
		struct virt_dma_desc *_vd;
		dmaengine_desc_callback_invoke(&cb, &vd->tx_result);

		list_for_each_entry_safe(vd, _vd, &head, node) {
			dmaengine_desc_get_callback(&vd->tx, &cb);

			list_del(&vd->node);
			dmaengine_desc_callback_invoke(&cb, &vd->tx_result);
			vchan_vdesc_fini(vd);
		}
	}
#else
	dmaengine_desc_callback_invoke(&cb, NULL);

	while (!list_empty(&head)) {
		vd = list_first_entry(&head, struct virt_dma_desc, node);
		dmaengine_desc_get_callback(&vd->tx, &cb);

		list_del(&vd->node);
		if (dmaengine_desc_test_reuse(&vd->tx))
			list_add(&vd->node, &vc->desc_allocated);
		else
			vc->desc_free(vd);

		dmaengine_desc_callback_invoke(&cb, NULL);
	}
#endif
}

void vchan_dma_desc_free_list(struct virt_dma_chan *vc, struct list_head *head)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct virt_dma_desc *vd, *_vd;

	list_for_each_entry_safe(vd, _vd, head, node) {
		list_del(&vd->node);
		vchan_vdesc_fini(vd);
	}
#else
	while (!list_empty(head)) {
		struct virt_dma_desc *vd = list_first_entry(head,
			struct virt_dma_desc, node);
		if (dmaengine_desc_test_reuse(&vd->tx)) {
			list_move_tail(&vd->node, &vc->desc_allocated);
		} else {
			dev_dbg(vc->chan.device->dev, "txd %p: freeing\n", vd);
			list_del(&vd->node);
			vc->desc_free(vd);
		}
	}
#endif
}
EXPORT_SYMBOL_GPL(vchan_dma_desc_free_list);

void vchan_init(struct virt_dma_chan *vc, struct dma_device *dmadev)
{
	dma_cookie_init(&vc->chan);

	spin_lock_init(&vc->lock);
	INIT_LIST_HEAD(&vc->desc_allocated);
	INIT_LIST_HEAD(&vc->desc_submitted);
	INIT_LIST_HEAD(&vc->desc_issued);
	INIT_LIST_HEAD(&vc->desc_completed);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0))
	INIT_LIST_HEAD(&vc->desc_terminated);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
	tasklet_setup(&vc->task, vchan_complete);
#else
	tasklet_init(&vc->task, vchan_complete, (unsigned long)vc);
#endif
	vc->chan.device = dmadev;
	list_add_tail(&vc->chan.device_node, &dmadev->channels);
}
EXPORT_SYMBOL_GPL(vchan_init);

MODULE_AUTHOR("Russell King");
MODULE_LICENSE("GPL");
