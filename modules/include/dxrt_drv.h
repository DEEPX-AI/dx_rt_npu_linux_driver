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
#include <linux/sched/signal.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/atomic.h>

#include "dxrt_drv_common.h"
#include "dxrt_drv_npu.h"

//#if IS_ACCELERATOR
#include "dx_pcie_api.h"
//#endif
#if IS_STANDALONE
/* L2 cache flush api */
//#include <asm/sbi.h>
#include <asm/cacheflush.h>
#endif

#define MODULE_NAME "dxrt"

#define DXRT_MSG_DATA_ADDR(dev) \
    DX_FIELD_ADDR((dev)->msg, dxrt_device_message_t, data)

/* Default PCIe channel count used across the driver */
#ifndef MAX_PCIE_CH_NUM
#define MAX_PCIE_CH_NUM     (3)
#endif

#define INFERENCE_REQUEST_FIFO_SIZE      (10)
#define INFERENCE_RESPONSE_FIFO_SIZE     (10)
#define DX_QUEUE_THRESHOLD_FOR_INTERRUPT (1)

#define DX_ACC              (0) /* accelator device */
#define DX_STD              (1) /* stand alone device */
#define DX_DEVICE_MAX_NUM   (64)

/**********************/
/* RT/driver sync     */

typedef enum {
    DXRT_EVENT_NONE,
    DXRT_EVENT_ERROR,
    DXRT_EVENT_NOTIFY_THROT,
    DXRT_EVENT_RECOVERY,
    DXRT_EVENT_NUM,
} dxrt_event_t;

/*
 * Device-level operational state — tracked per dxdev, exposed via
 * /sys/class/dxrt/dxrtN/recovery/link_state.  Separate from the PCIe
 * transport's dx_link_state (held by dw_edma) because the transport
 * may be healthy while FW is hung, and vice versa.
 *
 * See: docs/recovery/RECOVERY_IMPLEMENTATION_PLAN.md §1
 */
typedef enum {
    DXRT_STATE_READY        = 0, /* normal operation                          */
    DXRT_STATE_RECOVERING   = 1, /* transport-level recovery in flight        */
    DXRT_STATE_TRANSPORT_OK = 2, /* link restored, readiness probe pending   */
    DXRT_STATE_FW_HANG      = 3, /* readiness probe timed out — needs reset  */
    DXRT_STATE_WAITING_USER = 4, /* FW cmd timeout — user decision required   */
    DXRT_STATE_PERM_FAIL    = 5, /* retry threshold exceeded                  */
} dxrt_dev_state_t;

typedef enum _dxrt_error_t {
    ERR_NONE      = 0,
    ERR_NPU0_HANG = 1,
    ERR_NPU1_HANG,
    ERR_NPU2_HANG,
    ERR_NPU_BUS,
    ERR_PCIE_DMA_CH0_FAIL = 100,
    ERR_PCIE_DMA_CH1_FAIL,
    ERR_PCIE_DMA_CH2_FAIL,
    ERR_LPDDR_DED_WR      = 200,
    ERR_LPDDR_DED_RD,
    ERR_FW_TIMEOUT        = 300,
    ERR_PCIE_DMA_CH0_ABORT = 400,
    ERR_PCIE_DMA_CH1_ABORT,
    ERR_PCIE_DMA_CH2_ABORT,
    /*
     * Historical aliases for older link-down reporting.  Link-health
     * recovery now reports link flaps via DX_EVENT_TYPE_RECOVERY with
     * reason=DX_RECOVERY_REASON_LINK_FLAP instead of emitting these
     * DMA-abort-shaped ERROR events.
     */
    ERR_PCIE_LINK_DOWN_CH0 = ERR_PCIE_DMA_CH0_ABORT,
    ERR_PCIE_LINK_DOWN_CH1 = ERR_PCIE_DMA_CH1_ABORT,
    ERR_PCIE_LINK_DOWN_CH2 = ERR_PCIE_DMA_CH2_ABORT,
    ERR_DEVICE_ERR        = 1000,
} dxrt_error_t;

