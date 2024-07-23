// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */
#include <linux/mutex.h>
#include <linux/delay.h>

// #include "dx_cdev_ctrl.h"
#include "dw-edma-core.h"
#include "dx_lib.h"
#include "version.h"
#include "dx_util.h"
#include "dx_pcie_api.h"
#include "dw-edma-thread.h"
#include "dx_message.h"
#include "dx_cdev_ctrl.h"

#define MSGRAM_MSG_OFFSET_V2       (0x0)	/* Base address of message */
#define MSGRAM_REQUEST_OFFSET_V2   (0x1000)
#define MSGRAM_RESPONSE0_OFFSET_V2 (0x2000)
#define MSGRAM_RESPONSE1_OFFSET_V2 (0x3000)
#define MSGRAM_LOG_OFFSET_V2       (0x4000)
#define MSGRAM_DEBUG_OFFSET_V2     (0x8000)
#define MSGRAM_ERROR_OFFSET_V2     (0x9000)
#define MSGRAM_IRQ_OFFSET_V2       (0xA000)

#define MSGRAM_MSG_OFFSET_V3       (0x7000) /* Base address of message */
#define MSGRAM_REQUEST0_OFFSET_V3  (0x1000)
#define MSGRAM_REQUEST1_OFFSET_V3  (0x1A00)
#define MSGRAM_REQUEST2_OFFSET_V3  (0x2400)
#define MSGRAM_REQUEST3_OFFSET_V3  (0x2E00)
#define MSGRAM_RESPONSE0_OFFSET_V3 (0x3800)
#define MSGRAM_RESPONSE1_OFFSET_V3 (0x3A00)
#define MSGRAM_RESPONSE2_OFFSET_V3 (0x3C00)
#define MSGRAM_LOG_OFFSET_V3       (0x4000)
#define MSGRAM_DEBUG_OFFSET_V3     (0x8000)
#define MSGRAM_ERROR_OFFSET_V3     (0x8000)
#define MSGRAM_IRQ_OFFSET_V3       (0x8200)
#define MSGRAM_END_REGION_V3       (0x8F00)

typedef struct _message_ram_table {
	uint32_t base_offs;
	uint32_t req0_offs;
	uint32_t req1_offs;
	uint32_t req2_offs;
	uint32_t req3_offs;
	uint32_t resp0_offs;
	uint32_t resp1_offs;
	uint32_t resp2_offs;
	uint32_t log_offs;
	uint32_t debug_offs;
	uint32_t err_offs;
	uint32_t irq_offs;
} message_ram_table;
message_ram_table ep_ram_info;

#define RES_POOL_SIZE           (10) /* TODO */
static uint32_t resp_pool_header[MAX_DEV_NUM][USER_NUM_MAX];
static dx_pcie_response_list_t resp_pool[MAX_DEV_NUM][USER_NUM_MAX][RES_POOL_SIZE];

void __iomem *dx_pcie_get_message_area(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	void __iomem *reg = dw->npu_region[0].vaddr;
	return reg + ep_ram_info.base_offs;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_message_area);

void __iomem *dx_pcie_get_log_area(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	void __iomem *reg = dw->npu_region[0].vaddr;
	return reg + ep_ram_info.log_offs;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_log_area);

/* priority : high:10 / normal:0,1,2 (queue0,queue1...) */
void __iomem *dx_pcie_get_request_queue(u32 dev_id, u32 priority)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	void __iomem *reg = dw->npu_region[0].vaddr;
	if (dw->dx_ver == 2) {
		reg = reg + ep_ram_info.req0_offs;
	} else {
		switch (priority) {
			case DX_NORMAL_QUEUE0:
				reg = reg + ep_ram_info.req0_offs;
				break;
			case DX_NORMAL_QUEUE1:
				reg = reg + ep_ram_info.req1_offs;
				break;
			case DX_NORMAL_QUEUE2:
				reg = reg + ep_ram_info.req2_offs;
				break;
			case DX_HIGH_QUEUE:
				reg = reg + ep_ram_info.req3_offs;
				break;
			default:
				pr_err("%s:Priority is not defined(%d)\n", __func__, priority);
				reg = NULL;
				break;
		}
	}
	return reg;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_request_queue);

int dx_pcie_clear_response_queue(u32 dev_id)
{
	struct dx_pcie_msg *dx_msg = dx_dev_list_get(dev_id)->dx_msg;
	int i;

	for(i = 0; i < DX_PCIE_RESP_NUM; i++) {
		dx_pcie_response_list_t *entry, *tmp;
		spin_lock(&dx_msg->responses_lock[i]);
		list_for_each_entry_safe(entry, tmp, &dx_msg->responses[i].list, list) {
			list_del(&entry->list);
			kfree(entry);
		}
		spin_unlock(&dx_msg->responses_lock[i]);
	}
	return 0;
}
EXPORT_SYMBOL_GPL(dx_pcie_clear_response_queue);

