// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef _DW_EDMA_CORE_H
#define _DW_EDMA_CORE_H

#include <linux/msi.h>
#include <linux/pci.h>
#include <linux/interrupt.h>

#include "dx_pci_compat.h"	//DEEPX MODIFIED: 4.4 pci_* compat
#include "dx_mm_compat.h"	//DEEPX MODIFIED: 4.4 kvmalloc/kvfree compat
#include "dx_ida_compat.h"	//DEEPX MODIFIED: 4.4 ida_alloc_max/ida_free compat
#include "dx_edma.h"
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
#include "dx_message.h"
#endif

#include "virt-dma.h"		//DEEPX MODIFIED
#include "dx_cdev.h"

#define EDMA_LL_SZ						24
#define EDMA_MAX_WR_CH					8
#define EDMA_MAX_RD_CH					8

/* M1 wiring: READ CH0/1 are public H2C data channels; READ CH2/3 are
 * reserved for linked-list descriptor copy into device LL memory. */
#define DX_H2C_DATA_CH_CNT			2

#define USER_NUM_MAX					4
#define USER_IRQ_NUMS					16

enum dw_edma_dir {
	EDMA_DIR_WRITE = 0,
	EDMA_DIR_READ
};

enum dw_edma_engine {
	EDMA_ENG_DIS = 0,
	EDMA_ENG_EN
};

enum dx_edma_map_format {
	DX_DMA_MF_EDMA_LEGACY = 0x0,
	DX_DMA_MF_EDMA_UNROLL = 0x1,
	DX_DMA_MF_HDMA_COMPAT = 0x5
};

enum dw_edma_request {
	EDMA_REQ_NONE = 0,
	EDMA_REQ_STOP,
	EDMA_REQ_PAUSE
};

enum dw_edma_status {
	EDMA_ST_IDLE = 0,
	EDMA_ST_PAUSE,
	EDMA_ST_BUSY
};

enum dw_edma_xfer_type {
	EDMA_XFER_SCATTER_GATHER = 0,
	EDMA_XFER_CYCLIC,
	EDMA_XFER_INTERLEAVED
};

enum dw_iatu_inb_usage {
    IATU_INB_DMA_DESC  = 0,
    IATU_INB_USER0     = 1,
    IATU_INB_USER1     = 2,
    IATU_INB_USER2     = 3,
	IATU_INB_MAX       = 4
};

enum dw_edma_ch_id {
	EDMA_CH_ID_0 = 0,
	EDMA_CH_ID_1,
	EDMA_CH_ID_2,
	EDMA_CH_ID_3,
};

#define DX_READ_HELPER_CH_FIRST		EDMA_CH_ID_2
#define DX_READ_HELPER_CH_LAST		EDMA_CH_ID_3

struct dw_edma_chan;
struct dw_edma_chunk;

struct dx_edma_region {
	phys_addr_t			paddr;
	void				__iomem *vaddr;
	size_t				sz;
};

struct dx_iatu_inbound {
	u8 mode;
	u64 base_addr;
	u64 tgt_addr;
	u64 size;
	u32 idx;
	u8 bar_no;
};

struct dx_user_region {
	phys_addr_t			paddr;
	phys_addr_t			ep_addr;
	void				__iomem *vaddr;
	size_t				sz;
	u32					bar_num;
};

struct dw_edma_burst {
	struct list_head		list;
	u64				sar;
	u64				dar;
	u32				sz;
	bool			from_pool;
};

struct dw_edma_chunk {
	struct list_head		list;
	struct dw_edma_chan		*chan;
	struct dw_edma_burst	*burst;

