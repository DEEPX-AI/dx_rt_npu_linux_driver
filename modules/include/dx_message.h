// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef _DX_MESSAGE_H
#define _DX_MESSAGE_H

#include <linux/types.h>
#include <linux/io.h>

#define DX_PCIE_RESP_NUM    (3)

/* DMA error codes (must match dxrt_error_t in dxrt_drv.h) */
#define ERR_PCIE_DMA_FAIL_BASE   100
#define ERR_PCIE_DMA_CH_FAIL(ch)  (ERR_PCIE_DMA_FAIL_BASE + (ch))
#define ERR_PCIE_DMA_ABORT_BASE  400
#define ERR_PCIE_DMA_CH_ABORT(ch) (ERR_PCIE_DMA_ABORT_BASE + (ch))

/*
 * Event type values for dx_pcie_dev_event_t.event_type.
 * Must stay in sync with dxrt_event_type_t in dxrt_drv.h.
 */
#define DX_EVENT_TYPE_ERROR     1

typedef struct {
    uint32_t  req_id;
    uint32_t  inf_time;
    uint16_t  argmax;
    uint8_t   model_type;
    uint8_t   model_format;
    int32_t   status;
    int32_t   ppu_filter_num;
    uint32_t  proc_id;
    uint32_t  queue;
    int32_t   dma_ch;
    uint64_t  data;
    uint64_t  base;
    uint32_t  offset;
    uint32_t  size;
    uint32_t  ddr_rd_bw;
    uint32_t  ddr_wr_bw;
} dx_pcie_response_t;

typedef struct
{
    struct list_head list;
    dx_pcie_response_t response;
} dx_pcie_response_list_t;

typedef struct {
    uint32_t err_code;

    /* Version */
    uint32_t fw_ver;
    uint32_t rt_driver_version;
    uint32_t pcie_driver_version;
    uint32_t reserved_ver[4];

    /* Npu information */
    uint32_t npu_id;
    uint64_t base_axi;
    uint32_t base_rmap;
    uint32_t base_weight;
    uint32_t base_in;
    uint32_t base_out;
    uint32_t cmd_num;
    uint32_t last_cmd;
    uint32_t busy;
    uint32_t abnormal_cnt;
    uint32_t irq_status;
    uint32_t dma_err;
    uint32_t reserved_npu[10];

    /* System infomation power / temperature, etc,,,, */
    uint32_t temperature[20];
    uint32_t npu_voltage[4];
    uint32_t npu_freq[4];
    uint32_t reserved_sys[10];

    /* PCIe information */
    uint8_t  bus;
    uint8_t  dev;
    uint8_t  func;
    uint8_t  reserved;
    int      speed; /* GEN1, GEN2...*/
    int      width; /* 1, 2, 4 */
    uint32_t ltssm;
    uint32_t dma_rd_ch_sts[4];
    uint32_t dma_wr_ch_sts[4];
    uint32_t reserved_pcie[2];
    /* Extra Version information */
    char     rt_driver_version_suffix[16];
    char     fw_version_suffix[16];

    /* DDR information */
    uint32_t ddr_temperature[4];
    uint32_t reserved_ddr[10];

} dx_pcie_dev_err_t;

typedef struct {
    uint32_t ntfy_code;
    uint32_t npu_id;
    uint32_t throt_voltage[2];      // [0] current, [1] target
    uint32_t throt_freq[2];         // [0] current, [1] target
    uint32_t throt_temper;
} dx_pcie_dev_ntfy_throt_t;

typedef struct {
    uint32_t proc_id;               /* Process ID that exited */
} dx_pcie_dev_proc_exit_t;

/* Queue for PROC_EXIT events (multiple processes can exit simultaneously) */
#define DX_PROC_EXIT_QUEUE_SIZE  16
typedef struct {
    dx_pcie_dev_proc_exit_t entries[DX_PROC_EXIT_QUEUE_SIZE];
    int head;
    int tail;
    int count;
} dx_pcie_proc_exit_queue_t;

typedef struct {
    uint32_t            event_type;
    union {
        dx_pcie_dev_err_t           dx_rt_err;
        dx_pcie_dev_ntfy_throt_t    dx_rt_ntfy_throt;
        dx_pcie_dev_proc_exit_t     dx_rt_proc_exit;
    };
} dx_pcie_dev_event_t;

/* Circular buffer for device events (replaces single-slot event_response) */
#define DX_EVENT_QUEUE_SIZE  32
typedef struct {
    dx_pcie_dev_event_t entries[DX_EVENT_QUEUE_SIZE];
    int head;
    int tail;
    int count;
} dx_pcie_event_queue_t;

struct dx_pcie_msg {
    void __iomem                *response[DX_PCIE_RESP_NUM];
    void __iomem                *events;
    void __iomem                *irq_status;
    void __iomem                *notify;        /* generate irq to device */
    dx_pcie_response_list_t     responses[DX_PCIE_RESP_NUM];
    dx_pcie_event_queue_t       event_queue;      /* Circular buffer for device events */
    dx_pcie_proc_exit_queue_t   proc_exit_queue;  /* Queue for PROC_EXIT events */
    spinlock_t                  responses_lock[DX_PCIE_RESP_NUM];
    spinlock_t                  event_lock;
    spinlock_t                  proc_exit_lock;   /* Lock for proc_exit_queue */
};

void dx_pcie_enqueue_response(u32 dev_id, int dma_ch);
int dx_pcie_message_init(int dev_id);
void dx_pcie_enqueue_event_response(u32 dev_id, uint32_t err_code);
void dx_pcie_enqueue_abort_event(u32 dev_id, uint32_t err_code,
				 uint32_t dma_err_status,
				 const uint32_t *wr_ch_sts,
				 const uint32_t *rd_ch_sts);
void dx_pcie_clear_proc_exit_for_pid(u32 dev_id, pid_t proc_id);

/* Callback types for notifying RT module from ISR context */
typedef void (*dx_pcie_response_cb_t)(u32 dev_id, int dma_ch, void *data);
typedef void (*dx_pcie_event_cb_t)(u32 dev_id, void *data);

void dx_pcie_register_response_callback(u32 dev_id, dx_pcie_response_cb_t cb, void *data);
void dx_pcie_unregister_response_callback(u32 dev_id);
void dx_pcie_register_event_callback(u32 dev_id, dx_pcie_event_cb_t cb, void *data);
void dx_pcie_unregister_event_callback(u32 dev_id);

#endif
