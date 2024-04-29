// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */
#include <linux/mutex.h>
#include <linux/delay.h>

#include "dx_pcie_api.h"

void dx_sgdma_init(int dev_id) {}
EXPORT_SYMBOL_GPL(dx_sgdma_init);
void dx_sgdma_deinit(int dev_id) {}
EXPORT_SYMBOL_GPL(dx_sgdma_deinit);

extern ssize_t api_char_sgdma_read_write(int id, const char __user *buf,
		size_t count, loff_t *pos, bool write);
ssize_t dx_sgdma_write(char *dest, u64 src, size_t count, int dev_id, int dma_ch, bool npu_run)
{
    // u64 pos = src;
    return api_char_sgdma_read_write(dev_id, dest, count, &src, 1);
}
EXPORT_SYMBOL_GPL(dx_sgdma_write);

ssize_t dx_sgdma_read(char *src, u64 dest, size_t count, int dev_id, int dma_ch)
{
    // u64 pos = dest;
    return api_char_sgdma_read_write(dev_id, src, count, &dest, 0);
}
EXPORT_SYMBOL_GPL(dx_sgdma_read);

unsigned int dx_pcie_interrupt(int dev_id, int irq_id) { return 0; }
EXPORT_SYMBOL_GPL(dx_pcie_interrupt);
unsigned int dx_pcie_interrupt_wakeup(int dev_id, int irq_id) { return 0; }
EXPORT_SYMBOL_GPL(dx_pcie_interrupt_wakeup);

int dx_pcie_send_message(u32 cmd, u32 dir, u32 size, u32 dev_id, void *data) { return 0; }
EXPORT_SYMBOL_GPL(dx_pcie_send_message);
int dx_pcie_get_response(u32 dev_id, u32 npu_id, void *info, u32 size) { return 0; }
EXPORT_SYMBOL_GPL(dx_pcie_get_response);

void __iomem *dx_pcie_get_message_area(u32 dev_id) { return NULL; }
EXPORT_SYMBOL_GPL(dx_pcie_get_message_area);
void __iomem *dx_pcie_get_log_area(u32 dev_id) { return NULL; }
EXPORT_SYMBOL_GPL(dx_pcie_get_log_area);
void __iomem *dx_pcie_get_request_queue(u32 dev_id) { return NULL; }
EXPORT_SYMBOL_GPL(dx_pcie_get_request_queue);
void __iomem *dx_pcie_get_response_queue(u32 dev_id, int dma_ch) { return NULL; }
EXPORT_SYMBOL_GPL(dx_pcie_get_response_queue);
int dx_pcie_clear_response_queue(u32 dev_id) { return 0; }
EXPORT_SYMBOL_GPL(dx_pcie_clear_response_queue);
int dx_pcie_is_response_queue_empty(u32 dev_id, int dma_ch) { return 0; }
EXPORT_SYMBOL_GPL(dx_pcie_is_response_queue_empty);
void dx_pcie_enqueue_response(u32 dev_id, int dma_ch) { }
EXPORT_SYMBOL_GPL(dx_pcie_enqueue_response);
int dx_pcie_dequeue_response(u32 dev_id, int dma_ch, dx_pcie_response_t* response) { return 0; }
EXPORT_SYMBOL_GPL(dx_pcie_dequeue_response);
void dx_pcie_enqueue_error_response(u32 dev_id, uint32_t err_code) { }
EXPORT_SYMBOL_GPL(dx_pcie_enqueue_error_response);
void dx_pcie_dequeue_error_response(u32 dev_id, dx_pcie_dev_err_t* response) { }
EXPORT_SYMBOL_GPL(dx_pcie_dequeue_error_response);
void dx_pcie_get_driver_info(struct deepx_pcie_info *info, int dev_id) { }
EXPORT_SYMBOL_GPL(dx_pcie_get_driver_info);