	u32						bursts_alloc;
	u8						cb;
	struct dx_edma_region	ll_region;	/* Linked list */
	/*
	 * host_region: a struct copy of chan->desc_buf populated at
	 * dw_edma_alloc_chunk_for_chan().  HW DMA reads descriptors only
	 * from device LL SRAM after dw_edma_v0_core_precopy_lli() /
	 * xfer_llm_desc completes, so the buffer is reused across all
	 * chunks of all transfers on the same channel.  Free paths must
	 * NEVER free host_region — only kfree the chunk metadata.  The
	 * underlying buffer is owned by chan->desc_buf and freed at remove.
	 */
	struct dx_edma_region	host_region;
};

struct dw_edma_desc {
	struct virt_dma_desc	vd;
	struct dw_edma_chan		*chan;
	struct dw_edma_chunk	*chunk;
	/* Embedded list head only.  It does not own a 1MB descriptor staging
	 * buffer; real DMA descriptor bytes live in the per-channel desc_buf
	 * referenced by each active chunk's host_region. */
	struct dw_edma_chunk	chunk_head;

	u32						chunks_alloc;

	u32						alloc_sz;
	u32						xfer_sz;
	bool					from_pool;

	/*
	 * Deferred desc free for atomic context (tasklet/IRQ).
	 * When dw_edma_free_desc() is called from vchan_complete() tasklet,
	 * cancel_work_sync() is not safe:
	 * - cancel_work_sync() can sleep in atomic context
	 * This work defers the entire desc cleanup to process context.
	 */
	struct work_struct		deferred_free_work;

	/*
	 * Lazy burst mode: For SG transfers exceeding one chunk's capacity
	 * (ll_max bursts), only the first chunk-worth of bursts is populated
	 * at PREP time. The ISR reuses the same chunk for subsequent batches,
	 * eliminating runtime chunk allocation entirely.
	 *
	 * sg_cur:        next SG entry to process (NULL when done)
	 * sg_remaining:  number of SG entries left
	 * addr_accum:    accumulated src/dst address for linear addressing
	 * lazy_mode:     true if SG entries remain after first chunk fill
	 */
	struct scatterlist		*sg_cur;
	u32						sg_remaining;
	u64						addr_accum;
	bool					lazy_mode;
	/*
	 * Shadow pre-build: a workqueue builds the NEXT chunk's LLI while the
	 * current DMA runs. The Done ISR only marks the safe handoff point;
	 * helper-copy and launch stay in workqueue context.
	 */
	enum {
		SHADOW_IDLE = 0,	/* No shadow work in progress */
		SHADOW_BUILDING,	/* WQ is building shadow chunk */
		SHADOW_READY,		/* Host LLI ready; waiting for Done handoff */
		SHADOW_ISR_PENDING,	/* Done observed; WQ may copy/launch */
		SHADOW_CANCELLED,	/* Transfer terminated; WQ must discard */
	}							shadow_state;
	struct dw_edma_chunk	*shadow_chunk;	/* Pre-built chunk */
	struct dw_edma_chunk	*shadow_next_chunk; /* Pre-allocated for next cycle */
	struct scatterlist		*shadow_next_sg;
	u32						shadow_next_remaining;
	u64						shadow_next_addr;
	u32						shadow_alloc_sz;
	bool						shadow_has_more;
	bool						shadow_work_initialized;
	u8							shadow_expected_cb; /* CB value for next shadow LLI */
	struct work_struct		shadow_work;

	/* Fixed-buffer lazy continuation for H2C/non-shadow transfers. */
	enum {
		LAZY_REFILL_IDLE = 0,		/* No fixed-buffer refill pending */
		LAZY_REFILL_PENDING,		/* Done ISR queued lazy_work */
		LAZY_REFILL_PREPARING,		/* WQ owns detached chunk outside vc.lock */
	}						lazy_refill_state;
	bool						lazy_work_initialized;
	struct work_struct		lazy_work;
};

/*
 * Global Shared Memory Pool Configuration
 * - DESC:  Max concurrent transfers across all channels
 * - BURST: Total pre-allocated burst structures for active + shadow chunks
 *
 * Step 3: chunk pool removed — per-channel desc_buf (1MB each) replaces it.
 * See struct dw_edma_chan::desc_buf.
 */