int dx_pcie_is_response_queue_empty(u32 dev_id, int dma_ch)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;

	if(list_empty(&dx_msg->responses[dma_ch].list))
		return 1;
	else
		return 0;
}
EXPORT_SYMBOL_GPL(dx_pcie_is_response_queue_empty);

void dx_pcie_enqueue_response(u32 dev_id, int dma_ch)
{
	unsigned long flags;
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	dx_pcie_response_list_t *entry;
	uint32_t *header = &resp_pool_header[dev_id][dma_ch];

	spin_lock_irqsave(&dx_msg->responses_lock[dma_ch], flags);
	writel(1, ((void*)dx_msg->response[dma_ch]+0x100));
	(*header) %= RES_POOL_SIZE;
	entry = &resp_pool[dev_id][dma_ch][(*header)++];
	{
		memcpy_fromio(&entry->response, dx_msg->response[dma_ch], sizeof(dx_pcie_response_t));
		list_add_tail(&entry->list, &dx_msg->responses[dma_ch].list);
		dbg_msg("%s: dev_id %d, ch %d, %d, %d", __func__, dw->idx, dma_ch, entry->response.req_id, *header);
	}
	writel(0, ((void*)dx_msg->response[dma_ch]+0x100));
	spin_unlock_irqrestore(&dx_msg->responses_lock[dma_ch], flags);
}

int dx_pcie_dequeue_response(u32 dev_id, int dma_ch, dx_pcie_response_t* response)
{
	int ret;
	unsigned long flags;
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;

	spin_lock_irqsave(&dx_msg->responses_lock[dma_ch], flags);
	if(!list_empty(&dx_msg->responses[dma_ch].list))
	{
		dx_pcie_response_list_t *entry = list_first_entry(&dx_msg->responses[dma_ch].list, dx_pcie_response_list_t, list);
		dx_pcie_response_t *src = &entry->response;
		dbg_msg("%s: dev_id %d, ch %d, %d, sts:%d", __func__, dw->idx, dma_ch, src->req_id, src->status);
		memcpy(response, src, sizeof(dx_pcie_response_t));
		list_del(&entry->list);
		ret = 0;
	}
	else
	{
		dbg_msg("%s: dev_id %d, ch %d, empty", __func__, dw->idx, dma_ch);
		ret = -1;
	}
	spin_unlock_irqrestore(&dx_msg->responses_lock[dma_ch], flags);
	return ret;
}
EXPORT_SYMBOL_GPL(dx_pcie_dequeue_response);

void dx_pcie_enqueue_error_response(u32 dev_id, uint32_t err_code)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	unsigned long flags;

	spin_lock_irqsave(&dx_msg->err_lock, flags);
	if (err_code != 0) { /* Check point */
		dx_msg->err_response.err_code = err_code;
	} else {
		memcpy_fromio(&dx_msg->err_response, (dx_pcie_dev_err_t *)dx_msg->errors, sizeof(dx_pcie_dev_err_t));
		dbg_msg("%s:  dev_id %d, code:%d\n", __func__, dev_id, dx_msg->err_response.err_code);
		memset_io((dx_pcie_dev_err_t *)dx_msg->errors, 0x00, sizeof(dx_pcie_dev_err_t));
	}
	spin_unlock_irqrestore(&dx_msg->err_lock, flags);
}
EXPORT_SYMBOL_GPL(dx_pcie_enqueue_error_response);

void dx_pcie_dequeue_error_response(u32 dev_id, dx_pcie_dev_err_t* response)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;

	dx_pcie_interrupt_err(dev_id);
	*response = dx_msg->err_response;
	memset(&dx_msg->err_response, 0x00, sizeof(dx_pcie_dev_err_t));
}
EXPORT_SYMBOL_GPL(dx_pcie_dequeue_error_response);

#define EP_IRQ_MSG_OFFSET (0x20)
void dx_pcie_notify_msg_to_device(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	if(dw->dx_ver == 3) {
		writel(1, ((void*)(dx_msg->notify + EP_IRQ_MSG_OFFSET)));
	}
}
EXPORT_SYMBOL_GPL(dx_pcie_notify_msg_to_device);

