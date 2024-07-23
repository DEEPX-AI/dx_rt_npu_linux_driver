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

/* MSG : DX_PCIE_CMD_IDENTIFY_DEVICE */
typedef struct _msg_mem_info {
    uint64_t memAddr;
    uint64_t memSize;
} msg_mem_info;

enum pcie_msg_type {
    DX_PCIE_CMD_IDENTIFY_DEVICE,    /* Dir : Recv */
    DX_PCIE_CMD_NPU_RUN,            /* Dir : Send */
    DX_PCIE_CMD_MAX,
};

/* Response datas from device */
typedef struct pcie_output_info {
    uint32_t req_id;
    uint64_t data;
    uint64_t base;
    uint32_t offset;
    uint32_t size;
} __packed pcie_output_info ;

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
ssize_t dx_sgdma_write(char *dest, u64 src, size_t count, int dev_id, int dma_ch, bool npu_run);
ssize_t dx_sgdma_read(char *src, u64 dest, size_t count, int dev_id, int dma_ch);
unsigned int dx_pcie_interrupt(int dev_id, int irq_id);
unsigned int dx_pcie_interrupt_wakeup(int dev_id, int irq_id);
void __iomem *dx_pcie_get_message_area(u32 dev_id);
void __iomem *dx_pcie_get_log_area(u32 dev_id);
void __iomem *dx_pcie_get_request_queue(u32 dev_id, u32 priority);
void __iomem *dx_pcie_get_response_queue(u32 dev_id, int dma_ch);
int dx_pcie_clear_response_queue(u32 dev_id);
int dx_pcie_is_response_queue_empty(u32 dev_id, int dma_ch);
int dx_pcie_dequeue_response(u32 dev_id, int dma_ch, dx_pcie_response_t* response);
uint32_t dx_pcie_get_dev_num(void);
u64 dx_pcie_get_download_region(int dev_id);
u32 dx_pcie_get_download_size(int dev_id);
void dx_pcie_dequeue_error_response(u32 dev_id, dx_pcie_dev_err_t* response);
void dx_pcie_enqueue_error_response(u32 dev_id, uint32_t err_code);
void dx_pcie_get_driver_info(struct deepx_pcie_info *info, int dev_id);
void dx_pcie_notify_msg_to_device(u32 dev_id);
int dx_pcie_notify_req_to_device(u32 dev_id, u32 priority);

#endif /*_DX_PCIE_API_H*/