#define EDMA_GLOBAL_DESC_POOL_SIZE		4
#define EDMA_CHUNK_SIZE					(1024 * 1024) /* 1MB */
#define EDMA_BURSTS_PER_CHUNK			(((EDMA_CHUNK_SIZE) + EDMA_LL_SZ - 1) / EDMA_LL_SZ)

/*
 * Worst-case concurrent burst metadata under shadow WQ:
 *  - 4 C2H channels can each hold active + prebuilt shadow chunks = 8 chunks
 *  - 2 public H2C channels can each hold one active chunk = 2 chunks
 * Keep two chunks of headroom to avoid process-context dynamic allocation in
 * the DMA hot path.
 */
#define EDMA_GLOBAL_BURST_CHUNKS		12
#define EDMA_GLOBAL_BURST_POOL_SIZE		(EDMA_BURSTS_PER_CHUNK * EDMA_GLOBAL_BURST_CHUNKS)

struct dw_edma_chan {
	struct virt_dma_chan	vc;
	struct dw_edma_chip		*chip;
	int				id;
	enum dw_edma_dir		dir;

	u32						ll_max;

	struct msi_msg			msi;

	enum dw_edma_request	request;
	enum dw_edma_status		status;
	u8						configured;
	u8 						en_lie;   /* Generate a local interrupt to start npu */
	bool					is_llm;   /* Enable Linked List Mode */
	bool					aborted;  /* Set by abort ISR, checked by sg_process */
	bool					hw_err;   /* Channel stuck in CS=2, HW irrecoverable */
	bool					xfer_started; /* Data doorbell was issued for current chunk */
	unsigned long			last_xfer_start_jiffies;

	/* Completion-path diagnostics: used to distinguish whether a timeout
	 * is caused by a genuinely missing MSI/status bit, Done ISR state
	 * handling, or delayed vchan callback delivery. */
	atomic64_t			done_status_seen_cnt;
	atomic64_t			done_isr_cnt;
	atomic64_t			done_replay_cnt;
	unsigned long		last_done_status_jiffies;
	unsigned long		last_done_isr_jiffies;
	unsigned long		last_done_replay_jiffies;

	wait_queue_head_t		*transfer_wq;  /* Thread waitqueue for in-flight wake */

	struct dw_edma_burst	**burst_batch; /* Pre-alloc scratch for batch alloc in ISR */
	struct dw_edma_burst	**shadow_burst_batch; /* Scratch for shadow WQ build */

	/*
	 * Per-channel fixed 1MB descriptor staging buffer.
	 * Reused across all chunks of all transfers on this channel.
	 * Allocated at probe, freed at remove.
	 *
	 * Helper RD channels (id >= DX_READ_HELPER_CH_FIRST when dir=READ)
	 * never own a desc_buf — they only do device-to-device LLI copies
	 * and never call dw_edma_alloc_chunk*().  desc_buf.vaddr stays NULL.
	 *
	 * Reuse safety: HW reads descriptors only from device LL SRAM after
	 * precopy_lli/xfer_llm_desc returns; the host buffer becomes free
	 * for the next chunk's LLI build.  Concurrency is serialized by
	 * vc.lock + desc->shadow_state machine — no extra lock needed.
	 */
	struct dx_edma_region	desc_buf;
	bool					desc_buf_is_buddy;

	struct dma_slave_config	config;

	/*
	 * Per-channel launch worker.  dw_edma_start_transfer() runs under
	 * vc.lock (issue_pending / done_interrupt continuation / resume), so
	 * it cannot directly call helper-channel acquire which may sleep
	 * under multi-channel contention.  Instead it stages the next chunk
	 * here and queues launch_work on shadow_wq, where helper acquire is
	 * free to sleep with proper fair scheduling.
	 *
	 * pending_launch_chunk is single-slot: a new chunk is only ever
	 * staged when the previous one has completed (Done ISR) or never
	 * started (terminate_all cancel), so there is no overlap.  Worker
	 * consumes it under vc.lock to avoid races with terminate_all.
	 */
	struct work_struct		launch_work;
	struct dw_edma_chunk	*pending_launch_chunk;
	bool					pending_launch_first;
};

