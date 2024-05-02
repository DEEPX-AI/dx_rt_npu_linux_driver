// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#ifndef __DXRT_DRV_H
#define __DXRT_DRV_H

#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/reset.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>

#include "dxrt_drv_common.h"
#include "dxrt_drv_npu.h"

#if IS_ACCELERATOR
#include "dx_pcie_api.h"
#endif
#if IS_STANDALONE
/* L2 cache flush api */
#include <asm/sbi.h>
#endif
#if DEVICE_VARIANT==DX_L1
#include <soc/eyenix/cdma.h>
#endif

#define MODULE_NAME "dxrt"

#define INFERENCE_REQUEST_FIFO_SIZE 10
#define INFERENCE_RESPONSE_FIFO_SIZE 10

/**********************/
/* RT/driver sync     */

typedef enum
{
    ERR_NONE      = 0,
    ERR_NPU0_HANG = 1,
    ERR_NPU1_HANG,
    ERR_NPU2_HANG,
    ERR_PCIE_DMA_CH0_FAIL = 100,
    ERR_PCIE_DMA_CH1_FAIL,
    ERR_PCIE_DMA_CH2_FAIL,
} dxrt_error_t;

typedef struct device_info
{
    uint32_t type; /* 0: ACC type, 1: STD type */
    uint32_t variant; /* 100: L1, 101: L2, 102: L3, 103: L4,
                        200: M1, 201: M1A */
    uint64_t mem_addr;
    uint64_t mem_size;
    uint32_t num_dma_ch;
    uint16_t fw_ver;                // firmware version. A.B.C (e.g. 103)
    uint16_t bd_rev;                // board revision. /10 (e.g. 1 = 0.1, 2 = 0.2)
    uint16_t bd_type;               // board type. (1 = SOM, 2 = M.2, 3 = H1)
    uint16_t ddr_freq;              // ddr frequency. (e.g. 4200, 5500)
    uint16_t ddr_type;              // ddr type. (1 = lpddr4, 2= lpddr5)
    uint16_t interface;
} dxrt_device_info_t;

typedef struct _dxrt_meminfo_t
{
    uint64_t data;
    uint64_t base;
    uint32_t offset;
    uint32_t size;
} dxrt_meminfo_t;

typedef struct _dxrt_request_t {
    uint32_t  req_id;
    dxrt_meminfo_t input;
    dxrt_meminfo_t output;
    uint32_t  model_type;
    uint32_t  model_cmds;
    uint32_t  cmd_offset;
    uint32_t  weight_offset;
    uint32_t  last_output_offset;
} dxrt_request_t;

typedef struct _dxrt_request_acc_t {
    uint32_t  req_id;
    uint32_t  task_id;
    dxrt_meminfo_t input;
    dxrt_meminfo_t output;
    int16_t   npu_id;
    int16_t   model_type;
    uint32_t  model_cmds;
    uint32_t  cmd_offset;
    uint32_t  weight_offset;
    uint32_t  model_cmds2; // for m1 8k
    uint32_t  cmd_offset2; // for m1 8k
    uint32_t  weight_offset2; // for m1 8k
    int32_t   dma_ch;
    uint32_t  arg0; // additional parameter dependent to hw (for m1 8k)
    uint32_t  status;
} dxrt_request_acc_t;

typedef struct _dxrt_response_t {
    uint32_t  req_id;
    uint32_t  inf_time;
    uint16_t  argmax;
    uint16_t  model_type;
    int32_t   status;
    uint32_t  ppu_filter_num;
} dxrt_response_t;

typedef struct
{
    int32_t     cmd;
    int32_t     sub_cmd;
    void*       data;
    uint32_t    size;
} dxrt_message_t;
typedef struct
{
    uint32_t cmd;	    /* Command */
    uint32_t sub_cmd;   /* Sub command */
    uint32_t ack;       /* Response from device */
    uint32_t size;      /* Data Size */
    uint32_t data[1000];
} dxrt_device_message_t;
typedef enum {
    DXRT_CMD_IDENTIFY_DEVICE        = 0,
    DXRT_CMD_GET_STATUS             ,
    DXRT_CMD_RESET                  ,
    DXRT_CMD_UPDATE_CONFIG          ,
    DXRT_CMD_UPDATE_FIRMWARE        , /* Sub-command */
    DXRT_CMD_GET_LOG                ,
    DXRT_CMD_DUMP                   ,
    DXRT_CMD_WRITE_MEM              ,
    DXRT_CMD_READ_MEM               ,
    DXRT_CMD_CPU_CACHE_FLUSH        ,
    DXRT_CMD_SOC_CUSTOM             ,
    DXRT_CMD_WRITE_INPUT_DMA_CH0    ,
    DXRT_CMD_WRITE_INPUT_DMA_CH1    ,
    DXRT_CMD_WRITE_INPUT_DMA_CH2    ,
    DXRT_CMD_READ_OUTPUT_DMA_CH0    ,
    DXRT_CMD_READ_OUTPUT_DMA_CH1    ,
    DXRT_CMD_READ_OUTPUT_DMA_CH2    ,
    DXRT_CMD_TERMINATE              ,
    DXRT_CMD_ERROR                  ,
    DXRT_CMD_DRV_INFO               , /* Sub-command */
    DXRT_CMD_MAX,
} dxrt_cmd_t;

/* CMD : DXRT_CMD_DRV_INFO*/
typedef enum {
    DRVINFO_CMD_GET_RT_INFO     = 0,
    DRVINFO_CMD_GET_PCIE_INFO   = 1,
} dxrt_drvinfo_sub_cmd_t;

