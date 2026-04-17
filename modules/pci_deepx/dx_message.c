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
#include "dx_mmio_compat.h"

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
#define MSGRAM_EVENT_OFFSET_V3     (0x8000)
#define MSGRAM_IRQ_OFFSET_V3       (0x8200)
#define MSGRAM_END_REGION_V3       (0x8F00)
#define MSGRAM_DL_OFFSET_V3        (0x8FC0)

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
	uint32_t event_offs;
	uint32_t irq_offs;
	uint32_t dl_offs;
} message_ram_table;
message_ram_table ep_ram_info;

typedef enum dxresp_lock_t {
    DX_RESP_UNLOCK  = 0,
    DX_RESP_LOCK    = 1,
} dxresp_lock;

#define RES_POOL_SIZE           (32)
static uint32_t resp_pool_header[MAX_DEV_NUM][USER_NUM_MAX];
static dx_pcie_response_list_t resp_pool[MAX_DEV_NUM][USER_NUM_MAX][RES_POOL_SIZE];
static atomic_t resp_pool_drop_cnt[MAX_DEV_NUM];

/* Callbacks for notifying RT module when responses/events are enqueued */
static dx_pcie_response_cb_t response_callbacks[MAX_DEV_NUM];
static void *response_cb_data[MAX_DEV_NUM];
static dx_pcie_event_cb_t event_callbacks[MAX_DEV_NUM];
static void *event_cb_data[MAX_DEV_NUM];

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

void __iomem *dx_pcie_get_dl_area(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	void __iomem *reg = dw->npu_region[0].vaddr;
	return reg + ep_ram_info.dl_offs;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_dl_area);

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
			list_del_init(&entry->list);
		}
		resp_pool_header[dev_id][i] = 0;
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
	uint32_t *header;
	dx_pcie_response_cb_t cb;

	spin_lock_irqsave(&dx_msg->responses_lock[dma_ch], flags);
	header = &resp_pool_header[dev_id][dma_ch];
	writel(DX_RESP_LOCK, ((void*)dx_msg->response[dma_ch]+0x100));
	(*header) %= RES_POOL_SIZE;
	entry = &resp_pool[dev_id][dma_ch][(*header)++];

	/* Pool overflow guard: if this slot is still linked (not yet dequeued by RT),
	 * remove it from the list before reuse. Without this, list_add_tail on an
	 * already-linked node corrupts the doubly-linked list and causes kernel panic. */
	if (!list_empty(&entry->list)) {
		int drop_total = atomic_inc_return(&resp_pool_drop_cnt[dev_id]);
		pr_warn("%s: dev_id %d, ch %d, pool[%d] overwritten (unconsumed resp req_id %d, total_drops %d)\n",
			__func__, dw->idx, dma_ch, (*header) - 1, entry->response.req_id, drop_total);
		list_del_init(&entry->list);
	}

	dx_memcpy_fromio32(&entry->response, dx_msg->response[dma_ch], sizeof(dx_pcie_response_t));
	list_add_tail(&entry->list, &dx_msg->responses[dma_ch].list);
	dbg_msg("%s: dev_id %d, ch %d, %d, %d", __func__, dw->idx, dma_ch, entry->response.req_id, *header);

	writel(DX_RESP_UNLOCK, ((void*)dx_msg->response[dma_ch]+0x100));
	spin_unlock_irqrestore(&dx_msg->responses_lock[dma_ch], flags);

	/* Notify RT module that a response is available */
	cb = READ_ONCE(response_callbacks[dev_id]);
	if (cb)
		cb(dev_id, dma_ch, response_cb_data[dev_id]);
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
		list_del_init(&entry->list);
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