struct dx_dma_user_irq {
	struct dw_edma		*dw;
	u8					user_idx;			/* 0 ~ 15 */
	u8					events_irq;			/* accumulated IRQs */
	spinlock_t			events_lock;		/* lock to safely update events_irq */
	wait_queue_head_t	events_wq;	/* wait queue to sync waiting threads */
	irq_handler_t		handler;
	char				name[40];

	void *dev;
};

struct dx_edma_irq {
	struct msi_msg			msi;
	u32						wr_mask;
	u32						rd_mask;
	struct dw_edma			*dw;
	void					*data[EDMA_MAX_WR_CH][2];	/* user datas [dma channel number][write/read] */

	struct dx_dma_user_irq	user_irq;	/* user IRQ management for multi interrupt */
	struct dx_dma_user_irq	user_irqs[USER_IRQ_NUMS];	/* user IRQ management for one interrupt */
};

struct dma_chan_lock {
	spinlock_t ch_lock;
	bool       ch_in_use;
};

static inline bool dx_dma_try_acquire_chan_ownership(struct dma_chan_lock *target_lock,
						      unsigned long *flags)
{
	if (!spin_trylock_irqsave(&target_lock->ch_lock, *flags))
		return false;

	if (target_lock->ch_in_use) {
		spin_unlock_irqrestore(&target_lock->ch_lock, *flags);
		return false;
	}

	target_lock->ch_in_use = true;
	spin_unlock_irqrestore(&target_lock->ch_lock, *flags);

	return true;
}

static inline void dx_dma_release_chan_ownership(struct dma_chan_lock *target_lock,
						 unsigned long *flags)
{
	spin_lock_irqsave(&target_lock->ch_lock, *flags);
	target_lock->ch_in_use = false;
	spin_unlock_irqrestore(&target_lock->ch_lock, *flags);
}

struct dx_edma_core_ops {
	int	(*irq_vector)(struct device *dev, unsigned int nr);
};
typedef struct user_irq_v_table_t {
	irq_handler_t handler;
	char          name[40];
	int           irq_pos;   /* Position in host IRQ table */
	int           event_id;  /* Max ID defined by EDMA_EVENT_NUM_MAX */
	int           dma_ch_n;  /* DMA channel number */
	uint32_t      bit;       /* Bit position mask */
} user_irq_v_table_t;

/*
 * Unified device lifecycle state machine (control plane).
 * Transitions: UNINIT → LIVE → RECOVERING → LIVE
 *                             → AER_RESET  → LIVE
 *              LIVE → REMOVING → (freed)
 */
enum dx_dev_state {
	DX_DEV_UNINIT     = 0, /* probe not yet complete */
	DX_DEV_LIVE       = 1, /* normal operation */
	DX_DEV_RECOVERING = 2, /* DMA recovery / SBR in progress */
	DX_DEV_AER_RESET  = 3, /* AER error_detected → slot_reset */
	DX_DEV_REMOVING   = 4, /* remove in progress */
};

/*
 * Physical link health state (data plane).
 * Separate from dev_state: a device can be DX_DEV_RECOVERING with
 * LINK_UP (DMA error) or LINK_DOWN (physical link loss).
 * The two axes must be checked together to decide recovery strategy.
 */
enum dx_link_state {
	DX_LINK_UNKNOWN    = 0, /* pre-probe or indeterminate */
	DX_LINK_UP         = 1, /* link active, DLLLA=1, healthy */
	DX_LINK_DOWN       = 2, /* link lost, config reads return 0xFFFF */
	DX_LINK_RECOVERING = 3, /* re-init in progress after link-up detected */
	DX_LINK_DEGRADED   = 4, /* link up but speed/width below expected */
};

