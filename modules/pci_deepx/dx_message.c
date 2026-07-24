// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/rcupdate.h>

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
static message_ram_table ep_ram_info;

typedef enum dxresp_lock_t {
    DX_RESP_UNLOCK  = 0,
    DX_RESP_LOCK    = 1,
} dxresp_lock;

static inline void __iomem *dx_resp_lock_addr(void __iomem *response)
{
	return (u8 __iomem *)response + 0x100;
}

static inline void __iomem *dx_notify_addr(struct dx_pcie_msg *dx_msg, u32 offset)
{
	return (u8 __iomem *)dx_msg->notify + offset;
}

#define RES_POOL_SIZE           (32)
static uint32_t resp_pool_header[MAX_DEV_NUM][USER_NUM_MAX];
static dx_pcie_response_list_t resp_pool[MAX_DEV_NUM][USER_NUM_MAX][RES_POOL_SIZE];

/* Callbacks for notifying RT module when responses/events are enqueued */
static dx_pcie_response_cb_t response_callbacks[MAX_DEV_NUM];
static void *response_cb_data[MAX_DEV_NUM];
static dx_pcie_event_cb_t event_callbacks[MAX_DEV_NUM];
static void *event_cb_data[MAX_DEV_NUM];

/* Link-event callback: RT module registers here to be told when the
 * link-health worker observes EP-initiated link drop / restore. */
static dx_pcie_link_event_cb_t link_event_callbacks[MAX_DEV_NUM];
static void *link_event_cb_data[MAX_DEV_NUM];

void __iomem *dx_pcie_get_message_area(u32 dev_id)
{
	struct dw_edma *dw;

	if (dev_id >= MAX_DEV_NUM)
		return NULL;
	dw = dx_dev_list_get(dev_id);
	if (!dw)
		return NULL;
	return dw->npu_region[0].vaddr + ep_ram_info.base_offs;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_message_area);

void __iomem *dx_pcie_get_log_area(u32 dev_id)
{
	struct dw_edma *dw;

	if (dev_id >= MAX_DEV_NUM)
		return NULL;
	dw = dx_dev_list_get(dev_id);
	if (!dw)
		return NULL;
	return dw->npu_region[0].vaddr + ep_ram_info.log_offs;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_log_area);

void __iomem *dx_pcie_get_dl_area(u32 dev_id)
{
	struct dw_edma *dw;

	if (dev_id >= MAX_DEV_NUM)
		return NULL;
	dw = dx_dev_list_get(dev_id);
	if (!dw)
		return NULL;
	return dw->npu_region[0].vaddr + ep_ram_info.dl_offs;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_dl_area);

/* priority : high:10 / normal:0,1,2 (queue0,queue1...) */
void __iomem *dx_pcie_get_request_queue(u32 dev_id, u32 priority)
{
	struct dw_edma *dw;
	void __iomem *reg;

	if (dev_id >= MAX_DEV_NUM)
		return NULL;
	dw = dx_dev_list_get(dev_id);
	if (!dw)
		return NULL;
	reg = dw->npu_region[0].vaddr;
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
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg;
	int i;

	if (!dw)
		return -ENODEV;
	dx_msg = dw->dx_msg;

	for(i = 0; i < DX_PCIE_RESP_NUM; i++) {
		dx_pcie_response_list_t *entry, *tmp;
		unsigned long flags;

		spin_lock_irqsave(&dx_msg->responses_lock[i], flags);
		list_for_each_entry_safe(entry, tmp, &dx_msg->responses[i].list, list) {
			list_del_init(&entry->list);
		}
		resp_pool_header[dev_id][i] = 0;
		spin_unlock_irqrestore(&dx_msg->responses_lock[i], flags);
	}
	return 0;
}
EXPORT_SYMBOL_GPL(dx_pcie_clear_response_queue);