int dx_pcie_cleanup_responses_for_proc(u32 dev_id, uint32_t proc_id)
{
	int i, count = 0;
	unsigned long flags;
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	dx_pcie_response_list_t *entry, *tmp;

	for (i = 0; i < DX_PCIE_RESP_NUM; i++) {
		spin_lock_irqsave(&dx_msg->responses_lock[i], flags);
		list_for_each_entry_safe(entry, tmp, &dx_msg->responses[i].list, list) {
			if (entry->response.proc_id == proc_id) {
				dbg_msg("%s: dev_id %d, ch %d, proc %d removed",
					__func__, dw->idx, i, proc_id);
				list_del_init(&entry->list);
				count++;
			}
		}
		spin_unlock_irqrestore(&dx_msg->responses_lock[i], flags);
	}
	return count;
}
EXPORT_SYMBOL_GPL(dx_pcie_cleanup_responses_for_proc);

void dx_pcie_enqueue_event_response(u32 dev_id, uint32_t err_code)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	unsigned long flags;
	dx_pcie_event_queue_t *q;

	spin_lock_irqsave(&dx_msg->event_lock, flags);
	q = &dx_msg->event_queue;
	if (q->count < DX_EVENT_QUEUE_SIZE) {
		dx_pcie_dev_event_t *entry = &q->entries[q->tail];
		memset(entry, 0, sizeof(*entry));
		if (err_code != 0) {
			entry->event_type = DX_EVENT_TYPE_ERROR;
			entry->dx_rt_err.err_code = err_code;
		} else {
			dx_memcpy_fromio32(entry, (dx_pcie_dev_event_t *)dx_msg->events,
					   sizeof(dx_pcie_dev_event_t));
			dbg_msg("%s: dev_id %d, code:%d\n", __func__, dev_id,
				entry->event_type);
			dx_memset_io32((volatile void __iomem *)dx_msg->events,
				       0x00, sizeof(dx_pcie_dev_event_t));
		}
		q->tail = (q->tail + 1) % DX_EVENT_QUEUE_SIZE;
		q->count++;
	} else {
		pr_warn("%s: event queue full, dropping event (err_code=%u)\n",
			__func__, err_code);
	}
	spin_unlock_irqrestore(&dx_msg->event_lock, flags);

	/* For SW-generated error events (err_code != 0), also wake the HW
	 * event IRQ waitqueue.  dx_pcie_dequeue_event_response() calls
	 * dx_pcie_interrupt_event() which blocks on event_irq->events_wq.
	 * Normal device events wake that queue via the MSI handler, but
	 * SW-enqueued errors bypass the MSI path — without this wakeup
	 * the dequeue call deadlocks and user-space never receives the
	 * error event, preventing recovery. */
	if (err_code != 0)
		dx_pcie_interrupt_event_wakeup(dev_id);

	/* Notify RT module that an event is available */
	{
		dx_pcie_event_cb_t cb = READ_ONCE(event_callbacks[dev_id]);
		if (cb)
			cb(dev_id, event_cb_data[dev_id]);
	}
}
EXPORT_SYMBOL_GPL(dx_pcie_enqueue_event_response);

/**
 * dx_pcie_enqueue_abort_event - Enqueue enriched DMA abort event.
 * @dev_id:        Device index
 * @err_code:      Error code (ERR_PCIE_DMA_CH_ABORT(ch))
 * @dma_err_status: HW err_status register value from the aborted channel
 * @wr_ch_sts:     Array of 4 write channel CS values
 * @rd_ch_sts:     Array of 4 read channel CS values
 *
 * Like dx_pcie_enqueue_event_response() but populates additional
 * diagnostic fields (dma_err, dma_wr_ch_sts[], dma_rd_ch_sts[])
 * so user-space can determine the abort cause and channel states
 * without needing a separate status query.
 *
 * Called from IRQ context (abort ISR).
 */
