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
 * set_queue_from_sched_op 
 *  - Save option to list of dev if option is not duplicated in list
 * @dev: The deepx device on kernel structure
 * @op:  Bound option of scheduler
 * 
 * Return: 0 on success,
 *        -ENOMEM   if an error occurs as memory allocation fail
 *        -EBUSY    There are no queues to assign to the user.
 *                  User scheduler options need modification.
 */
int set_queue_from_sched_op(struct dxdev* dev, npu_bound_op bound)
{
    dx_sched_list *n_node = kmalloc(sizeof(dx_sched_list), GFP_KERNEL);
    int ret = 0, save = 0;

    if (!n_node) {
        pr_err("%s: Failed to allocated memory\n", __func__);
        return -ENOMEM;
    }
    spin_lock(&dev->sched_lock);
    n_node->data.bound = bound;
    if (list_empty(&dev->sched)) {
        n_node->data.queue = DX_NORMAL_QUEUE0;
        n_node->data.bound = bound;
        save = 1;
    } else {
        dx_sched_list *entry;
        req_queue_t prev_que;
        list_for_each_entry(entry, &dev->sched, list) {
            prev_que = entry->data.queue;
            if (entry->data.bound != bound) {
                n_node->data.queue = prev_que++;
                save = 1;
                if (n_node->data.queue > DX_NORMAL_QUEUE_MAX) {
                    ret = -EBUSY;
                }
            }
        }
    }
    if (save && ret == 0) list_add_tail(&n_node->list, &dev->sched);
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
 * delete_queue_list 
 *  - Delete the queue to use based on option information.
 * @dev: The deepx device on kernel structure
 * @op:  Bound option of scheduler
 * 
 * Return: 0 on success,
 *        -ENOENT   There are no matching queues in the list.
 */
int delete_matching_queue_list(struct dxdev* dev, npu_bound_op bound)
{
    dx_sched_list *entry, *tmp;
    bool found = false;
    int ret = 0;

    spin_lock(&dev->sched_lock);
    list_for_each_entry_safe(entry, tmp, &dev->sched, list) {
        if (bound == entry->data.bound) {
            list_del(&entry->list);
            kfree(entry);
            found = true;
        }
    }
    spin_unlock(&dev->sched_lock);
    if (!found) {
        pr_err("%s:No matching entries found for op: %d\n", __func__, bound);
        ret = -ENOENT;
    }
    return ret;
}