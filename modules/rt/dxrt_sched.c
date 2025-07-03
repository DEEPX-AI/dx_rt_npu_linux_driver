// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *  -> This module is responsible for scheduling 
 *     which queue data will be delivered to based on the scheduler option provided by the user.
 *  -> This API is supported only on accelerator devices.
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#include "dxrt_drv.h"
#include "dxrt_version.h"

/**
 * add_queue_from_sched_op 
 *  - Save option to list of dev if option is not duplicated in list
 * @dev: The deepx device on kernel structure
 * @op:  Bound option of scheduler
 * 
 * Return: 0 on success,
 *        -ENOMEM   if an error occurs as memory allocation fail
 *        -EBUSY    There are no queues to assign to the user.
 *                  User scheduler options need modification.
 *        -EEXIST   The same bound option is in the queue, so it is not added.
 */
int add_queue_from_sched_op(struct dxdev* dev, npu_bound_op bound)
{
    dx_sched_list *n_node = kmalloc(sizeof(dx_sched_list), GFP_KERNEL);
    int ret = 0;
    int min_available_queue = 0;
    bool found = false;
    dx_sched_list *entry;

    if (!n_node) {
        pr_err("%s: Failed to allocate memory\n", __func__);
        return -ENOMEM;
    }

    spin_lock(&dev->sched_lock);

    list_for_each_entry(entry, &dev->sched, list) {
        if (entry->data.bound == bound) {
            entry->data.ref_cnt++;
            pr_debug("%s:%d add ref_cnt:%d\n", __func__, dev->id, entry->data.ref_cnt);
            kfree(n_node);
            ret = -EEXIST;
            found = true;
            break;
        }
        if (entry->data.queue == min_available_queue) {
            min_available_queue++;
        }
    }

    if (!found) {
        if (min_available_queue >= DX_NORMAL_QUEUE_MAX) {
            pr_err("%s:%d Queue limit exceeded for bound:%d\n", __func__, dev->id, bound);
            kfree(n_node);
            ret = -EBUSY;
        } else {
            n_node->data.bound = bound;
            n_node->data.ref_cnt = 1;
            n_node->data.queue = min_available_queue;
            list_add_tail(&n_node->list, &dev->sched);
            pr_debug("%s:%d add bound:%d, queue:%d, ref_cnt:%d\n",
                    __func__, dev->id, bound, n_node->data.queue, n_node->data.ref_cnt);
            ret = 0;
        }
    }
    spin_unlock(&dev->sched_lock);
    return ret;
}

/**
 * get_queue_from_sched_op 
 *  - Retrieves the queue to use based on option information.
 * @dev: The deepx device on kernel structure
 * @op:  Bound option of scheduler
 * 
 * Return: 0 on success,
 *        -ENOENT   There are no matching queues in the list.
 */
int get_queue_from_sched_op(struct dxdev* dev, npu_bound_op bound, uint32_t *q)
{
    dx_sched_list *entry;
    req_queue_t queue = DX_NORMAL_QUEUE_MAX;
    int ret = 0;

    spin_lock(&dev->sched_lock);
    list_for_each_entry(entry, &dev->sched, list) {
        if (bound == entry->data.bound) {
            queue = entry->data.queue;
            *q = queue;
            break;
        }
    }
    if (queue == DX_NORMAL_QUEUE_MAX) {
        ret = -ENOENT;
    }
    spin_unlock(&dev->sched_lock);
    return ret;
}

/**
 * delete_matching_queue 
 *  - Delete the queue to use based on option information.
 * @dev: The deepx device on kernel structure
 * @op:  Bound option of scheduler
 * 
 * Return: 0 on success,
 *        -ENOENT   There are no matching queues in the list.
 *        -EBUSY    The bounding option will not be deleted 
 *                  because some process is referencing it.
 */
int delete_matching_queue(struct dxdev* dev, npu_bound_op bound)
{
    dx_sched_list *entry, *tmp;
    bool found = false;
    int ret = 0;

    spin_lock(&dev->sched_lock);
    list_for_each_entry_safe(entry, tmp, &dev->sched, list) {
        if (entry->data.bound == bound) {
            pr_debug("%s:%d delete bound:%d, entry_bound:%d, queue:%d, ref_cnt:%d\n",
                    __func__, dev->id, bound, entry->data.bound, entry->data.queue, entry->data.ref_cnt);
            if (--entry->data.ref_cnt == 0) {
                list_del(&entry->list);
                kfree(entry);
            } else {
                ret = -EBUSY;
            }
            found = true;
            break;
        }
    }
    spin_unlock(&dev->sched_lock);
    if (!found) {
        pr_err("%s: No matching entries found for op: %d\n", __func__, bound);
        ret = -ENOENT;
    }
    return ret;
}

/**
 * clear_queue_list 
 *  - Delete the queue list.
 * @dev: The deepx device on kernel structure
 */
void clear_queue_list(struct dxdev* dev)
{
    dx_sched_list *entry, *tmp;

    spin_lock(&dev->sched_lock);
    list_for_each_entry_safe(entry, tmp, &dev->sched, list) {
        list_del(&entry->list);
        kfree(entry);
    }
    spin_unlock(&dev->sched_lock);
}