/* CMD : DXRT_CMD_UPDATE_FIRMWARE */
typedef enum {
    FWUPDATE_ONLY      = 0,
    FWUPDATE_DEV_RESET = BIT(1),
    FWUPDATE_FORCE     = BIT(2),
} dxrt_fwupdate_sub_cmd_t;

#define DXRT_IOCTL_MAGIC     'D'
typedef enum {
    DXRT_IOCTL_MESSAGE = _IOW(DXRT_IOCTL_MAGIC, 0, dxrt_message_t),
    DXRT_IOCTL_DUMMY = _IOW(DXRT_IOCTL_MAGIC, 1, dxrt_message_t),
    DXRT_IOCTL_MAX
} dxrt_ioctl_t;

/**********************/

typedef struct _dxrt_queue_t {
    union {
        uint8_t buffer[2048];
        // dxrt_request_t req[INFERENCE_REQUEST_FIFO_SIZE]; // not used in acc device
        dxrt_request_acc_t req_acc[INFERENCE_REQUEST_FIFO_SIZE];
        dxrt_response_t res[INFERENCE_RESPONSE_FIFO_SIZE];
    };
    uint32_t lock;
    uint32_t front;
    uint32_t rear;
    uint32_t count;
    uint32_t max_count;
    uint32_t elem_size;
    int32_t  enable;
} dxrt_queue_t;

typedef struct dxrt_request_list
{
    struct list_head list;
    dxrt_request_t request;
} dxrt_request_list_t;
typedef struct dxrt_response_list
{
    struct list_head list;
    dxrt_response_t response;
} dxrt_response_list_t;

struct dxdev {
    int id;
    struct cdev cdev;
    struct platform_device *pdev;
    struct device *dev;    
    uint32_t type; /* 0: ACC type, 1: STD type */
    uint32_t variant; /* 100: L1, 101: L2, 102: L3, 103: L4, 200: M1, 201: M1A */
    uint64_t mem_addr;
    uint64_t mem_size;
    uint32_t num_dma_ch;
    dxnpu_t *npu;
    // pcie : TODO
    dxrt_device_message_t *msg;
    struct mutex msg_lock;
    uint32_t *log;

    dxrt_queue_t *request_queue;
    spinlock_t request_queue_lock;

    struct task_struct *request_handler;
    wait_queue_head_t request_wq;
    dxrt_request_list_t requests;
    spinlock_t requests_lock;

    dxrt_response_list_t responses;
    spinlock_t responses_lock;

    dxrt_response_t response;
    wait_queue_head_t error_wq;
    dxrt_error_t error;
    spinlock_t error_lock;
};

struct dxrt_driver {
    dev_t dev_num;
    struct class *dev_class;
    int num_devices;
    struct dxdev *devices[8];
    struct platform_device *pdev;
};

typedef int (*dxrt_message_handler)(struct dxdev*, dxrt_message_t*);

int dxrt_driver_cdev_init(struct dxrt_driver *drv);
void dxrt_driver_cdev_deinit(struct dxrt_driver *drv);
int dxrt_request_handler(void *data);
void dxrt_init_queue(dxrt_queue_t* q, uint32_t max_count, uint32_t elem_size);
void dxrt_enable_queue(dxrt_queue_t *q);
void dxrt_disable_queue(dxrt_queue_t *q);
int dxrt_is_queue_empty(dxrt_queue_t* q);
int dxrt_is_queue_full(dxrt_queue_t* q);
void dxrt_lock_queue(dxrt_queue_t *q);
void dxrt_unlock_queue(dxrt_queue_t *q);
int dxrt_enqueue(dxrt_queue_t* q, void *elem);
int dxrt_dequeue(dxrt_queue_t* q, void *elem);
int dxrt_is_request_list_empty(dxrt_request_list_t *requests, spinlock_t *lock);
int message_handler_general(struct dxdev *dx, dxrt_message_t *msg);

extern dxrt_message_handler message_handler[];

#if IS_STANDALONE
#define dx_pcie_send_message(...) 0
#define dx_sgdma_init(...) 0
#define dx_sgdma_deinit(...) 0
#define dx_sgdma_write(...) 0
#define dx_sgdma_read(...) 0
#define dx_pcie_get_message_area(...) 0
#define dx_pcie_get_log_area(...) 0
#define dx_pcie_get_request_queue(...) 0
#define dx_pcie_clear_response_queue(...) 0
#define dx_pcie_is_response_queue_empty(...) 0
#define dx_pcie_interrupt(...) 0
#define dx_pcie_interrupt_wakeup(...) 0
#define dx_pcie_dequeue_response(...) 0
#define dx_pcie_get_dev_num(...) 1
#define dx_pcie_enqueue_error_response(...) 0
#define dx_pcie_dequeue_error_response(...) 0
typedef struct {
    uint32_t  req_id;
    uint32_t  inf_time;
    uint16_t  argmax;
    uint16_t  model_type;
	int32_t   status;
    int32_t   dma_ch;
    uint64_t  data;
    uint64_t  base;
    uint32_t  offset;
    uint32_t  size;
} dx_pcie_response_t;
typedef struct {
    uint32_t err_code;
    /* System Infomation power / temperature, etc,,,, */
} dx_pcie_dev_err_t;
#endif
#if IS_ACCELERATOR
#define sbi_l2cache_flush(...) { /*do nothing*/ }
#endif

#endif // __DXRT_DRV_H