int dx_pcie_is_response_queue_empty(u32 dev_id, int dma_ch)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg;

	if (!dw)
		return 1;
	dx_msg = dw->dx_msg;

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
	struct dx_pcie_msg *dx_msg;
	dx_pcie_response_list_t *entry;
	uint32_t *header;
	uint32_t proc_id;
	dx_pcie_response_cb_t cb;
	void *cb_data;

	if (!dw)
		return;
	dx_msg = dw->dx_msg;

	spin_lock_irqsave(&dx_msg->responses_lock[dma_ch], flags);
	header = &resp_pool_header[dev_id][dma_ch];
	writel(DX_RESP_LOCK, dx_resp_lock_addr(dx_msg->response[dma_ch]));
	(*header) %= RES_POOL_SIZE;
	entry = &resp_pool[dev_id][dma_ch][(*header)++];

	/* Pool overflow guard: if this slot is still linked (not yet dequeued by RT),
	 * remove it from the list before reuse. Without this, list_add_tail on an
	 * already-linked node corrupts the doubly-linked list and causes kernel panic. */
	if (!list_empty(&entry->list)) {
		pr_warn("%s: dev_id %d, ch %d, pool[%d] overwritten (unconsumed resp req_id %d)\n",
			__func__, dw->idx, dma_ch, (*header) - 1, entry->response.req_id);
		list_del_init(&entry->list);
	}

	dx_memcpy_fromio32(&entry->response, dx_msg->response[dma_ch], sizeof(dx_pcie_response_t));
	proc_id = entry->response.proc_id;
	list_add_tail(&entry->list, &dx_msg->responses[dma_ch].list);
	dbg_msg("%s: dev_id %d, ch %d, req_id %d, proc_id %d, pool %d",
		__func__, dw->idx, dma_ch, entry->response.req_id,
		proc_id, *header);

	writel(DX_RESP_UNLOCK, dx_resp_lock_addr(dx_msg->response[dma_ch]));

	/* Notify RT before dropping responses_lock so recovery queue-clear or
	 * pool reuse cannot create a stale response credit for this entry. */
	rcu_read_lock();
	cb = smp_load_acquire(&response_callbacks[dev_id]);
	cb_data = READ_ONCE(response_cb_data[dev_id]);
	if (cb)
		cb(dev_id, dma_ch, proc_id, cb_data);
	rcu_read_unlock();

	spin_unlock_irqrestore(&dx_msg->responses_lock[dma_ch], flags);
}

