// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef _DX_PCIE_API_H
#define _DX_PCIE_API_H

#include <linux/poll.h>
#include <dx_message.h>

enum mem_type {
	USER_SPACE_BUF   = 0,
	KERNEL_SPACE_BUF = 1,
};

typedef enum _dx_msg_dir_t {
    DX_MSG_SEND = 1,
    DX_MSG_RECV = 2,
} dx_msg_dir_t;

/* Request queue type */
typedef enum _req_queue_t {
    DX_NORMAL_QUEUE0    = 0,
    DX_NORMAL_QUEUE1    = 1,
    DX_NORMAL_QUEUE2    = 2,
    DX_NORMAL_QUEUE_MAX = 3,
    DX_HIGH_QUEUE       = 3,
    DX_QUEUE_MAX        = 4
} req_queue_t;

/* PCIe Information */
struct deepx_pcie_info {
    unsigned int    driver_version;
    unsigned char   bus;
    unsigned char   dev;
    unsigned char   func;
    int             speed; /* GEN1, GEN2...*/
    int             width; /* 1, 2, 4 */
};

/* PCIe EXternal API */
void dx_sgdma_init(int dev_id);
void dx_sgdma_deinit(int dev_id);
ssize_t dx_sgdma_write(char *dest, u64 src, size_t count, int dev_id, int dma_ch, bool npu_run, enum mem_type type, dma_addr_t dma_addr);
ssize_t dx_sgdma_read(char *src, u64 dest, size_t count, int dev_id, int dma_ch, enum mem_type type);
unsigned int dx_pcie_interrupt(int dev_id, int irq_id);
void dx_pcie_interrupt_clear(int dev_id, int irq_id);
unsigned int dx_pcie_interrupt_wakeup(int dev_id, int irq_id);
void __iomem *dx_pcie_get_message_area(u32 dev_id);
void __iomem *dx_pcie_get_log_area(u32 dev_id);
void __iomem *dx_pcie_get_dl_area(u32 dev_id);
void __iomem *dx_pcie_get_request_queue(u32 dev_id, u32 priority);
void __iomem *dx_pcie_get_response_queue(u32 dev_id, int dma_ch);
int dx_pcie_clear_response_queue(u32 dev_id);
int dx_pcie_is_response_queue_empty(u32 dev_id, int dma_ch);
int dx_pcie_dequeue_response(u32 dev_id, int dma_ch, dx_pcie_response_t* response);
uint32_t dx_pcie_get_dev_num(void);
u64 dx_pcie_get_download_region(int dev_id);
u32 dx_pcie_get_download_size(int dev_id);
u64 dx_pcie_get_booting_region(int dev_id, int id);
bool dx_pcie_get_init_completed(int dev_id);
void dx_pcie_set_init_completed(int dev_id);
void dx_pcie_dequeue_event_response(u32 dev_id, dx_pcie_dev_event_t* response);
void dx_pcie_enqueue_event_response(u32 dev_id, uint32_t err_code);
void dx_pcie_clear_event_response(u32 dev_id);
void dx_pcie_get_driver_info(struct deepx_pcie_info *info, int dev_id);
void dx_pcie_notify_msg_to_device(u32 dev_id);
int dx_pcie_notify_req_to_device(u32 dev_id, u32 queue, u8 lock);

#endif /*_DX_PCIE_API_H*/
