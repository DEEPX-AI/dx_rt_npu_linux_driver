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

typedef enum dxqueue_flag_t {
    DX_FLAG_UNLOCK  = 0,
    DX_FLAG_LOCK    = 1,
} dxqueue_flag;

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

int dxrt_lock_queue(dxrt_queue_t __iomem* q)
{
    int ret = 1;
    writel(DX_QUEUE_HOST_LOCK, &q->lock);
    return ret;
}

#define DXRT_ENQUEUE_TIMEOUT        (100000) /* 100ms */
#define DXRT_ENQUEUE_DELAY          (1)
int dxrt_lock_check(dxrt_queue_t __iomem* q)
{
    int timeout = DXRT_ENQUEUE_TIMEOUT;
    volatile uint32_t val;
    int ret = 1;

    while(timeout >= 0) {
        val = readl(&q->flag);
        if (val == DX_FLAG_LOCK) {
                break;
        } else {
            timeout--;
            udelay(DXRT_ENQUEUE_DELAY);
        }
    }
    if (timeout < 0) {
        ret = 0;
        pr_err("%s: lock check failed(%d)\n", __func__, timeout);
    }
    return ret;
}
void dxrt_unlock_queue(dxrt_queue_t __iomem* q)
{    
    writel(DX_QUEUE_UNLOCK, &q->lock);
    writel(DX_FLAG_UNLOCK, &q->flag);
}

void dxrt_enqueue_irq_notify(dxrt_queue_t __iomem* q)
{
    void __iomem *irq_done = &q->irq_done;
    writel(1, irq_done);
}

int dxrt_enqueue_irq_done(dxrt_queue_t __iomem* q)
{
    int timeout = 100000;
    int ret = 0;
    do {
        if (timeout-- < 0) {
            ret = -1;
            break;
        }
        udelay(1);
    } while(readl(&q->irq_done) == 1);
    return ret;
}

int dxrt_enqueue(dxrt_queue_t __iomem* q, void *elem)
{
    int ret = 0;
    pr_debug( "%s: %d\n", __func__, ((dxrt_request_acc_t*)elem)->req_id);
    if (!q->enable) return -EINVAL;
    else
    {
        memcpy_toio(
            (void __iomem*)q + buffer_offset + (q->rear * q->elem_size), 
            elem,
            q->elem_size
        );
        q->rear = (q->rear + 1) % q->max_count;
        q->acces_count++;
        q->count++;
        dxrt_enqueue_irq_notify(q);
        pr_debug( "%s: %d f:%d r:%d c:%d a:%d done.\n",
            __func__, ((dxrt_request_acc_t*)elem)->req_id,
            q->front, q->rear, q->count, q->acces_count);
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