void dx_pcie_enqueue_abort_event(u32 dev_id, uint32_t err_code,
				 uint32_t dma_err_status,
				 const uint32_t *wr_ch_sts,
				 const uint32_t *rd_ch_sts)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	unsigned long flags;
	dx_pcie_event_queue_t *q;

	spin_lock_irqsave(&dx_msg->event_lock, flags);
	q = &dx_msg->event_queue;
	if (q->count < DX_EVENT_QUEUE_SIZE) {
		dx_pcie_dev_event_t *entry = &q->entries[q->tail];
		memset(entry, 0, sizeof(*entry));
		entry->event_type = DX_EVENT_TYPE_ERROR;
		entry->dx_rt_err.err_code = err_code;
		entry->dx_rt_err.dma_err = dma_err_status;
		memcpy(entry->dx_rt_err.dma_wr_ch_sts, wr_ch_sts,
		       sizeof(uint32_t) * 4);
		memcpy(entry->dx_rt_err.dma_rd_ch_sts, rd_ch_sts,
		       sizeof(uint32_t) * 4);
		q->tail = (q->tail + 1) % DX_EVENT_QUEUE_SIZE;
		q->count++;
	} else {
		pr_warn("%s: event queue full, dropping abort event (err_code=%u)\n",
			__func__, err_code);
	}
	spin_unlock_irqrestore(&dx_msg->event_lock, flags);

	/* Wake HW event IRQ waitqueue — same reason as enqueue_event_response */
	dx_pcie_interrupt_event_wakeup(dev_id);

	/* Notify RT module that an event is available */
	{
		dx_pcie_event_cb_t cb = READ_ONCE(event_callbacks[dev_id]);
		if (cb)
			cb(dev_id, event_cb_data[dev_id]);
	}
}
EXPORT_SYMBOL_GPL(dx_pcie_enqueue_abort_event);


void dx_pcie_enqueue_proc_exit_event(u32 dev_id, pid_t proc_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	dx_pcie_proc_exit_queue_t *q = &dx_msg->proc_exit_queue;
	unsigned long flags;

	spin_lock_irqsave(&dx_msg->proc_exit_lock, flags);
	if (q->count < DX_PROC_EXIT_QUEUE_SIZE) {
		q->entries[q->tail].proc_id = proc_id;
		q->tail = (q->tail + 1) % DX_PROC_EXIT_QUEUE_SIZE;
		q->count++;
		dbg_msg("%s: dev_id %d, proc_id %d queued (count=%d)\n",
			__func__, dev_id, proc_id, q->count);
	} else {
		pr_warn("%s: proc_exit queue full, dropping proc_id %d\n",
			__func__, proc_id);
	}
	spin_unlock_irqrestore(&dx_msg->proc_exit_lock, flags);
}
EXPORT_SYMBOL_GPL(dx_pcie_enqueue_proc_exit_event);

int dx_pcie_dequeue_proc_exit_event(u32 dev_id, pid_t *proc_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	dx_pcie_proc_exit_queue_t *q = &dx_msg->proc_exit_queue;
	unsigned long flags;
	int ret = -1;

	spin_lock_irqsave(&dx_msg->proc_exit_lock, flags);
	if (q->count > 0) {
		*proc_id = q->entries[q->head].proc_id;
		q->head = (q->head + 1) % DX_PROC_EXIT_QUEUE_SIZE;
		q->count--;
		ret = 0;
		dbg_msg("%s: dev_id %d, proc_id %d dequeued (count=%d)\n",
			__func__, dev_id, *proc_id, q->count);
	}
	spin_unlock_irqrestore(&dx_msg->proc_exit_lock, flags);
	return ret;
}
EXPORT_SYMBOL_GPL(dx_pcie_dequeue_proc_exit_event);

int dx_pcie_is_proc_exit_pending(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;

	/* Return the count of pending proc_exit events */
	return dx_msg->proc_exit_queue.count;
}
EXPORT_SYMBOL_GPL(dx_pcie_is_proc_exit_pending);

void dx_pcie_clear_proc_exit_queue(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	dx_pcie_proc_exit_queue_t *q = &dx_msg->proc_exit_queue;
	unsigned long flags;

	spin_lock_irqsave(&dx_msg->proc_exit_lock, flags);
	q->head = 0;
	q->tail = 0;
	q->count = 0;
	spin_unlock_irqrestore(&dx_msg->proc_exit_lock, flags);
}
EXPORT_SYMBOL_GPL(dx_pcie_clear_proc_exit_queue);