typedef enum _dxrt_notify_throt_t {
    NTFY_NONE       = 0,
    NTFY_THROT_FREQ_DOWN,
    NTFY_THROT_FREQ_UP,
    NTFY_THROT_VOLT_DOWN,
    NTFY_THROT_VOLT_UP,
    NTFY_EMERGENCY_BLOCK,
    NTFY_EMERGENCY_RELEASE,
} dxrt_notify_throt_t;

typedef enum _npu_priority_op {
    N_PRIORITY_NORMAL = 0,
    N_PRIORITY_HIGH,
} npu_priority_op;

typedef enum _npu_bandwidth_op {
    N_BANDWIDTH_NORMAL = 0,
    N_BANDWIDTH_NPU0,
    N_BANDWIDTH_NPU1,
    N_BANDWIDTH_NPU2,
    N_BANDWIDTH_PCIE,
    N_BANDWIDTH_MAX,
} npu_bandwidth_op;

typedef enum _npu_bound_op {
    N_BOUND_NORMAL = 0,     /*inference with 3-npu */
    N_BOUND_INF_ONLY_NPU0,
    N_BOUND_INF_ONLY_NPU1,
    N_BOUND_INF_ONLY_NPU2,
    N_BOUND_INF_2_NPU_01,   /* Infrence with 2-npu */
    N_BOUND_INF_2_NPU_12,   /* Infrence with 2-npu */
    N_BOUND_INF_2_NPU_02,   /* Infrence with 2-npu */
    N_BOUND_INF_MAX,
} npu_bound_op;

typedef struct _dx_shced_data {
    npu_bound_op        bound;
    req_queue_t         queue;
    uint32_t            ref_cnt;
} dx_shced_data;

typedef struct _dx_sched_list {
    struct list_head    list;
    dx_shced_data       data;
} dx_sched_list;

