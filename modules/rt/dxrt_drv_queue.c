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
#if 0
	return q->count == 0;
#else
	volatile uint32_t val;
	void __iomem *count = &q->count;
	val = readl(count);
	return val == 0;
#endif
}
int dxrt_is_queue_full(dxrt_queue_t* q)
{
#if 0
	return q->count == q->max_count;
#else
	volatile uint32_t val;
	void __iomem *count = &q->count;
	val = readl(count);
	return val == q->max_count;
#endif
}
void dxrt_lock_queue(dxrt_queue_t *q)
{
#if 0
	while(q->lock);
	q->lock = 1;
#else	
	volatile uint32_t val;
	void __iomem *lock = &q->lock;
	while(1)
	{
		val = readl(lock);
		if(val==0)
			break;
		else
		{
			pr_debug( "%s: wait for lock.\n", __func__);
			udelay(100);
		}
	}
	writel(33, lock);
#endif
}
void dxrt_unlock_queue(dxrt_queue_t *q)
{	
#if 0
	q->lock = 0;
#else
	void __iomem *lock = &q->lock;
	writel(0, lock);
#endif
}
int dxrt_enqueue(dxrt_queue_t* q, void *elem)
{
	int ret;
	pr_debug( "%s: %d\n", __func__, ((dxrt_request_acc_t*)elem)->req_id);
	if(!q->enable) return 0;
	if(dxrt_is_queue_full(q))
	{
		ret = -1;
		pr_debug( "%s: %d rejected.\n", __func__, ((dxrt_request_acc_t*)elem)->req_id);
	}
	else
	{
		dxrt_lock_queue(q);
		memcpy(
			(void*)q + buffer_offset + (q->rear * q->elem_size), 
			elem,
			q->elem_size
		);
		q->rear = (q->rear + 1) % q->max_count;
		q->count++;
		ret = 0;
		pr_debug( "%s: %d done.\n", __func__, ((dxrt_request_acc_t*)elem)->req_id);
		dxrt_unlock_queue(q);
	}
	return ret;
}
int dxrt_dequeue(dxrt_queue_t* q, void *elem)
{
	int ret;
	if(!q->enable) return 0;
	dxrt_lock_queue(q);
	if(dxrt_is_queue_empty(q))
	{
		ret = -1;
	}
	else
	{
		memcpy(
			elem,
			(void*)q + buffer_offset + (q->front * q->elem_size), 
			q->elem_size
		);
		q->front = (q->front + 1) % q->max_count;
		q->count--;
		ret = 0;
	}
	dxrt_unlock_queue(q);
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