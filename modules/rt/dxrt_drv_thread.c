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

int dxrt_request_handler(void *data)
{
	struct dxdev *dx = (struct dxdev*)data;
	struct dxnpu *npu = dx->npu;
    int num = dx->id;
    dxrt_request_list_t *entry;
    // dxrt_request_t *req;
    pr_debug( MODULE_NAME "%d: %s start.\n", num, __func__);
    while(!kthread_should_stop())
    {
        wait_event_interruptible(
            dx->request_wq,
            !dxrt_is_request_list_empty(&dx->requests, &dx->requests_lock) || kthread_should_stop()            
        );
        if(kthread_should_stop()) break;
        pr_debug( MODULE_NAME "%d: %s wake up.\n", num, __func__);        
        spin_lock(&dx->requests_lock);
        entry = list_first_entry(&dx->requests.list, dxrt_request_list_t, list);            
            // req = &entry->request;
            // pr_debug( MODULE_NAME "%d: %s: req %d, type %d, %d cmds @ %x, weight @ %x, input %llx @ %llx+%x(%x), output %llx @ %llx+%x(%x)\n", 
            //     num, __func__, req->req_id, req->model_type, req->model_cmds,
            //     req->cmd_offset, req->weight_offset,
            //     req->input.data, req->input.base, req->input.offset, req->input.size,
            //     req->output.data, req->output.base, req->output.offset, req->output.size
            // );
        spin_unlock(&dx->requests_lock);
        npu->run(npu, &entry->request);
        spin_lock(&dx->requests_lock);
        list_del(&entry->list);
        spin_unlock(&dx->requests_lock);
        kfree(entry);
    }
    pr_debug( MODULE_NAME "%d: %s end.\n", num, __func__);
    return 0;
}