/* Link health monitoring configuration defaults */
#define DX_HEALTH_INTERVAL_MS		200	/* health worker poll period */
#define DX_HEALTH_FAIL_THRESHOLD	3	/* consecutive fails before LINK_DOWN */
#define DX_LINK_VALIDATION_TIMEOUT_MS	300	/* post-reinit link check timeout */
#define DX_RECOVERY_WINDOW_SECS		60	/* rate-limit window */
#define DX_RECOVERY_MAX_IN_WINDOW	10	/* max recoveries per window */
#define DX_RECOVERY_COOLDOWN_SECS	10	/* silence after rate-limit hit */

struct dw_edma {
	struct list_head		list_head; /* Detect multi-device */

	char					name[20];
	char					irq_name[EDMA_MAX_WR_CH][20];
	struct pci_dev			*pdev;
	struct dx_dma_pci_dev	*xpdev;
	u8						idx;

	struct dma_device		wr_edma;
	u16						wr_ch_cnt;

	struct dma_device		rd_edma;
	u16						rd_ch_cnt;

	struct dx_edma_region	rg_region;	/* Registers */
	struct dx_edma_region	ll_region_wr[EDMA_MAX_WR_CH];
	struct dx_edma_region	ll_region_rd[EDMA_MAX_RD_CH];

	/* Global Shared Memory Pools */
	struct dw_edma_desc		*desc_pool;
	int						*desc_free_list;
	int						desc_free_cnt;

	struct dw_edma_burst	*burst_pool;
	int						*burst_free_list;
	int						burst_free_cnt;

	spinlock_t				pool_lock; /* Protects all global pools */
	spinlock_t				cleanup_lock; /* Protects deferred chunk cleanup list */
	struct list_head		cleanup_chunks;
	struct work_struct		cleanup_work;

	struct workqueue_struct		*shadow_wq; /* Dedicated WQ for parallel shadow builds */
	bool						shutting_down;

	u16							dx_ver;			/* DXNN Version */
	u16							user_bar_cnt;
	struct dx_user_region		npu_region[USER_NUM_MAX];
	u64							dma_desc_base_bar;
	u64							dma_desc_base_addr;
	u64							dma_desc_size;
	u32							dma_desc_bar_num;
	bool						pm_runtime_managed;

	struct dx_edma_region		iatu_region;
	struct dx_iatu_inbound		iatu_inb[IATU_INB_MAX];

	struct dx_edma_irq			*irq;
	int							dma_irqs; /* number of edma irqs */
	int							nr_irqs;
	u16							event_irq_idx;	/* Error index of MSI vector table */

	bool						vm_env;		/* true when running as a VFIO/VM guest */
	struct dx_hw_msi {
		u32  address_lo;
		u32  address_hi;
		u32  data;
		u32  nr_vectors;
		bool valid;
	} hw_msi;					/* MSI tuple reported by FW (live HW MSI cap) */
	
	enum dx_edma_map_format		mf;

	struct dw_edma_chan			*chan;
	int							rd_dma_id;	/* host - read  : DMA_WR */
	int							wr_dma_id;	/* host - write : DMA_RD */
	struct dma_chan 			*rd_dma_chan[EDMA_MAX_RD_CH];	/* DMA_READ */
	struct dma_chan_lock        rd_dma_chan_locks[EDMA_MAX_RD_CH];
	struct dma_chan 			*wr_dma_chan[EDMA_MAX_WR_CH];	/* DMA_WRITE */
	struct dma_chan_lock        wr_dma_chan_locks[EDMA_MAX_WR_CH];
	atomic_t					dev_state;      /* enum dx_dev_state */
	atomic_t					recovery_epoch; /* incremented on each DMA channel reset/recovery */
	bool						init_completed; /* lazy re-init trigger for RT module */
	int 						ref_count;	/* external module reference count */

