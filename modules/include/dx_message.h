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
 * Must stay in sync with dxrt_event_t in dxrt_drv.h.
 */
#define DX_EVENT_TYPE_ERROR         1
#define DX_EVENT_TYPE_NOTIFY_THROT  2
#define DX_EVENT_TYPE_RECOVERY      3

/*
 * Sub-codes carried inside DX_EVENT_TYPE_RECOVERY.
 *
 * Surface the stage of a recovery pass so user-space can distinguish
 * "driver is handling it" from "driver gave up, needs human/service
 * action".
 *
 * See: docs/recovery/RECOVERY_IMPLEMENTATION_PLAN.md §3
 */
enum dx_recovery_subcode {
	DX_RECOVERY_STARTED        = 1, /* link-down / reset_prepare observed  */
    DX_RECOVERY_DONE           = 2, /* link + FW readiness probe both OK   */
    DX_RECOVERY_FW_HANG        = 3, /* FW readiness probe failed           */
    DX_RECOVERY_PERM_FAIL      = 4, /* retry threshold exceeded            */
};

/*
 * Reason codes for "why did we enter recovery this time" — surfaced
 * to sysfs (last_recovery_reason) and embedded in the RECOVERY event
 * payload so user-space does not have to correlate with dmesg.
 */
enum dx_recovery_reason {
	DX_RECOVERY_REASON_NONE          = 0,
    DX_RECOVERY_REASON_LINK_FLAP     = 1,
    DX_RECOVERY_REASON_FW_TIMEOUT    = 2,
    DX_RECOVERY_REASON_CPU_RESET     = 3,
    DX_RECOVERY_REASON_FW_UPDATE     = 4,
    /*
     * FW update to a legacy image (< 2.7.0) that does not publish the
     * DLMSG boot-aware readiness block.  Recovery must skip the
     * MAILBOX_READY edge gate and rely on PING only.
     */
    DX_RECOVERY_REASON_FW_UPDATE_NODLMSG = 5,
};

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

/*
 * Recovery event payload — union sibling inside dx_pcie_dev_event_t.
 *
 * Keeps the overall event struct size dominated by dx_pcie_dev_err_t
 * (already ~200 bytes), so adding this member does NOT change ABI for
 * user-space code that reads event_type and switches on it.
 */
typedef struct {
    uint32_t subcode;             /* enum dx_recovery_subcode        */
    uint32_t reason;              /* enum dx_recovery_reason         */
    uint32_t recovery_count;      /* cumulative successful reinits   */
    uint32_t recovery_fail_count; /* cumulative failed attempts      */
    uint32_t dev_state;           /* dxrt_dev_state_t (see dxrt_drv.h) */
    uint32_t reserved[3];
} dx_pcie_dev_recovery_t;

typedef struct {
    uint32_t            event_type;
    union {
        dx_pcie_dev_err_t           dx_rt_err;
        dx_pcie_dev_ntfy_throt_t    dx_rt_ntfy_throt;
        dx_pcie_dev_recovery_t      dx_rt_recovery;
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
    spinlock_t                  responses_lock[DX_PCIE_RESP_NUM];
    spinlock_t                  event_lock;
};

void dx_pcie_enqueue_response(u32 dev_id, int dma_ch);
int dx_pcie_message_init(int dev_id);
void dx_pcie_enqueue_event_response(u32 dev_id, uint32_t err_code);
void dx_pcie_enqueue_recovery_event(u32 dev_id, uint32_t subcode,
				    uint32_t reason,
				    uint32_t recovery_count,
				    uint32_t recovery_fail_count,
				    uint32_t dev_state);
void dx_pcie_enqueue_abort_event(u32 dev_id, uint32_t err_code,
				 uint32_t dma_err_status,
				 const uint32_t *wr_ch_sts,
				 const uint32_t *rd_ch_sts);
/* Callback types for notifying RT module from ISR context */
typedef void (*dx_pcie_response_cb_t)(u32 dev_id, int dma_ch,
                      uint32_t proc_id, void *data);
typedef void (*dx_pcie_event_cb_t)(u32 dev_id, void *data);

/* Link-state transitions observed by the PCIe module's link-health
 * worker.  Delivered to the RT module so it can block new ioctls,
 * wake waiters with -ENODATA, and clear stale response queues.
 * No SBR/AER callback fires for EP-initiated link flaps, so this
 * callback is the only signal the RT module gets. */
typedef enum {
	DX_PCIE_LINK_EV_DOWN = 0, /* link dropped — set recovering=1, wake waiters */
	DX_PCIE_LINK_EV_UP   = 1, /* link restored + eDMA reinit done — clear recovering */
} dx_pcie_link_event_t;

typedef void (*dx_pcie_link_event_cb_t)(u32 dev_id,
					dx_pcie_link_event_t ev,
					void *data);

void dx_pcie_register_response_callback(u32 dev_id, dx_pcie_response_cb_t cb, void *data);
void dx_pcie_unregister_response_callback(u32 dev_id);
void dx_pcie_register_event_callback(u32 dev_id, dx_pcie_event_cb_t cb, void *data);
void dx_pcie_unregister_event_callback(u32 dev_id);
void dx_pcie_register_link_event_callback(u32 dev_id,
					  dx_pcie_link_event_cb_t cb,
					  void *data);
void dx_pcie_unregister_link_event_callback(u32 dev_id);
void dx_pcie_notify_link_event(u32 dev_id, dx_pcie_link_event_t ev);

#endif
