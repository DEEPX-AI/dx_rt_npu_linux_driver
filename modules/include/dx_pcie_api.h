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
int dx_sgdma_init(int dev_id);
void dx_sgdma_deinit(int dev_id);
int dx_pcie_reset_dma_channels(int dev_id);
ssize_t dx_sgdma_write(char *dest, u64 src, size_t count, int dev_id, int dma_ch, bool npu_run, enum mem_type type, dma_addr_t dma_addr);
ssize_t dx_sgdma_read(char *src, u64 dest, size_t count, int dev_id, int dma_ch, enum mem_type type);
unsigned int dx_pcie_interrupt(int dev_id, int irq_id);
void dx_pcie_interrupt_clear(int dev_id, int irq_id);
unsigned int dx_pcie_interrupt_wakeup(int dev_id, int irq_id);
unsigned int dx_pcie_interrupt_event_wakeup(int dev_id);
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
bool dx_pcie_test_and_clear_init_completed(int dev_id);
void dx_pcie_set_init_completed(int dev_id);
void dx_pcie_dequeue_event_response(u32 dev_id, dx_pcie_dev_event_t* response);
int dx_pcie_is_event_pending(u32 dev_id);
void dx_pcie_enqueue_event_response(u32 dev_id, uint32_t err_code);
void dx_pcie_enqueue_recovery_event(u32 dev_id, uint32_t subcode,
				    uint32_t reason,
				    uint32_t recovery_count,
				    uint32_t recovery_fail_count,
				    uint32_t dev_state);
/*
 * dx_pcie_enqueue_abort_event - Enqueue an ERROR event carrying an
 * err_code plus DMA channel-status snapshot.  Originally used for
 * true DMA aborts; reused by the RT module's link-flap path with
 * err_code=ERR_PCIE_LINK_DOWN_CH<n> (== ERR_PCIE_DMA_CH<n>_ABORT)
 * and zeroed status fields so legacy user-space sees a familiar
 * abort shape while new handlers can distinguish link-flap via
 * the parallel RECOVERY event.
 */
void dx_pcie_enqueue_abort_event(u32 dev_id, uint32_t err_code,
				 uint32_t dma_err_status,
				 const uint32_t *wr_ch_sts,
				 const uint32_t *rd_ch_sts);
void dx_pcie_clear_event_response(u32 dev_id);
void dx_pcie_get_driver_info(struct deepx_pcie_info *info, int dev_id);
void dx_pcie_notify_msg_to_device(u32 dev_id);
int dx_pcie_notify_req_to_device(u32 dev_id, u32 queue, u8 lock);

/* Response/Event callbacks from ISR to RT module */
void dx_pcie_register_response_callback(u32 dev_id, dx_pcie_response_cb_t cb, void *data);
void dx_pcie_unregister_response_callback(u32 dev_id);
void dx_pcie_register_event_callback(u32 dev_id, dx_pcie_event_cb_t cb, void *data);
void dx_pcie_unregister_event_callback(u32 dev_id);

/* Link-state event callback — fires when link-health worker
 * detects an EP-initiated link drop / restore. */
void dx_pcie_register_link_event_callback(u32 dev_id,
					  dx_pcie_link_event_cb_t cb,
					  void *data);
void dx_pcie_unregister_link_event_callback(u32 dev_id);

#endif /*_DX_PCIE_API_H*/