void dx_pcie_clear_proc_exit_for_pid(u32 dev_id, pid_t proc_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	dx_pcie_proc_exit_queue_t *q = &dx_msg->proc_exit_queue;
	unsigned long flags;
	dx_pcie_dev_proc_exit_t temp[DX_PROC_EXIT_QUEUE_SIZE];
	int i, new_count = 0;

	spin_lock_irqsave(&dx_msg->proc_exit_lock, flags);
	/* Copy entries that do NOT match proc_id */
	for (i = 0; i < q->count; i++) {
		int idx = (q->head + i) % DX_PROC_EXIT_QUEUE_SIZE;
		if (q->entries[idx].proc_id != (uint32_t)proc_id) {
			temp[new_count++] = q->entries[idx];
		}
	}
	/* Rebuild queue without matching entries */
	q->head = 0;
	q->tail = new_count;
	q->count = new_count;
	for (i = 0; i < new_count; i++)
		q->entries[i] = temp[i];
	spin_unlock_irqrestore(&dx_msg->proc_exit_lock, flags);
}
EXPORT_SYMBOL_GPL(dx_pcie_clear_proc_exit_for_pid);

void dx_pcie_dequeue_event_response(u32 dev_id, dx_pcie_dev_event_t* response)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	unsigned long flags;

	dx_pcie_interrupt_event(dev_id);

	spin_lock_irqsave(&dx_msg->event_lock, flags);
	if (dx_msg->event_queue.count > 0) {
		*response = dx_msg->event_queue.entries[dx_msg->event_queue.head];
		dx_msg->event_queue.head = (dx_msg->event_queue.head + 1) % DX_EVENT_QUEUE_SIZE;
		dx_msg->event_queue.count--;
	} else {
		memset(response, 0, sizeof(*response));
	}
	spin_unlock_irqrestore(&dx_msg->event_lock, flags);
}
EXPORT_SYMBOL_GPL(dx_pcie_dequeue_event_response);

int dx_pcie_is_event_pending(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;

	/* Check if there are pending events in the circular buffer */
	return dx_msg->event_queue.count > 0;
}
EXPORT_SYMBOL_GPL(dx_pcie_is_event_pending);

void dx_pcie_clear_event_response(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	unsigned long flags;

	spin_lock_irqsave(&dx_msg->event_lock, flags);
	dx_msg->event_queue.head = 0;
	dx_msg->event_queue.tail = 0;
	dx_msg->event_queue.count = 0;
	spin_unlock_irqrestore(&dx_msg->event_lock, flags);
}
EXPORT_SYMBOL_GPL(dx_pcie_clear_event_response);

#define EP_IRQ_MSG_OFFSET		(0x20)
#define EP_IRQ_MSG_EN_OFFSET	(0x60)
static uint32_t dx_pcie_is_notify_enable(struct dx_pcie_msg *dx_msg)
{
	uint32_t ret = 0, retry = 0;
	while (retry++ < 1000) {
		ret = readl((void*)(dx_msg->notify + EP_IRQ_MSG_EN_OFFSET));
		if (ret)
			break;
		udelay(10);
	}
	return ret;
}
void dx_pcie_notify_msg_to_device(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;

	if(dx_pcie_is_notify_enable(dx_msg)) {
		if(dw->dx_ver == 3) {
			writel(1, ((void*)(dx_msg->notify + EP_IRQ_MSG_OFFSET)));
		}
	} else {
		pr_err("Device error for interrupt\n");
	}
}
EXPORT_SYMBOL_GPL(dx_pcie_notify_msg_to_device);

