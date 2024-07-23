// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#include <linux/io.h>
#include <linux/delay.h>
#include "dxrt_drv.h"

typedef enum dxqueue_lock_t {
    DX_QUEUE_UNLOCK     = 0,
    DX_QUEUE_HOST_LOCK  = 33,
    DX_QUEUE_DEV_LOCK   = 2,
} dxqueue_lock;

static size_t buffer_offset = offsetof(dxrt_queue_t, buffer[0]);
void dxrt_init_queue(dxrt_queue_t* q, uint32_t max_count, uint32_t elem_size)
{
	memset(q, 0, sizeof(dxrt_queue_t));
	q->lock = 0;
	q->front = 0;
	q->rear = 0;
	q->count = 0;
	q->max_count = max_count;
	q->elem_size = elem_size;
	q->enable = 1;
}
void dxrt_enable_queue(dxrt_queue_t *q)
{
	q->enable = 1;
}
void dxrt_disable_queue(dxrt_queue_t *q)
{
	q->enable = 0;
}
int dxrt_is_queue_empty(dxrt_queue_t* q)
{
	volatile uint32_t val;
	void __iomem *count = &q->count;
	val = readl(count);
	return val == 0;
}
int dxrt_is_queue_full(dxrt_queue_t* q)
{
	volatile uint32_t val;
	void __iomem *count = &q->count;
	val = readl(count);
	return val == q->max_count;
}
#define DXRT_ENQUEUE_TIMEOUT	(1000)
int dxrt_lock_queue(dxrt_queue_t *q)
{
	volatile uint32_t val;
	void __iomem *lock = &q->lock;
	int timeout = DXRT_ENQUEUE_TIMEOUT;
	int ret = 1;
	while(timeout >= 0)
	{
		val = readl(lock);
		if (val==0)
			break;
		else
		{
			timeout--;
			pr_debug( "%s: wait for lock.\n", __func__);
			udelay(10);
		}
	}
	if (timeout < 0) {
		ret = 0;
		pr_err("%s: failed(%d)\n", __func__, timeout);
	} else {
		writel(DX_QUEUE_HOST_LOCK, lock);
	}
	return ret;
}
void dxrt_unlock_queue(dxrt_queue_t *q)
{	
	void __iomem *lock = &q->lock;
	writel(DX_QUEUE_UNLOCK, lock);
}
void dxrt_enqueue_irq_notify(dxrt_queue_t* q)
{
	q->irq = 1;
}
int dxrt_enqueue_irq_done(dxrt_queue_t* q)
{
	int timeout = 10000;
	int ret = 0;
	do {
		if (timeout-- < 0) {
			ret = -1;
			break;
		}
		udelay(1);
	} while(q->irq == 1);
	return ret;
}
int dxrt_enqueue(dxrt_queue_t* q, void *elem)
{
	int ret;
	pr_debug( "%s: %d\n", __func__, ((dxrt_request_acc_t*)elem)->req_id);
	if (!q->enable) return -EINVAL;
	if (dxrt_is_queue_full(q))
	{
		ret = -1;
		pr_debug( "%s: %d rejected.\n", __func__, ((dxrt_request_acc_t*)elem)->req_id);
	}
	else
	{
		if (dxrt_lock_queue(q)) {
			memcpy_toio(
				(void*)q + buffer_offset + (q->rear * q->elem_size), 
				elem,
				q->elem_size
			);
			q->rear = (q->rear + 1) % q->max_count;
			q->count++;
			ret = 0;
			if (q->count == DX_QUEUE_THRESHOLD_FOR_INTERRUPT)
				dxrt_enqueue_irq_notify(q);
			pr_debug( "%s: %d/%d/%d done.\n", __func__, ((dxrt_request_acc_t*)elem)->req_id, q->front, q->rear);
			dxrt_unlock_queue(q);
		} else {
			ret = -EINVAL;
		}
	}
	return ret;
}
int dxrt_is_request_list_empty(dxrt_request_list_t *requests, spinlock_t *lock)
{
	int empty;
	spin_lock(lock);
	empty = list_empty(&requests->list);
	spin_unlock(lock);
	return empty;
}