/* 
Return value:
   0      : success
  -1      : device is not supported this api
  -EINVAL : priority fault
*/
#define EP_IRQ_NORMAL_REQ_QUE0_OFFSET (0x24)
#define EP_IRQ_NORMAL_REQ_QUE1_OFFSET (0x28)
#define EP_IRQ_NORMAL_REQ_QUE2_OFFSET (0x2C)
#define EP_IRQ_HIGH_REQ_OFFSET        (0x30)
int dx_pcie_notify_req_to_device(u32 dev_id, u32 priority)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	int ret = 0;
	if(dw->dx_ver == 3) {
		switch (priority) {
			case DX_NORMAL_QUEUE0:
				writel(1, ((void*)(dx_msg->notify + EP_IRQ_NORMAL_REQ_QUE0_OFFSET)));
				break;
			case DX_NORMAL_QUEUE1:
				writel(1, ((void*)(dx_msg->notify + EP_IRQ_NORMAL_REQ_QUE1_OFFSET)));
				break;
			case DX_NORMAL_QUEUE2:
				writel(1, ((void*)(dx_msg->notify + EP_IRQ_NORMAL_REQ_QUE2_OFFSET)));
				break;
			case DX_HIGH_QUEUE:
				writel(1, ((void*)(dx_msg->notify + EP_IRQ_HIGH_REQ_OFFSET)));
				break;
			default:
				pr_err("%s:Priority is not defined(%d)\n", __func__, priority);
			    ret = -EINVAL;
		}
	} else {
		ret = -1;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(dx_pcie_notify_req_to_device);

/* TODO - will be loaded these information from end point via a bar of pcie*/
static int dx_pcie_set_message_ram_offs(struct dw_edma *dw)
{
	int ret = 0;
	if (dw->dx_ver == 2) {
		ep_ram_info.base_offs	= MSGRAM_MSG_OFFSET_V2;
		ep_ram_info.req0_offs	= ep_ram_info.base_offs + MSGRAM_REQUEST_OFFSET_V2;
		ep_ram_info.resp0_offs	= ep_ram_info.base_offs + MSGRAM_RESPONSE0_OFFSET_V2;
		ep_ram_info.resp1_offs	= ep_ram_info.base_offs + MSGRAM_RESPONSE1_OFFSET_V2;
		ep_ram_info.log_offs	= ep_ram_info.base_offs + MSGRAM_LOG_OFFSET_V2;
		ep_ram_info.debug_offs	= ep_ram_info.base_offs + MSGRAM_DEBUG_OFFSET_V2;
		ep_ram_info.err_offs	= ep_ram_info.base_offs + MSGRAM_ERROR_OFFSET_V2;
		ep_ram_info.irq_offs	= ep_ram_info.base_offs + MSGRAM_IRQ_OFFSET_V2;
	} else if (dw->dx_ver == 3) {
		ep_ram_info.base_offs	= MSGRAM_MSG_OFFSET_V3;
		ep_ram_info.req0_offs	= ep_ram_info.base_offs + MSGRAM_REQUEST0_OFFSET_V3;
		ep_ram_info.req1_offs	= ep_ram_info.base_offs + MSGRAM_REQUEST1_OFFSET_V3;
		ep_ram_info.resp0_offs	= ep_ram_info.base_offs + MSGRAM_RESPONSE0_OFFSET_V3;
		ep_ram_info.resp1_offs	= ep_ram_info.base_offs + MSGRAM_RESPONSE1_OFFSET_V3;
		ep_ram_info.resp2_offs	= ep_ram_info.base_offs + MSGRAM_RESPONSE2_OFFSET_V3;
		ep_ram_info.log_offs	= ep_ram_info.base_offs + MSGRAM_LOG_OFFSET_V3;
		ep_ram_info.debug_offs	= ep_ram_info.base_offs + MSGRAM_DEBUG_OFFSET_V3;
		ep_ram_info.err_offs	= ep_ram_info.base_offs + MSGRAM_ERROR_OFFSET_V3;
		ep_ram_info.irq_offs	= ep_ram_info.base_offs + MSGRAM_IRQ_OFFSET_V3;
	} else {
		ret = -1;
	}
	return ret;
}

int dx_pcie_message_init(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct device *dev = &dw->pdev->dev;
	struct dx_pcie_msg *dx_msg;
	int ret = 0;
	/* TODO - dx_msg is needed to per NPU and we should consieder in case of one handler  */
	dx_msg = devm_kcalloc(dev, 1, sizeof(struct dx_pcie_msg), GFP_KERNEL);
	if (!dx_msg)
		return -ENOMEM;
	dw->dx_msg = dx_msg;

	/* Set meassage ram address */
	ret = dx_pcie_set_message_ram_offs(dw);

	dx_msg->response[0] = dw->npu_region[0].vaddr + ep_ram_info.resp0_offs;
	dx_msg->response[1] = dw->npu_region[0].vaddr + ep_ram_info.resp1_offs;
	dx_msg->response[2] = dw->npu_region[0].vaddr + ep_ram_info.resp2_offs;
	dx_msg->errors      = dw->npu_region[0].vaddr + ep_ram_info.err_offs;
	dx_msg->irq_status  = dw->npu_region[0].vaddr + ep_ram_info.irq_offs;
	dx_msg->notify      = dw->npu_region[1].vaddr;

	INIT_LIST_HEAD(&dx_msg->responses[0].list);
	INIT_LIST_HEAD(&dx_msg->responses[1].list);
	INIT_LIST_HEAD(&dx_msg->responses[2].list);

	spin_lock_init(&dx_msg->responses_lock[0]);
	spin_lock_init(&dx_msg->responses_lock[1]);
	spin_lock_init(&dx_msg->responses_lock[2]);
	spin_lock_init(&dx_msg->err_lock);

	/* IRQ Status Clear */
	if (dw->nr_irqs == 1) {
		memset_io(dx_msg->irq_status, 0x00, 0x100);
	}

	return ret;
}