int dx_pcie_dequeue_response(u32 dev_id, int dma_ch, dx_pcie_response_t* response)
{
	int ret;
	unsigned long flags;
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg;

	if (!dw)
		return -ENODEV;
	dx_msg = dw->dx_msg;

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

void dx_pcie_enqueue_event_response(u32 dev_id, uint32_t err_code)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg;
	unsigned long flags;
	dx_pcie_event_queue_t *q;

	if (!dw)
		return;
	dx_msg = dw->dx_msg;

	spin_lock_irqsave(&dx_msg->event_lock, flags);
	q = &dx_msg->event_queue;
	if (q->count < DX_EVENT_QUEUE_SIZE) {
		dx_pcie_dev_event_t *entry = &q->entries[q->tail];
		memset(entry, 0, sizeof(*entry));
		if (err_code != 0) {
			entry->event_type = DX_EVENT_TYPE_ERROR;
			entry->dx_rt_err.err_code = err_code;
		} else {
			dx_memcpy_fromio32(entry, dx_msg->events,
					   sizeof(dx_pcie_dev_event_t));
			dbg_msg("%s: dev_id %d, code:%d\n", __func__, dev_id,
				entry->event_type);
			dx_memset_io32(dx_msg->events,
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
		dx_pcie_event_cb_t cb;

		rcu_read_lock();
		cb = smp_load_acquire(&event_callbacks[dev_id]);
		if (cb)
			cb(dev_id, READ_ONCE(event_cb_data[dev_id]));
		rcu_read_unlock();
	}
}
EXPORT_SYMBOL_GPL(dx_pcie_enqueue_event_response);

/**
 * dx_pcie_enqueue_recovery_event - Enqueue a DX_EVENT_TYPE_RECOVERY entry.
 * @dev_id:              Device index
 * @subcode:             enum dx_recovery_subcode
 * @reason:              enum dx_recovery_reason (0 if not applicable)
 * @recovery_count:      Cumulative successful recoveries (snapshot)
 * @recovery_fail_count: Cumulative failed recovery attempts (snapshot)
 * @dev_state:           Current dxrt_dev_state_t value (snapshot)
 *
 * Unlike dx_pcie_enqueue_event_response() which only carries an
 * err_code, this helper fills the recovery union member so user-space
 * can tell which phase of the recovery pass is firing without having
 * to correlate with dmesg.  Safe from process context (callers are
 * the RT link-event notify hook and the link-health worker).
 */
void dx_pcie_enqueue_recovery_event(u32 dev_id, uint32_t subcode,
				    uint32_t reason,
				    uint32_t recovery_count,
				    uint32_t recovery_fail_count,
				    uint32_t dev_state)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg;
	unsigned long flags;
	dx_pcie_event_queue_t *q;

	if (!dw)
		return;
	dx_msg = dw->dx_msg;

	spin_lock_irqsave(&dx_msg->event_lock, flags);
	q = &dx_msg->event_queue;
	if (q->count < DX_EVENT_QUEUE_SIZE) {
		dx_pcie_dev_event_t *entry = &q->entries[q->tail];
		memset(entry, 0, sizeof(*entry));
		entry->event_type = DX_EVENT_TYPE_RECOVERY;
		entry->dx_rt_recovery.subcode             = subcode;
		entry->dx_rt_recovery.reason              = reason;
		entry->dx_rt_recovery.recovery_count      = recovery_count;
		entry->dx_rt_recovery.recovery_fail_count = recovery_fail_count;
		entry->dx_rt_recovery.dev_state           = dev_state;
		q->tail = (q->tail + 1) % DX_EVENT_QUEUE_SIZE;
		q->count++;
	} else {
		pr_warn("%s: event queue full, dropping recovery event (subcode=%u)\n",
			__func__, subcode);
	}
	spin_unlock_irqrestore(&dx_msg->event_lock, flags);

	/* Wake HW event IRQ waitqueue — same reason as enqueue_event_response */
	dx_pcie_interrupt_event_wakeup(dev_id);

	/* Notify RT module that an event is available */
	{
		dx_pcie_event_cb_t cb;

		rcu_read_lock();
		cb = smp_load_acquire(&event_callbacks[dev_id]);
		if (cb)
			cb(dev_id, READ_ONCE(event_cb_data[dev_id]));
		rcu_read_unlock();
	}
}
EXPORT_SYMBOL_GPL(dx_pcie_enqueue_recovery_event);

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
	struct dx_pcie_msg *dx_msg;
	unsigned long flags;
	dx_pcie_event_queue_t *q;

	if (!dw)
		return;
	dx_msg = dw->dx_msg;

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
		dx_pcie_event_cb_t cb;

		rcu_read_lock();
		cb = smp_load_acquire(&event_callbacks[dev_id]);
		if (cb)
			cb(dev_id, READ_ONCE(event_cb_data[dev_id]));
		rcu_read_unlock();
	}
}
EXPORT_SYMBOL_GPL(dx_pcie_enqueue_abort_event);