	/* DXNN V2 only */
	u16							rd_dma_chan_list[EDMA_MAX_RD_CH]; /* 1 : used, 0: not-used */
	u16							wr_dma_chan_list[EDMA_MAX_WR_CH]; /* 1 : used, 0: not-used */
	struct mutex				rd_lock;
	struct mutex				wr_lock;
	struct dx_pcie_msg			*dx_msg;

	const struct dx_edma_core_ops	*ops;

	raw_spinlock_t				lock;		/* Only for legacy */
	spinlock_t					engine_reset_lock; /* Serialize engine_en cycles */

	/* Device Specific Datas */
	u64							download_region;
	u32							download_size;
	u64							booting_region[2];

	user_irq_v_table_t			*user_irq_vec_table;  /* selected at runtime by set_user_irq_vec_table() */
	u32							sw_active_mask;       /* bit i set when event i is active */
	int							sw_active_count;      /* number of active events */
	atomic_t					rr_toggle;            /* per-device round-robin for dedicated DMA channels */

	/* Link health monitoring (Layer 2: active detection) */
	atomic_t					link_state;           /* enum dx_link_state */
	atomic_t					link_health_enabled;  /* user-controlled sysfs on/off */
	atomic_t					background_recovery_paused; /* 1 while AER/sysfs reset owns worker lifecycle */
	struct delayed_work			health_work;          /* periodic link health poll */
	struct work_struct			recovery_work;        /* deferred recovery (enqueued, not inline) */
	int							health_fail_count;    /* consecutive health check failures */
	unsigned long				last_recovery_jiffies; /* rate-limit: last auto-recovery time */
	int							recovery_count_in_window; /* rate-limit: recovery attempts in current window */
	unsigned long				recovery_window_start;    /* rate-limit: window start jiffies */
	unsigned long				rate_limit_cooldown_until; /* jiffies; health worker skips scheduling until then */

#ifdef CONFIG_DEBUG_FS
	struct dentry			*debugfs;
#endif /* CONFIG_DEBUG_FS */
};

struct dw_edma_sg {
	struct scatterlist		*sgl;
	unsigned int			len;
};

struct dw_edma_cyclic {
	dma_addr_t			paddr;
	size_t				len;
	size_t				cnt;
};

struct dw_edma_transfer {
	struct dma_chan			*dchan;
	union dw_edma_xfer {
		struct dw_edma_sg		sg;
		struct dw_edma_cyclic		cyclic;
		struct dma_interleaved_template *il;
	} xfer;
	enum dma_transfer_direction	direction;
	unsigned long			flags;
	enum dw_edma_xfer_type		type;
};

static inline
struct dw_edma_chan *vc2dw_edma_chan(struct virt_dma_chan *vc)
{
	return container_of(vc, struct dw_edma_chan, vc);
}

static inline
struct dw_edma_chan *dchan2dw_edma_chan(struct dma_chan *dchan)
{
	return vc2dw_edma_chan(to_virt_chan(dchan));
}

static inline
struct dw_edma_desc *vd2dw_edma_desc(struct virt_dma_desc *vd)
{
	return container_of(vd, struct dw_edma_desc, vd);
}

void dw_edma_free_single_burst(struct dw_edma_chan *chan, struct dw_edma_burst *burst);
int dw_edma_process_done_if_stopped(struct dw_edma_chan *chan);
bool dw_edma_shadow_transfer_active(struct dw_edma_chan *chan);
int dx_dma_probe(struct dw_edma_chip *chip);
int dx_dma_remove(struct dw_edma_chip *chip);

void set_user_irq_vec_table(struct dw_edma *dw);
int get_nr_user_irqs(struct dw_edma *dw);
int get_pos_user_irqs(struct dw_edma *dw, int event_id);
bool check_event_id(struct dw_edma *dw, int event_id);

#endif /* _DW_EDMA_CORE_H */