/* 
Return value:
   0      : success
  -1      : device is not supported this api
  -EINVAL : priority fault
*/
// #define EP_IRQ_RPI_SHUTDOWN_OFFSET		(0x18)
#define EP_IRQ_NORMAL_QUE0_LOCK_OFFSET		(0x1C)
#define EP_IRQ_NORMAL_QUE0_UNLOCK_OFFSET	(0x24)
#define EP_IRQ_NORMAL_QUE1_LOCK_OFFSET		(0x28)
#define EP_IRQ_NORMAL_QUE1_UNLOCK_OFFSET	(0x2C)
#define EP_IRQ_NORMAL_QUE2_LOCK_OFFSET		(0x30)
#define EP_IRQ_NORMAL_QUE2_UNLOCK_OFFSET	(0x34)
#define EP_IRQ_HIGH_LOCK_OFFSET				(0x38)
#define EP_IRQ_HIGH_UNLOCK_OFFSET			(0x3C)
int dx_pcie_notify_req_to_device(u32 dev_id, u32 queue, u8 lock)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg = dw->dx_msg;
	int ret = 0;
	if(dw->dx_ver == 3) {
		switch (queue) {
			case DX_NORMAL_QUEUE0:
				if (lock)
					writel(1, ((void*)(dx_msg->notify + EP_IRQ_NORMAL_QUE0_LOCK_OFFSET)));
				else
					writel(1, ((void*)(dx_msg->notify + EP_IRQ_NORMAL_QUE0_UNLOCK_OFFSET)));
				break;
			case DX_NORMAL_QUEUE1:
				if (lock)
					writel(1, ((void*)(dx_msg->notify + EP_IRQ_NORMAL_QUE1_LOCK_OFFSET)));
				else
					writel(1, ((void*)(dx_msg->notify + EP_IRQ_NORMAL_QUE1_UNLOCK_OFFSET)));
				break;
			case DX_NORMAL_QUEUE2:
				if (lock)
					writel(1, ((void*)(dx_msg->notify + EP_IRQ_NORMAL_QUE2_LOCK_OFFSET)));
				else
					writel(1, ((void*)(dx_msg->notify + EP_IRQ_NORMAL_QUE2_UNLOCK_OFFSET)));
				break;
			case DX_HIGH_QUEUE:
				if (lock)
					writel(1, ((void*)(dx_msg->notify + EP_IRQ_HIGH_LOCK_OFFSET)));
				else
					writel(1, ((void*)(dx_msg->notify + EP_IRQ_HIGH_UNLOCK_OFFSET)));
				break;
			default:
				pr_err("%s:queue is not defined(%d)\n", __func__, queue);
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
	if (dw->dx_ver == 3) {
		ep_ram_info.base_offs	= MSGRAM_MSG_OFFSET_V3;
		ep_ram_info.req0_offs	= ep_ram_info.base_offs + MSGRAM_REQUEST0_OFFSET_V3;
		ep_ram_info.req1_offs	= ep_ram_info.base_offs + MSGRAM_REQUEST1_OFFSET_V3;
		ep_ram_info.req2_offs	= ep_ram_info.base_offs + MSGRAM_REQUEST2_OFFSET_V3;
		ep_ram_info.resp0_offs	= ep_ram_info.base_offs + MSGRAM_RESPONSE0_OFFSET_V3;
		ep_ram_info.resp1_offs	= ep_ram_info.base_offs + MSGRAM_RESPONSE1_OFFSET_V3;
		ep_ram_info.resp2_offs	= ep_ram_info.base_offs + MSGRAM_RESPONSE2_OFFSET_V3;
		ep_ram_info.log_offs	= ep_ram_info.base_offs + MSGRAM_LOG_OFFSET_V3;
		ep_ram_info.debug_offs	= ep_ram_info.base_offs + MSGRAM_DEBUG_OFFSET_V3;
		ep_ram_info.event_offs	= ep_ram_info.base_offs + MSGRAM_EVENT_OFFSET_V3;
		ep_ram_info.irq_offs	= ep_ram_info.base_offs + MSGRAM_IRQ_OFFSET_V3;
		ep_ram_info.dl_offs		= ep_ram_info.base_offs + MSGRAM_DL_OFFSET_V3;
	} else {
		ret = -1;
	}
	return ret;
}

void dx_pcie_register_response_callback(u32 dev_id, dx_pcie_response_cb_t cb, void *data)
{
	response_cb_data[dev_id] = data;
	/* Ensure data is visible before callback pointer */
	smp_wmb();
	WRITE_ONCE(response_callbacks[dev_id], cb);
}
EXPORT_SYMBOL_GPL(dx_pcie_register_response_callback);

void dx_pcie_unregister_response_callback(u32 dev_id)
{
	WRITE_ONCE(response_callbacks[dev_id], NULL);
	smp_wmb();
	response_cb_data[dev_id] = NULL;
}
EXPORT_SYMBOL_GPL(dx_pcie_unregister_response_callback);

void dx_pcie_register_event_callback(u32 dev_id, dx_pcie_event_cb_t cb, void *data)
{
	event_cb_data[dev_id] = data;
	smp_wmb();
	WRITE_ONCE(event_callbacks[dev_id], cb);
}
EXPORT_SYMBOL_GPL(dx_pcie_register_event_callback);

void dx_pcie_unregister_event_callback(u32 dev_id)
{
	WRITE_ONCE(event_callbacks[dev_id], NULL);
	smp_wmb();
	event_cb_data[dev_id] = NULL;
}
EXPORT_SYMBOL_GPL(dx_pcie_unregister_event_callback);

int dx_pcie_message_init(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct device *dev = &dw->pdev->dev;
	struct dx_pcie_msg *dx_msg;
	int i, n;
	int ret = 0;
	/* TODO - dx_msg is needed to per NPU and we should consider in case of one handler  */
	dx_msg = devm_kcalloc(dev, 1, sizeof(struct dx_pcie_msg), GFP_KERNEL);
	if (!dx_msg)
		return -ENOMEM;
	dw->dx_msg = dx_msg;

	/* Initialize resp_pool entries for this device */
	for (i = 0; i < USER_NUM_MAX; i++) {
		resp_pool_header[dev_id][i] = 0;
		for (n = 0; n < RES_POOL_SIZE; n++)
			INIT_LIST_HEAD(&resp_pool[dev_id][i][n].list);
	}

	/* Set meassage ram address */
	ret = dx_pcie_set_message_ram_offs(dw);

	dx_msg->response[0] = dw->npu_region[0].vaddr + ep_ram_info.resp0_offs;
	dx_msg->response[1] = dw->npu_region[0].vaddr + ep_ram_info.resp1_offs;
	dx_msg->response[2] = dw->npu_region[0].vaddr + ep_ram_info.resp2_offs;
	dx_msg->events      = dw->npu_region[0].vaddr + ep_ram_info.event_offs;
	dx_msg->irq_status  = dw->npu_region[0].vaddr + ep_ram_info.irq_offs;
	dx_msg->notify      = dw->npu_region[1].vaddr;

	INIT_LIST_HEAD(&dx_msg->responses[0].list);
	INIT_LIST_HEAD(&dx_msg->responses[1].list);
	INIT_LIST_HEAD(&dx_msg->responses[2].list);

	spin_lock_init(&dx_msg->responses_lock[0]);
	spin_lock_init(&dx_msg->responses_lock[1]);
	spin_lock_init(&dx_msg->responses_lock[2]);
	spin_lock_init(&dx_msg->event_lock);
	spin_lock_init(&dx_msg->proc_exit_lock);

	/* Initialize proc_exit_queue */
	dx_msg->proc_exit_queue.head = 0;
	dx_msg->proc_exit_queue.tail = 0;
	dx_msg->proc_exit_queue.count = 0;

	/* Initialize event_queue (circular buffer) */
	dx_msg->event_queue.head = 0;
	dx_msg->event_queue.tail = 0;
	dx_msg->event_queue.count = 0;

	writel(DX_RESP_UNLOCK, ((void*)dx_msg->response[0]+0x100));
	writel(DX_RESP_UNLOCK, ((void*)dx_msg->response[1]+0x100));
	writel(DX_RESP_UNLOCK, ((void*)dx_msg->response[2]+0x100));

	/* IRQ Status Clear */
	if (dw->nr_irqs == 1) {
		dx_memset_io32(dx_msg->irq_status, 0x00, 0x100);
	}

	return ret;
}