typedef struct device_info
{
    uint32_t type; /* 0: ACC type, 1: STD type */
    uint32_t variant; /* 104: V3, 200: M1 */
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

typedef struct _dxrt_req_meminfo_t
{
    uint64_t data;
    uint64_t base;
    uint32_t offset;
    uint32_t size;
    uint32_t ch;
} dxrt_req_meminfo_t;

typedef struct _dxrt_request_t {
    uint32_t  req_id;
    dxrt_meminfo_t input;
    dxrt_meminfo_t output;
    uint32_t  model_type;
    uint32_t  model_format;
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
    int8_t    model_type;
    int8_t    model_format;
    uint32_t  model_cmds;
    uint32_t  cmd_offset;
    uint32_t  weight_offset;
    uint32_t  model_cmds2;    /* for m1 8k */
    uint32_t  cmd_offset2;    /* for m1 8k */
    uint32_t  weight_offset2; /* for m1 8k */
    int32_t   dma_ch;
    uint32_t  arg0;   /* additional parameter dependent to hw (for m1 8k) */
    uint32_t  status;
    uint32_t  proc_id;
    uint32_t  prior;        /* scheduler option - priority(npu_priority_op) */
    uint32_t  prior_level;  /* scheduler option - priority level */
    uint32_t  bandwidth;    /* scheduler option - bandwith(npu_bandwidth_op) */
    uint32_t  bound;        /* scheduler option - bound   (npu_bound_op) */
    uint32_t  queue;
} dxrt_request_acc_t;

typedef struct _dxrt_response_t {
    uint32_t  req_id;
    uint32_t  inf_time;
    uint16_t  argmax;
    uint16_t  model_type;
    int32_t   status;
    uint32_t  ppu_filter_num;
    uint32_t  proc_id;
    uint32_t  queue;
    int32_t   dma_ch;
    uint32_t  ddr_wr_bw;
    uint32_t  ddr_rd_bw;
} dxrt_response_t;

/* TODO: 
    args for 'to' in copy_to_user & copy_from_user function type is different
    Therefore, we need to define two structure for dxrt_message_t
    This will fix 'redundant type casting' problem.
*/
typedef struct
{
    int32_t     cmd;
    int32_t     sub_cmd;
    void*       data;
    uint32_t    size;
} dxrt_message_t;
typedef struct
{
    uint32_t cmd;       /* Command */
    uint32_t sub_cmd;   /* Sub command */
    uint32_t ack;       /* Response from device */
    uint32_t size;      /* Data Size */
    uint32_t data[1000];
} dxrt_device_message_t;
typedef enum {
    DXRT_CMD_IDENTIFY_DEVICE    = 0, /* Sub-command */
    DXRT_CMD_GET_STATUS         ,
    DXRT_CMD_RESET              ,
    DXRT_CMD_UPDATE_CONFIG      ,
    DXRT_CMD_UPDATE_FIRMWARE    , /* Sub-command */
    DXRT_CMD_GET_LOG            ,
    DXRT_CMD_DUMP               ,
    DXRT_CMD_WRITE_MEM          ,
    DXRT_CMD_READ_MEM           ,
    DXRT_CMD_CPU_CACHE_FLUSH    ,
    DXRT_CMD_SOC_CUSTOM         ,
    DXRT_CMD_WRITE_INPUT_DMA_CH0,
    DXRT_CMD_WRITE_INPUT_DMA_CH1,
    DXRT_CMD_WRITE_INPUT_DMA_CH2,
    DXRT_CMD_READ_OUTPUT_DMA_CH0,
    DXRT_CMD_READ_OUTPUT_DMA_CH1,
    DXRT_CMD_READ_OUTPUT_DMA_CH2,
    DXRT_CMD_TERMINATE_EVENT    ,
    DXRT_CMD_EVENT              ,
    DXRT_CMD_DRV_INFO           , /* Sub-command */
    DXRT_CMD_SCHEDULE           , /* Sub-command */
    DXRT_CMD_UPLOAD_FIRMWARE    ,
    DXRT_CMD_NPU_RUN_REQ        ,
    DXRT_CMD_NPU_RUN_RESP       ,
    DXRT_CMD_UPDATE_CONFIG_JSON ,
    DXRT_CMD_RECOVERY           ,
    DXRT_CMD_CUSTOM             , /* Sub-command */
    DXRT_CMD_START              ,
    DXRT_CMD_TERMINATE          ,
    DXRT_CMD_PCIE               , /* Sub-command */
    DXRT_CMD_NPU_RUN_RESP_V2    ,
    DXRT_CMD_EVENT_V2           ,
    DXRT_CMD_MAX                ,
} dxrt_cmd_t;

/* CMD : DXRT_CMD_IDENTIFY_DEVICE*/
typedef enum {
    DX_IDENTIFY_NONE        = 0,
    DX_IDENTIFY_FWUPLOAD    = 1,
} dxrt_ident_sub_cmd_t;

/* CMD : DXRT_CMD_SCHEDULE */
typedef enum {
    DX_SCHED_ADD    = 1,
    DX_SCHED_DELETE = 2
} dxrt_sche_sub_cmd_t;

/* CMD : DXRT_CMD_DRV_INFO*/
typedef enum {
    DRVINFO_CMD_GET_RT_INFO     = 0,
    DRVINFO_CMD_GET_PCIE_INFO   = 1,
    DRVINFO_CMD_GET_RT_INFO_V2   = 2,
} dxrt_drvinfo_sub_cmd_t;

/* CMD : DXRT_CMD_UPDATE_FIRMWARE */
typedef enum {
    FWUPDATE_ONLY        = 0,
    FWUPDATE_DEV_UNRESET = BIT(1),
    FWUPDATE_FORCE       = BIT(2),
} dxrt_fwupdate_sub_cmd_t;

/*
 * FW image header (dx_fw_header_t) field locations needed by the driver to
 * auto-detect the target firmware version before the DMA download.  We do not
 * include the firmware header definition here; only the byte offset and length
 * of the ASCII version string ("A.B.C") are required.
 *
 * dx_fw_header_t layout (see dx_fw .../plat/include/dx_header.h):
 *   signature[16]                            @ 0x00
 *   dx_fw_image_info_t images[8] (8 * 24)    @ 0x10
 *   length, board_type, ddr_type (3 * 4)     @ 0xd0
 *   char fw_ver[16]                          @ 0xdc
 */
#define DX_FW_HDR_VER_OFFSET   (0xdcU) /* byte offset of fw_ver[] in image */
#define DX_FW_HDR_VER_LEN      (16U)   /* length of fw_ver[] in image      */

/*
 * Minimum firmware version that publishes the DLMSG boot-aware readiness
 * block.  Encoded as A*100 + B*10 + C (matches dxdev.fw_ver), e.g. 2.7.0 = 270.
 * Older images never set MAILBOX_READY, so post-update recovery must skip the
 * mailbox-ready edge gate for them.
 */
#define DX_FW_DLMSG_MIN_VER    (270U)  /* FW 2.7.0 */

#define DXRT_IOCTL_MAGIC     'D'
typedef enum {
    DXRT_IOCTL_MESSAGE = _IOW(DXRT_IOCTL_MAGIC, 0, dxrt_message_t),
    DXRT_IOCTL_DUMMY = _IOW(DXRT_IOCTL_MAGIC, 1, dxrt_message_t),
    DXRT_IOCTL_MAX
} dxrt_ioctl_t;

/**********************/
#define DX_DLMSG_MASIC_S   (8)
#define DX_DLMSG_SIZE      (0x40)
#define DX_DLMSG_BAR_MASIC (0xDEE1DEE1)
typedef enum {
    DW_NONE         = 0,
    DW_READY        = 1,
    DW_DONE         = 2, /* download done */
    DW_MAGIC_ERR    = 3,
    DW_HEADER_ERR   = 4,
    DW_CRC_ERR      = 5,
    DW_CERT_ERR     = 6,
    DW_EDMA_ERR     = 7, /* PCIe DMA */
    DW_CONN_ERR     = 8, /* Connection error */
    DW_TIMEOUT_ERR  = 9, /* Timeout error */
} dw_status;

typedef enum {
    DX_NONE        = 0,
    DX_ROM         = 1,
    DX_2ND_BOOT    = 2,
    DX_RTOS        = 3,
} boot_step_t;

typedef enum {
    DW_DEV_NONE = 0,
    DW_DEV_PCIE = 1,
    DW_DEV_UART = 2,
    DW_DEV_USB  = 3,
    DW_DEV_MAX  = 4,
} dw_dev_t;

/*
 * Boot-Aware Readiness extension (DLMSG bytes 0x20-0x3F).
 *
 * The lower 0x20 bytes are owned by ROM / 2nd-boot.  The upper 0x20
 * bytes are reserved for RTOS-published readiness signals so the
 * host can passively gate any mailbox traffic until FreeRTOS has
 * actually brought the message path up.  All fields are written by
 * the EP (RTOS) and read-only on the host.
 *
 * Writer ordering (RTOS side):
 *   1. Zero ready_flags on early RTOS boot / PERST.
 *   2. Set ready_magic = DX_DLMSG_READY_MAGIC and ready_version.
 *   3. After message_init() / event group / IRQ enable, set the
 *      individual flags then OR in MAILBOX_READY *last*.
 *
 * Reader (host) MUST validate ready_magic + ready_version before
 * trusting any other field.
 *
 * The driver detects FW reboot purely via a TRUE -> FALSE -> TRUE
 * edge on the MAILBOX_READY bit, so no persistent counter is required
 * to survive cold-boot memory wipe.
 */
#define DX_DLMSG_READY_MAGIC    (0x52454144u) /* "READ" */
#define DX_DLMSG_READY_VERSION  (1u)

/* ready_flags bits */
#define DX_DLMSG_READY_RTOS_BOOTED        BIT(0)
#define DX_DLMSG_READY_PCIE_IRQ_READY     BIT(1)
#define DX_DLMSG_READY_MESSAGE_TASK_READY BIT(2)
#define DX_DLMSG_READY_MAILBOX_READY      BIT(3)

/*
 * Host gate: only MAILBOX_READY is the contract bit (set LAST by FW after
 * all init steps). Other ready bits remain visible via dx_dlmsg_ready_flags()
 * for diagnostics, but must NOT gate mailbox sends.
 */
#define DX_DLMSG_READY_REQUIRED   DX_DLMSG_READY_MAILBOX_READY

typedef struct {
    uint8_t      magic[DX_DLMSG_MASIC_S]; /* 0x00 */
    uint32_t     mode;                    /* 0x08 */
    uint8_t      sts;                     /* 0x0C */ /*dw_status*/
    uint8_t      prev_sts;                /* 0x0D */ /*dw_status*/
    uint8_t      dev_type;                /* 0x0E */ /*dw_dev_t*/
    uint8_t      bt_step;                 /* 0x0F */ /*boot_step_t*/
    uint32_t     dl_size;                 /* 0x10 */
    uint32_t     dl_addr_s;               /* 0x14 */
    uint32_t     dl_addr_e;               /* 0x18 */
    uint32_t     bar_magic;               /* 0x1C */
    /* ---- RTOS-published readiness (0x20..0x3F) ---- */
    uint32_t     ready_magic;             /* 0x20 = DX_DLMSG_READY_MAGIC */
    uint16_t     ready_version;           /* 0x24 */
    uint16_t     ready_resv0;             /* 0x26 */
    uint32_t     ready_flags;             /* 0x28 */
    uint32_t     heartbeat;               /* 0x2C : optional, RTOS-maintained */
    uint32_t     ready_resv1;             /* 0x30 */
    uint32_t     ready_resv2;             /* 0x34 */
    uint32_t     ready_resv3;             /* 0x38 */
    uint32_t     ready_resv4;             /* 0x3C */
} __attribute__ ((packed,aligned(4))) dx_download_msg;

/**********************/
typedef struct _dxrt_queue_t {
    union {
        uint8_t buffer[2048];
        dxrt_request_acc_t req_acc[INFERENCE_REQUEST_FIFO_SIZE];
    };
    uint32_t lock;
    uint32_t front;
    uint32_t rear;
    uint32_t count;
    uint32_t max_count;
    uint32_t elem_size;
    int32_t  enable;
    uint32_t irq_done;      /* interrupt handshake */
    uint32_t id;
    uint32_t flag;          /* interrupt flags */
    uint32_t acces_count;   /* access counter from host and device */
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
    uint32_t variant; /* 104: V3, 200: M1 */
    uint64_t mem_addr;
    uint64_t mem_size;
    uint32_t num_dma_ch;
    dxnpu_t *npu;
    // pcie : TODO
    dxrt_device_message_t __iomem *msg;
    struct mutex msg_lock;
    uint32_t __iomem *log;
    dx_download_msg __iomem *dl;

    struct list_head sched;
    spinlock_t       sched_lock;

    dxrt_queue_t __iomem *request_queue;        /* normal priority / queue0 */
    dxrt_queue_t __iomem *request_queue1;       /* normal priority / queue1 */
    dxrt_queue_t __iomem *request_queue2;       /* normal priority / queue2 */
    dxrt_queue_t __iomem *request_high_queue;   /* high priority */
    struct mutex request_queue_mutex;
    struct mutex request_queue1_mutex;
    struct mutex request_queue2_mutex;
    struct mutex request_high_queue_mutex;

    struct task_struct *request_handler;
    wait_queue_head_t request_wq;
    dxrt_request_list_t requests;
    spinlock_t requests_lock;

    dxrt_response_list_t responses;
    spinlock_t responses_lock;

    /* Waiters for response availability per PCIe channel */
    wait_queue_head_t response_wq[MAX_PCIE_CH_NUM + 1];

    /* Pending response count per channel (avoids dx_dev_list_get in wait) */
    atomic_t response_pending[MAX_PCIE_CH_NUM + 1];

    /* Waiter for event availability */
    wait_queue_head_t event_wq;

    dxrt_response_t response;
    wait_queue_head_t error_wq;
    dxrt_error_t error;
    dxrt_notify_throt_t notify;
    atomic_t recovering;            /* Recovery in progress flag */
    atomic_t recovery_epoch;        /* Monotonic counter: increments each recovery */
    struct mutex recovery_mutex;    /* Serializes concurrent recovery requests */
    spinlock_t error_lock;

    /* ---- Recovery state machine (see RECOVERY_OVERVIEW.md) ---- */
    atomic_t dev_state;                 /* dxrt_dev_state_t */
    atomic_t recovery_count;            /* cumulative successful full_reinits */
    atomic_t recovery_fail_count;       /* cumulative failed recovery attempts */
    atomic_t last_recovery_reason;      /* enum dx_recovery_reason */
    u64  last_recovery_jiffies;         /* u64 on all archs; 64-bit atomicity not required (monotonic, advisory) */
    /* Recovery readiness worker — runs after LINK_EV_UP / CPU_RESET to
     * probe FW mailbox liveness before returning to READY. */
    struct delayed_work recovery_ready_work;
    /* PERM_FAIL sliding-window bookkeeping */
    unsigned long perm_fail_window_start;
    u32          perm_fail_count_in_window;
};

struct dxrt_driver {
    dev_t dev_num;
    struct class *dev_class;
    int num_devices;
    struct dxdev *devices[DX_DEVICE_MAX_NUM];
    struct platform_device *pdev;
};

struct dxrt_file_ctx {
    struct dxdev *dx;
    atomic_t terminating;
};

typedef int (*dxrt_message_handler)(struct dxdev*, dxrt_message_t*, struct dxrt_file_ctx*);

int dxrt_driver_cdev_init(struct dxrt_driver *drv);
void dxrt_driver_cdev_deinit(struct dxrt_driver *drv);
int dxrt_is_request_list_empty(dxrt_request_list_t *requests, spinlock_t *lock);
int message_handler_general(struct dxdev *dx, dxrt_message_t *msg, struct dxrt_file_ctx *ctx);
void dxrt_device_init(struct dxdev* dev);
void dxrt_device_init_early(struct dxdev *dev);
void dxrt_bind_pcie_resources(struct dxdev *dev, bool clear_queues);
void dxrt_clear_all_pending(struct dxdev *dev);

extern bool dxrt_fault_inject_skip_addr_check;

/* ---- Recovery state / sysfs ---- */
#if IS_ACCELERATOR
int  dxrt_sysfs_attach(struct dxdev *dev);
void dxrt_sysfs_detach(struct dxdev *dev);
void dxrt_recovery_state_init(struct dxdev *dev);
void dxrt_recovery_state_deinit(struct dxdev *dev);
/* Called from dxrt_drv_message.c on link events to drive state/events. */
void dxrt_link_event_notify(u32 dev_id, dx_pcie_link_event_t ev, void *data);
void dxrt_kick_cpu_reset_recovery(struct dxdev *dev);
void dxrt_kick_fw_update_recovery(struct dxdev *dev);
void dxrt_kick_fw_update_recovery_nodlmsg(struct dxdev *dev);
void dxrt_recovery_ready_work_fn(struct work_struct *work);
/* Module params */
extern unsigned int dxrt_perm_fail_window_ms;
extern unsigned int dxrt_perm_fail_max_retries;
extern bool         dxrt_auto_reset_on_fw_timeout;
extern unsigned int dxrt_dlmsg_ready_timeout_ms;
extern unsigned int dxrt_ping_timeout_ms;
#else
static inline int  dxrt_sysfs_attach(struct dxdev *dev)  { (void)dev; return 0; }
static inline void dxrt_sysfs_detach(struct dxdev *dev)  { (void)dev; }
static inline void dxrt_recovery_state_init(struct dxdev *dev)   { (void)dev; }
static inline void dxrt_recovery_state_deinit(struct dxdev *dev) { (void)dev; }
#endif
/* Queue */
void dxrt_init_queue(dxrt_queue_t __iomem *q, uint32_t max_count, uint32_t elem_size);
void dxrt_enable_queue(dxrt_queue_t __iomem *q);
void dxrt_disable_queue(dxrt_queue_t __iomem *q);
void dxrt_enqueue_irq_notify(dxrt_queue_t __iomem *q);
int dxrt_enqueue_irq_done(dxrt_queue_t __iomem *q);
int dxrt_is_queue_empty(dxrt_queue_t __iomem *q);
int dxrt_is_queue_full(dxrt_queue_t __iomem *q);
int dxrt_lock_queue(dxrt_queue_t __iomem *q);
void dxrt_unlock_queue(dxrt_queue_t __iomem *q);
int dxrt_enqueue(dxrt_queue_t __iomem *q, void *elem);
int dxrt_lock_check(dxrt_queue_t __iomem *q);
/* Scheduler */
int add_queue_from_sched_op(struct dxdev* dev, npu_bound_op bound);
int get_queue_from_sched_op(struct dxdev* dev, npu_bound_op bound, uint32_t *q);
int delete_matching_queue(struct dxdev* dev, npu_bound_op bound);
void clear_queue_list(struct dxdev* dev);
/* Update */
bool dx_get_flash_ready(dx_download_msg __iomem *msg, int timeout);
bool dx_get_flash_done(dx_download_msg __iomem *msg);
int8_t dx_get_boot_step(dx_download_msg __iomem *msg);
int8_t dx_get_dl_status(dx_download_msg __iomem *msg);

/* Boot-Aware readiness helpers (see DLMSG ready_flags above) */
bool     dx_dlmsg_ready_valid(dx_download_msg __iomem *msg);
uint32_t dx_dlmsg_ready_flags(dx_download_msg __iomem *msg);
bool     dx_dlmsg_mailbox_ready(dx_download_msg __iomem *msg);
bool     dx_dlmsg_wait_mailbox_ready(dx_download_msg __iomem *msg,
                                     unsigned int timeout_ms);

/* ---- DXRT_CMD_PCIE subcommands (must match firmware dx_message.h) ---- */
/* Existing: 0..3 owned by GET_PCIE_INFO/CLEAR_ERR_STAT/LINK_FLAP/AER_INJECT */
#define DX_PCIE_PING            (4u)
#define DX_PCIE_CPU_RESET       (5u) /* test-only: FW-side cpu_reset_with_reason */

#define DX_PCIE_PING_MAGIC      (0x50494E47u) /* "PING" */
#define DX_PCIE_PONG_MAGIC      (0x504F4E47u) /* "PONG" */

/* PING request payload (host -> EP, written into dev->msg->data[]) */
typedef struct {
    uint32_t magic;     /* DX_PCIE_PING_MAGIC */
    uint32_t seq;       /* echoed back by EP */
    uint32_t flags;     /* reserved, must be 0 */
    uint32_t resv;
} __attribute__((packed)) dx_pcie_ping_req_t;

/* PING response payload (EP -> host) */
typedef struct {
    uint32_t magic;        /* DX_PCIE_PONG_MAGIC */
    uint32_t seq;          /* echoed from request */
    uint32_t ready_flags;  /* same encoding as DLMSG ready_flags */
    uint32_t resv;
    uint32_t fw_ver;
    uint32_t uptime_ms;
    uint32_t resv0;
    uint32_t resv1;
} __attribute__((packed)) dx_pcie_pong_resp_t;

extern dxrt_message_handler message_handler[];

#if IS_STANDALONE
#define dx_pcie_send_message(...) 0
#define dx_sgdma_init(...) 0
#define dx_sgdma_deinit(...) 0
#define dx_pcie_reset_dma_channels(...) 0
#define dx_sgdma_write(...) 0
#define dx_sgdma_read(...) 0
#define dx_pcie_get_message_area(...) 0
#define dx_pcie_get_log_area(...) 0
#define dx_pcie_get_dl_area(...) 0
#define dx_pcie_get_request_queue(...) 0
#define dx_pcie_clear_response_queue(...) 0
#define dx_pcie_is_response_queue_empty(...) 0
#define dx_pcie_interrupt(...) 0
#define dx_pcie_interrupt_clear(...) 0
#define dx_pcie_interrupt_wakeup(...) 0
#define dx_pcie_interrupt_event_wakeup(...) 0
#define dx_pcie_dequeue_response(...) 0
#define dx_pcie_get_dev_num(...) 1
#define dx_pcie_get_download_region(...) 0
#define dx_pcie_get_download_size(...) 0
#define dx_pcie_get_booting_region(...) 0
#define dx_pcie_get_init_completed(...) 0
#define dx_pcie_set_init_completed(...) 0
#define dx_pcie_enqueue_event_response(...) 0
#define dx_pcie_dequeue_event_response(...) 0
#define dx_pcie_clear_event_response(...) 0
#define dx_pcie_enqueue_recovery_event(...) do {} while(0)
#define dx_pcie_enqueue_abort_event(...) do {} while(0)
#define dx_pcie_notify_msg_to_device(...) 0
#define dx_pcie_notify_req_to_device(...) 0
#define dx_pcie_get_driver_info(...) 0
#define dx_pcie_register_response_callback(...) do {} while(0)
#define dx_pcie_unregister_response_callback(...) do {} while(0)
#define dx_pcie_register_event_callback(...) do {} while(0)
#define dx_pcie_unregister_event_callback(...) do {} while(0)


int dxrt_request_handler(void *data);
#endif


#if IS_ACCELERATOR
#define sbi_l2cache_flush(...) { /*do nothing*/ }
#endif

#endif // __DXRT_DRV_H