void dx_pcie_dequeue_event_response(u32 dev_id, dx_pcie_dev_event_t* response)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg;
	unsigned long flags;

	if (!dw) {
		memset(response, 0, sizeof(*response));
		return;
	}
	dx_msg = dw->dx_msg;

	spin_lock_irqsave(&dx_msg->event_lock, flags);
	if (dx_msg->event_queue.count > 0) {
		*response = dx_msg->event_queue.entries[dx_msg->event_queue.head];
		dx_msg->event_queue.head = (dx_msg->event_queue.head + 1) % DX_EVENT_QUEUE_SIZE;
		dx_msg->event_queue.count--;
		spin_unlock_irqrestore(&dx_msg->event_lock, flags);
		return;
	}
	spin_unlock_irqrestore(&dx_msg->event_lock, flags);

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
	struct dx_pcie_msg *dx_msg;

	if (!dw)
		return 0;
	dx_msg = dw->dx_msg;

	/* Check if there are pending events in the circular buffer */
	return dx_msg->event_queue.count > 0;
}
EXPORT_SYMBOL_GPL(dx_pcie_is_event_pending);

void dx_pcie_clear_event_response(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg;
	unsigned long flags;

	if (!dw)
		return;
	dx_msg = dw->dx_msg;

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
		ret = readl(dx_notify_addr(dx_msg, EP_IRQ_MSG_EN_OFFSET));
		if (ret)
			break;
		udelay(10);
	}
	return ret;
}
void dx_pcie_notify_msg_to_device(u32 dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	struct dx_pcie_msg *dx_msg;

	if (!dw)
		return;
	dx_msg = dw->dx_msg;

	if(dx_pcie_is_notify_enable(dx_msg)) {
		if(dw->dx_ver == 3) {
			writel(1, dx_notify_addr(dx_msg, EP_IRQ_MSG_OFFSET));
		}
	} else {
		/*
		 * notify-enable bit on the EP did not become 1 within the
		 * 10ms poll window.  This is *expected* during a FW
		 * bootloader reboot kicked by DXRT_CMD_RECOVERY (the new FW
		 * contract): the mailbox interface goes away while romcode/
		 * bootloader/FreeRTOS re-initialize.  Demote the log to a
		 * ratelimited warning so this normal transient does not look
		 * like a hard fault and does not provoke userspace into
		 * issuing another recovery on top of the one already in
		 * flight.  Real link-down faults are reported via the
		 * MMIO-0xFFFFFFFF path in dxrt_polling_ack() and the
		 * link-health worker.
		 */
		pr_warn_ratelimited("dx_pcie: notify interface not ready (dev=%u) — FW may be rebooting\n",
			dev_id);
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
	struct dx_pcie_msg *dx_msg;
	int ret = 0;

	if (!dw)
		return -ENODEV;
	dx_msg = dw->dx_msg;

	if(dw->dx_ver == 3) {
		switch (queue) {
			case DX_NORMAL_QUEUE0:
				if (lock)
					writel(1, dx_notify_addr(dx_msg, EP_IRQ_NORMAL_QUE0_LOCK_OFFSET));
				else
					writel(1, dx_notify_addr(dx_msg, EP_IRQ_NORMAL_QUE0_UNLOCK_OFFSET));
				break;
			case DX_NORMAL_QUEUE1:
				if (lock)
					writel(1, dx_notify_addr(dx_msg, EP_IRQ_NORMAL_QUE1_LOCK_OFFSET));
				else
					writel(1, dx_notify_addr(dx_msg, EP_IRQ_NORMAL_QUE1_UNLOCK_OFFSET));
				break;
			case DX_NORMAL_QUEUE2:
				if (lock)
					writel(1, dx_notify_addr(dx_msg, EP_IRQ_NORMAL_QUE2_LOCK_OFFSET));
				else
					writel(1, dx_notify_addr(dx_msg, EP_IRQ_NORMAL_QUE2_UNLOCK_OFFSET));
				break;
			case DX_HIGH_QUEUE:
				if (lock)
					writel(1, dx_notify_addr(dx_msg, EP_IRQ_HIGH_LOCK_OFFSET));
				else
					writel(1, dx_notify_addr(dx_msg, EP_IRQ_HIGH_UNLOCK_OFFSET));
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
	WRITE_ONCE(response_cb_data[dev_id], data);
	smp_store_release(&response_callbacks[dev_id], cb);
}
EXPORT_SYMBOL_GPL(dx_pcie_register_response_callback);

void dx_pcie_unregister_response_callback(u32 dev_id)
{
	smp_store_release(&response_callbacks[dev_id], NULL);
	synchronize_rcu();
	WRITE_ONCE(response_cb_data[dev_id], NULL);
}
EXPORT_SYMBOL_GPL(dx_pcie_unregister_response_callback);

void dx_pcie_register_event_callback(u32 dev_id, dx_pcie_event_cb_t cb, void *data)
{
	WRITE_ONCE(event_cb_data[dev_id], data);
	smp_store_release(&event_callbacks[dev_id], cb);
}
EXPORT_SYMBOL_GPL(dx_pcie_register_event_callback);

void dx_pcie_unregister_event_callback(u32 dev_id)
{
	smp_store_release(&event_callbacks[dev_id], NULL);
	synchronize_rcu();
	WRITE_ONCE(event_cb_data[dev_id], NULL);
}
EXPORT_SYMBOL_GPL(dx_pcie_unregister_event_callback);

void dx_pcie_register_link_event_callback(u32 dev_id,
					  dx_pcie_link_event_cb_t cb,
					  void *data)
{
	if (dev_id >= MAX_DEV_NUM)
		return;
	WRITE_ONCE(link_event_cb_data[dev_id], data);
	smp_store_release(&link_event_callbacks[dev_id], cb);
}
EXPORT_SYMBOL_GPL(dx_pcie_register_link_event_callback);

void dx_pcie_unregister_link_event_callback(u32 dev_id)
{
	if (dev_id >= MAX_DEV_NUM)
		return;
	smp_store_release(&link_event_callbacks[dev_id], NULL);
	synchronize_rcu();
	WRITE_ONCE(link_event_cb_data[dev_id], NULL);
}
EXPORT_SYMBOL_GPL(dx_pcie_unregister_link_event_callback);

/*
 * dx_pcie_notify_link_event - Call the RT-module link-event handler
 *
 * Invoked from the link-health worker (process context) when the EP
 * drops or restores its link without any AER/reset callback firing.
 * The RT module uses this to set dev->recovering and wake waiters.
 */
void dx_pcie_notify_link_event(u32 dev_id, dx_pcie_link_event_t ev)
{
	dx_pcie_link_event_cb_t cb;
	void *data;

	if (dev_id >= MAX_DEV_NUM)
		return;

	rcu_read_lock();
	cb = smp_load_acquire(&link_event_callbacks[dev_id]);
	if (!cb)
		goto out;

	data = READ_ONCE(link_event_cb_data[dev_id]);
	cb(dev_id, ev, data);

out:
	rcu_read_unlock();
}
EXPORT_SYMBOL_GPL(dx_pcie_notify_link_event);

int dx_pcie_message_init(int dev_id)
{
	struct dw_edma *dw;
	struct device *dev;
	struct dx_pcie_msg *dx_msg;
	int i, n;
	int ret = 0;

	if (dev_id < 0 || dev_id >= MAX_DEV_NUM)
		return -EINVAL;
	dw = dx_dev_list_get(dev_id);
	if (!dw)
		return -ENODEV;
	dev = &dw->pdev->dev;
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

	/* Initialize event_queue (circular buffer) */
	dx_msg->event_queue.head = 0;
	dx_msg->event_queue.tail = 0;
	dx_msg->event_queue.count = 0;

	writel(DX_RESP_UNLOCK, dx_resp_lock_addr(dx_msg->response[0]));
	writel(DX_RESP_UNLOCK, dx_resp_lock_addr(dx_msg->response[1]));
	writel(DX_RESP_UNLOCK, dx_resp_lock_addr(dx_msg->response[2]));

	/* IRQ Status Clear */
	if (dw->nr_irqs == 1) {
		dx_memset_io32(dx_msg->irq_status, 0x00, 0x100);
	}

	return ret;
}
