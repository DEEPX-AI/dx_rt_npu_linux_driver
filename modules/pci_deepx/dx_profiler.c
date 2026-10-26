// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver — DMA performance profiler
 *
 * Runtime-gated (perf_enable sysfs) per-stage latency, multi-channel
 * concurrency correlation, and per-stage outlier capture. Split out of
 * dx_util.c; declarations live in dx_util.h.
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/math64.h>
#include <linux/smp.h>
#include <linux/limits.h>
#include <linux/string.h>
#include <linux/spinlock.h>

#include "dx_util.h"

#if defined(DMA_PERF_MEASURE)
#define uint64_t long long unsigned int
dx_pcie_profiler_t g_pcie_prof[16][4][2][PCIE_PERF_MAX_T];
bool g_perf_enabled;
EXPORT_SYMBOL_GPL(g_perf_enabled);

#define DX_PCIE_PROF_DEV_MAX 16
#define DX_STATS_PRT_MAX_BUFFER_SIZE 4096

struct dx_pcie_internal_stats {
	atomic64_t helper_acquire_cnt;
	atomic64_t helper_acquire_fail;
	atomic64_t helper_round_sum;
	atomic64_t helper_round_max;
	atomic64_t helper_ch2;
	atomic64_t helper_ch3;
	atomic64_t helper_llm_cnt;
	atomic64_t helper_llm_fail;
	atomic64_t helper_llm_ns_sum;
	atomic64_t helper_llm_ns_max;
	atomic64_t pool_alloc_cnt;
	atomic64_t pool_alloc_short;
	atomic64_t pool_burst_sum;
	atomic64_t pool_alloc_ns_sum;
	atomic64_t pool_alloc_ns_max;
};

static struct dx_pcie_internal_stats g_pcie_internal_stats[DX_PCIE_PROF_DEV_MAX];
static char dx_stats_buff[DX_STATS_PRT_MAX_BUFFER_SIZE];

static bool dx_pcie_perf_valid_dev(int dev_n)
{
	return dev_n >= 0 && dev_n < DX_PCIE_PROF_DEV_MAX;
}

static void dx_atomic64_update_max(atomic64_t *max, uint64_t val)
{
	s64 old;

	old = atomic64_read(max);
	while (val > (uint64_t)old) {
		s64 prev = atomic64_cmpxchg(max, old, (s64)val);

		if (prev == old)
			break;
		old = prev;
	}
}

static void dx_atomic_update_max(atomic_t *max, int val)
{
	int old;

	old = atomic_read(max);
	while (val > old) {
		int prev = atomic_cmpxchg(max, old, val);

		if (prev == old)
			break;
		old = prev;
	}
}

/*
 * DMA-transfer overlap tracking (peak-only).
 *
 * g_dma_xfer_inflight  = transfers in flight across ALL channels.
 * g_chan_inflight[..]  = transfers in flight on ONE channel.
 * Only the exact high-water of each is kept. Two numbers answer the overlap
 * question without a full latency-vs-concurrency table:
 *   - cross-channel peak > 1 => multi-channel overlap exists (link contention).
 *   - same-channel peak  > 1 => the same channel serialises transfers (HW queuing);
 *     if it stays 1, a large min/max spread is NOT same-channel — it is SW
 *     (IRQ/scheduling) or a measurement artifact (see perf_outliers/perf_floor_us).
 */
static atomic_t g_dma_xfer_inflight;
static atomic_t g_dma_xfer_peak;    /* exact high-water across all channels */
static atomic_t g_chan_inflight[16][4][2];
static atomic_t g_chan_depth_peak;  /* exact high-water on a single channel */

static void dx_pcie_conc_clear(void)
{
	int dev, dma, ch;

	atomic_set(&g_dma_xfer_inflight, 0);
	atomic_set(&g_dma_xfer_peak, 0);
	atomic_set(&g_chan_depth_peak, 0);
	for (dev = 0; dev < 16; dev++)
		for (dma = 0; dma < 4; dma++)
			for (ch = 0; ch < 2; ch++)
				atomic_set(&g_chan_inflight[dev][dma][ch], 0);
}

/*
 * Per-stage outlier capture.
 *
 * Aggregate min/max cannot explain WHY a stage fluctuates. A sample is logged
 * with the context live at completion (CPU, in-flight DMA concurrency, size,
 * timestamp) when it is either:
 *   - slower than perf_outlier_us  (tail latency: contention/scheduling/reclaim), or
 *   - faster than perf_floor_us    (suspect/impossible sample, e.g. a DMA-transfer
 *                                   window measuring far below the PCIe line rate).
 * The floor side matters because min/BW-max can be corrupted by measurement
 * artifacts that the slow-only threshold can never catch.
 */
#define DX_PCIE_OUTLIER_MAX  128  /* ring capacity (power of 2) */
#define DX_PCIE_OUTLIER_SHOW 24   /* most-recent entries printed (sysfs page limit) */

struct dx_pcie_outlier {
	uint64_t elapsed_ns;
	uint64_t ts_ns;
	uint64_t size;
	uint32_t conc;
	uint16_t cpu;
	uint8_t  type;
	uint8_t  dev;
	uint8_t  dma;
	uint8_t  ch;
	uint8_t  fast;   /* 1: captured by floor (too-fast/suspect), 0: by threshold */
	uint8_t  valid;
};

static struct dx_pcie_outlier g_outliers[DX_PCIE_OUTLIER_MAX];
static atomic_t g_outlier_idx;
static atomic_t g_outlier_stage_cnt[PCIE_PERF_MAX_T];
static atomic_t g_outlier_fast_cnt;             /* floor-side (suspect) captures */
static uint32_t g_outlier_thresh_ns = 500000;   /* default 500us; 0 disables slow capture */
static uint32_t g_outlier_floor_ns;             /* default 0 disables fast capture */
static bool g_perf_size_reset = true;           /* default: clear stats on transfer size change */

bool dx_pcie_size_reset_enabled(void)
{
	return READ_ONCE(g_perf_size_reset);
}

void dx_pcie_set_size_reset_enabled(bool val)
{
	WRITE_ONCE(g_perf_size_reset, val);
}

uint32_t dx_pcie_outlier_thresh_us(void)
{
	return READ_ONCE(g_outlier_thresh_ns) / 1000;
}

void dx_pcie_outlier_set_thresh_us(uint32_t us)
{
	WRITE_ONCE(g_outlier_thresh_ns, us * 1000);
}

uint32_t dx_pcie_outlier_floor_us(void)
{
	return READ_ONCE(g_outlier_floor_ns) / 1000;
}

void dx_pcie_outlier_set_floor_us(uint32_t us)
{
	WRITE_ONCE(g_outlier_floor_ns, us * 1000);
}

static void dx_pcie_outlier_clear(void)
{
	int i;

	atomic_set(&g_outlier_idx, 0);
	atomic_set(&g_outlier_fast_cnt, 0);
	for (i = 0; i < PCIE_PERF_MAX_T; i++)
		atomic_set(&g_outlier_stage_cnt[i], 0);
	for (i = 0; i < DX_PCIE_OUTLIER_MAX; i++)
		WRITE_ONCE(g_outliers[i].valid, 0);
}

static void dx_pcie_outlier_maybe_record(int type, int dev_n, int dma_n,
					 int ch_n, uint64_t ns, uint64_t size)
{
	uint32_t thr = READ_ONCE(g_outlier_thresh_ns);
	uint32_t floor = READ_ONCE(g_outlier_floor_ns);
	bool slow = (thr != 0 && ns >= thr);
	bool fast = (floor != 0 && ns < floor);
	struct dx_pcie_outlier *o;
	unsigned int idx;

	if (!slow && !fast)
		return;
	if (type >= 0 && type < PCIE_PERF_MAX_T)
		atomic_inc(&g_outlier_stage_cnt[type]);
	if (fast)
		atomic_inc(&g_outlier_fast_cnt);

	idx = (unsigned int)atomic_inc_return(&g_outlier_idx) - 1;
	o = &g_outliers[idx % DX_PCIE_OUTLIER_MAX];
	WRITE_ONCE(o->valid, 0);
	o->elapsed_ns = ns;
	o->ts_ns = ktime_get_ns();
	o->size = size;
	o->conc = (uint32_t)atomic_read(&g_dma_xfer_inflight);
	o->cpu = (uint16_t)raw_smp_processor_id();
	o->type = (uint8_t)type;
	o->dev = (uint8_t)dev_n;
	o->dma = (uint8_t)dma_n;
	o->ch = (uint8_t)ch_n;
	o->fast = fast ? 1 : 0;
	WRITE_ONCE(o->valid, 1);
}

/*
 * Top-N slowest transfers (by ioctl total latency) with per-stage breakdown.
 *
 * At each PCIE_TOTAL_TIME_T end we snapshot the per-stage 'last_t' of that
 * channel and keep the record if it ranks among the slowest DX_PCIE_TOP_N.
 * The breakdown shows WHERE each slow transfer spent its time (which stage
 * dominates), so a big total can be attributed at a glance.
 *
 * Kept per direction so a slow H2C burst cannot evict every C2H sample
 * (and vice versa) - the two paths have very different latency profiles.
 */
#define DX_PCIE_TOP_N 10
#define DX_PCIE_TOP_DIRS 2

struct dx_pcie_top_entry {
	uint64_t total_ns;
	uint64_t stage_ns[PCIE_PERF_MAX_T];
	uint64_t ts_ns;
	uint64_t size;
	uint8_t  dev;
	uint8_t  dma;
	uint8_t  ch;
	uint8_t  valid;
};

static struct dx_pcie_top_entry g_top[DX_PCIE_TOP_DIRS][DX_PCIE_TOP_N];
static DEFINE_SPINLOCK(g_top_lock);

static void dx_pcie_top_clear(void)
{
	unsigned long flags;

	spin_lock_irqsave(&g_top_lock, flags);
	memset(g_top, 0, sizeof(g_top));
	spin_unlock_irqrestore(&g_top_lock, flags);
}

static void dx_pcie_top_record(int dev_n, int dma_n, int ch_n, uint64_t size,
			       uint64_t total_ns)
{
	struct dx_pcie_top_entry *tbl = g_top[ch_n & 1];
	unsigned long flags;
	uint64_t min_val;
	int i, min_idx = 0;

	spin_lock_irqsave(&g_top_lock, flags);
	min_val = tbl[0].total_ns;
	for (i = 1; i < DX_PCIE_TOP_N; i++) {
		if (tbl[i].total_ns < min_val) {
			min_val = tbl[i].total_ns;
			min_idx = i;
		}
	}
	if (total_ns > min_val) {
		struct dx_pcie_top_entry *e = &tbl[min_idx];

		e->total_ns = total_ns;
		e->ts_ns = ktime_get_ns();
		e->size = size;
		e->dev = (uint8_t)dev_n;
		e->dma = (uint8_t)dma_n;
		e->ch = (uint8_t)ch_n;
		e->valid = 1;
		for (i = 0; i < PCIE_PERF_MAX_T; i++)
			e->stage_ns[i] = g_pcie_prof[dev_n][dma_n][ch_n][i].last_t;
	}
	spin_unlock_irqrestore(&g_top_lock, flags);
}


static void dx_pcie_internal_stats_clear_one(struct dx_pcie_internal_stats *s)
{
	atomic64_set(&s->helper_acquire_cnt, 0);
	atomic64_set(&s->helper_acquire_fail, 0);
	atomic64_set(&s->helper_round_sum, 0);
	atomic64_set(&s->helper_round_max, 0);
	atomic64_set(&s->helper_ch2, 0);
	atomic64_set(&s->helper_ch3, 0);
	atomic64_set(&s->helper_llm_cnt, 0);
	atomic64_set(&s->helper_llm_fail, 0);
	atomic64_set(&s->helper_llm_ns_sum, 0);
	atomic64_set(&s->helper_llm_ns_max, 0);
	atomic64_set(&s->pool_alloc_cnt, 0);
	atomic64_set(&s->pool_alloc_short, 0);
	atomic64_set(&s->pool_burst_sum, 0);
	atomic64_set(&s->pool_alloc_ns_sum, 0);
	atomic64_set(&s->pool_alloc_ns_max, 0);
}

void get_start_time(ktime_t *s)
{
	*s = ktime_get();
}

uint64_t get_elapsed_time_ns(ktime_t s)
{
	ktime_t e = ktime_get();
	return ktime_to_ns(ktime_sub(e, s));
}

static __maybe_unused const char* get_pcie_type_string(int type)
{
	const char* type_str;
	switch (type)
	{
		case PCIE_SG_ALLOC_T:
			type_str = "SG Alloc";
			break;
		case PCIE_USER_MAP_T:
			type_str = "User Pinning";
			break;
		case PCIE_DMA_MAP_T:
			type_str = "DMA Mapping";
			break;
		case PCIE_DMA_PREP_T:
			type_str = "DMA Prep";
			break;
		case PCIE_DESC_GEN_T:
			type_str = "Desc Gen";
			break;
		case PCIE_DMA_XFER_T:
			type_str = "DMA Transfer";
			break;
		case PCIE_ISR_EXEC_T:
			type_str = "ISR Exec";
			break;
		case PCIE_WAKEUP_LATENCY_T:
			type_str = "Wakeup Latency";
			break;
		case PCIE_POST_PROCESS_T:
			type_str = "Post Process";
			break;
		case PCIE_USER_UNMAP_T:
			type_str = "User Unmap";
			break;
		case PCIE_SG_FREE_T:
			type_str = "SG Free";
			break;
		case PCIE_LLI_ACQUIRE_T:
			type_str = "LLI AcqWait";
			break;
		case PCIE_LLI_COPY_T:
			type_str = "LLI Copy";
			break;
		case PCIE_KERNEL_DMA_TOTAL_T:
			type_str = "Kernel DMA Total";
			break;
		case PCIE_TOTAL_TIME_T:
			type_str = "Total Latency";
			break;
		default:
			type_str = "unknown";
			break;
	}
	return type_str;
}

static __maybe_unused const char* get_pcie_ctx_string(int type)
{
	const char* ctx_str;
	switch (type)
	{
		case PCIE_SG_ALLOC_T:
		case PCIE_DMA_MAP_T:
		case PCIE_DMA_PREP_T:
		case PCIE_DESC_GEN_T:
		case PCIE_POST_PROCESS_T:
		case PCIE_USER_UNMAP_T:
		case PCIE_SG_FREE_T:
			ctx_str = "[KERN]";
			break;
		case PCIE_USER_MAP_T:
			ctx_str = "[U->K]";
			break;
		case PCIE_DMA_XFER_T:
			ctx_str = "[HW]";
			break;
		case PCIE_ISR_EXEC_T:
			ctx_str = "[ISR]";
			break;
		case PCIE_WAKEUP_LATENCY_T:
			ctx_str = "[K->U]";
			break;
		case PCIE_LLI_ACQUIRE_T:
		case PCIE_LLI_COPY_T:
			ctx_str = "[HELP]";
			break;
		case PCIE_KERNEL_DMA_TOTAL_T:
		case PCIE_TOTAL_TIME_T:
			ctx_str = "[ALL]";
			break;
		default:
			ctx_str = "[UNK]";
			break;
	}
	return ctx_str;
}

#define DX_DBG_PRT_MAX_BUFFER_SIZE	40960
/* Integer 90% threshold: kernel code is built -mgeneral-regs-only (no FP). */
#define DX_BUF_SIZE_CHECKER(offset) do {\
	if ((offset) > DX_DBG_PRT_MAX_BUFFER_SIZE / 10 * 9) goto show_exit;\
} while(0);
static char dx_buff[DX_DBG_PRT_MAX_BUFFER_SIZE];

static void sprint_time(char *buf, uint64_t ns)
{
	if (ns < 1000) { /* < 1us : ns */
		sprintf(buf, "%5lluns", ns);
	} else if (ns < 1000000) { /* < 1ms : us */
		sprintf(buf, "%5lluus", ns / 1000);
	} else if (ns < 1000000000) { /* < 1s : ms */
		/* 1.2ms, 12.3ms, 123.4ms */
		uint64_t ms = ns / 1000000;
		uint64_t rem = ns % 1000000;
		sprintf(buf, "%3llu.%01llums", ms, rem / 100000);
	} else { /* >= 1s : s */
		/* 1.2s, 12.3s */
		uint64_t s = ns / 1000000000;
		uint64_t rem = ns % 1000000000;
		sprintf(buf, "%3llu.%01llus ", s, rem / 100000000);
	}
}

/* Compact variant of sprint_time (no fixed-width padding) for min/avg/max ranges */
static void sprint_time_compact(char *buf, uint64_t ns)
{
	if (ns < 1000) { /* < 1us : ns */
		sprintf(buf, "%lluns", ns);
	} else if (ns < 1000000) { /* < 1ms : us */
		sprintf(buf, "%lluus", ns / 1000);
	} else if (ns < 1000000000) { /* < 1s : ms */
		uint64_t ms = ns / 1000000;
		uint64_t rem = ns % 1000000;
		sprintf(buf, "%llu.%01llums", ms, rem / 100000);
	} else { /* >= 1s : s */
		uint64_t s = ns / 1000000000;
		uint64_t rem = ns % 1000000000;
		sprintf(buf, "%llu.%01llus", s, rem / 100000000);
	}
}

/* Renders "avg(min~max)" so per-section fluctuation is visible alongside the average */
static void sprint_time_range(char *buf, size_t bufsz, const dx_pcie_profiler_t *p)
{
	char avg_s[24], min_s[24], max_s[24];

	if (p->count == 0) {
		snprintf(buf, bufsz, "N/A");
		return;
	}
	sprint_time_compact(avg_s, p->perf_avg_t);
	sprint_time_compact(min_s, p->perf_min_t);
	sprint_time_compact(max_s, p->perf_max_t);
	snprintf(buf, bufsz, "%s(%s~%s)", avg_s, min_s, max_s);
}

static void sprint_size(char *buf, uint64_t size)
{
	if (size < 1024) {
		sprintf(buf, "%llu B", size);
	} else if (size < 1024*1024) {
		sprintf(buf, "%llu.%llu KB", size >> 10, (size & 1023) * 10 / 1024);
	} else if (size < 1024*1024*1024) {
		sprintf(buf, "%llu.%llu MB", size >> 20, (size & 0xFFFFF) * 10 / (1024*1024));
	} else {
		sprintf(buf, "%llu.%llu GB", size >> 30, (size & 0x3FFFFFFF) * 10 / (1024*1024*1024));
	}
}

/* Source strings for building exact-width pivot table dividers via "%.*s" */
static const char DASH80[] =
	"--------------------------------------------------------------------------------";
static const char EQUALS80[] =
	"================================================================================";

char *show_pcie_profile(void)
{
	int dev = 0, dma = 0, ch = 0, i;
	int offset = 0;
	char *ret;
	int *active_devs = NULL;
	int *active_dmas = NULL;
	int *active_chs = NULL;
	int active_count = 0;
	uint64_t total_count = 0;
	uint64_t total_bw = 0;
	uint64_t total_time_sum = 0;
	dx_pcie_profiler_t *p;
	uint64_t sw_prep, hw_exec, compl, total, bw;
	char t_sw[16], t_hw[16], t_compl[16], t_total[16], t_size[16], t_total_sum[16];

	active_devs = kcalloc(128, sizeof(int), GFP_KERNEL);
	active_dmas = kcalloc(128, sizeof(int), GFP_KERNEL);
	active_chs = kcalloc(128, sizeof(int), GFP_KERNEL);
	if (!active_devs || !active_dmas || !active_chs) {
		ret = NULL;
		goto out_free;
	}

	memset(dx_buff, 0x00, sizeof(dx_buff));

	/* 1. Collect Active Channels */
	for (dev = 0; dev < 16; dev++) {
		for (dma = 0; dma < 4; dma++) {
			for (ch = 0; ch < 2; ch++) {
				if (g_pcie_prof[dev][dma][ch][PCIE_DMA_XFER_T].in_use) {
					active_devs[active_count] = dev;
					active_dmas[active_count] = dma;
					active_chs[active_count] = ch;
					active_count++;
					if (active_count >= 128) goto collection_done;
				}
			}
		}
	}

collection_done:
	if (active_count == 0) {
		offset += sprintf(dx_buff+offset, "No active profiling data found.\n");
		goto show_exit;
	}

	/* 2. Summary View */
	offset += sprintf(dx_buff+offset, "================================================================================================================\n");
	offset += sprintf(dx_buff+offset, " CH ID    | Dir | Count | Size       | SW Prep  | HW Exec  | Compl.   | Total    | Bandwidth\n");
	offset += sprintf(dx_buff+offset, "================================================================================================================\n");

	for (i = 0; i < active_count; i++) {
		dev = active_devs[i];
		dma = active_dmas[i];
		ch = active_chs[i];
		p = g_pcie_prof[dev][dma][ch];
		
		sw_prep = p[PCIE_SG_ALLOC_T].perf_avg_t + p[PCIE_USER_MAP_T].perf_avg_t + 
						   p[PCIE_DMA_MAP_T].perf_avg_t + p[PCIE_DMA_PREP_T].perf_avg_t + 
						   p[PCIE_DESC_GEN_T].perf_avg_t;
		hw_exec = p[PCIE_DMA_XFER_T].perf_avg_t;
		compl = p[PCIE_ISR_EXEC_T].perf_avg_t + p[PCIE_WAKEUP_LATENCY_T].perf_avg_t + 
						 p[PCIE_POST_PROCESS_T].perf_avg_t + p[PCIE_USER_UNMAP_T].perf_avg_t +
						 p[PCIE_SG_FREE_T].perf_avg_t;
		total = p[PCIE_TOTAL_TIME_T].perf_avg_t;
		bw = (hw_exec > 0) ? PCIE_GET_BW(p[PCIE_DMA_XFER_T].size, hw_exec) : 0;

		sprint_time(t_sw, sw_prep);
		sprint_time(t_hw, hw_exec);
		sprint_time(t_compl, compl);
		sprint_time(t_total, total);
		sprint_size(t_size, p[PCIE_DMA_XFER_T].size);

		offset += sprintf(dx_buff+offset, " %d:%d:%d    | %s  | %5llu | %-10s | %-8s | %-8s | %-8s | %-8s | %llu MB/s\n",
			dev, dma, ch, ch==0?"WR":"RD",
			p[PCIE_DMA_XFER_T].count, t_size,
			t_sw, t_hw, t_compl, t_total, bw);
		
		total_count += p[PCIE_DMA_XFER_T].count;
		total_bw += bw;
		total_time_sum += total;
	}
	
	sprint_time(t_total_sum, total_time_sum);

	offset += sprintf(dx_buff+offset, "----------------------------------------------------------------------------------------------------------------\n");
	offset += sprintf(dx_buff+offset, " TOTAL    |     |       |            |          |          |          | %-8s |          \n",
		t_total_sum);
	offset += sprintf(dx_buff+offset, "================================================================================================================\n\n");

	/* 3. Pivot View (per-metric values shown as avg(min~max) to expose fluctuation) */
#define PIVOT_COL_W 24
	offset += sprintf(dx_buff+offset, "================================================================================================================\n");
	offset += sprintf(dx_buff+offset, " Values are avg(min~max) per metric\n");
	offset += sprintf(dx_buff+offset, " %-16s |", "Metric");
	for (i = 0; i < active_count; i++) {
		char hdr_buf[16];

		snprintf(hdr_buf, sizeof(hdr_buf), "%d:%d:%d(%s)",
			active_devs[i], active_dmas[i], active_chs[i], active_chs[i]==0?"W":"R");
		offset += sprintf(dx_buff+offset, " %-*s|", PIVOT_COL_W, hdr_buf);
		DX_BUF_SIZE_CHECKER(offset);
	}
	offset += sprintf(dx_buff+offset, "\n");
	offset += sprintf(dx_buff+offset, "================================================================================================================\n");

	// Helper macro for rows
	#define PRINT_PIVOT_ROW(title, type_idx) \
		offset += sprintf(dx_buff+offset, " %-16s |", title); \
		for (i = 0; i < active_count; i++) { \
			char t_buf[32]; \
			sprint_time_range(t_buf, sizeof(t_buf), \
				&g_pcie_prof[active_devs[i]][active_dmas[i]][active_chs[i]][type_idx]); \
			offset += sprintf(dx_buff+offset, " %-*s|", PIVOT_COL_W, t_buf); \
			DX_BUF_SIZE_CHECKER(offset); \
		} \
		offset += sprintf(dx_buff+offset, "\n");

	// Size Row
	offset += sprintf(dx_buff+offset, " %-16s |", "Size");
	for (i = 0; i < active_count; i++) {
		char t_buf[16];
		sprint_size(t_buf, g_pcie_prof[active_devs[i]][active_dmas[i]][active_chs[i]][PCIE_DMA_XFER_T].size);
		offset += sprintf(dx_buff+offset, " %-*s|", PIVOT_COL_W, t_buf);
		DX_BUF_SIZE_CHECKER(offset);
	}
	offset += sprintf(dx_buff+offset, "\n");

	// Count Row
	offset += sprintf(dx_buff+offset, " %-16s |", "Count");
	for (i = 0; i < active_count; i++) {
		offset += sprintf(dx_buff+offset, " %-*llu|", PIVOT_COL_W, g_pcie_prof[active_devs[i]][active_dmas[i]][active_chs[i]][PCIE_DMA_XFER_T].count);
		DX_BUF_SIZE_CHECKER(offset);
	}
	offset += sprintf(dx_buff+offset, "\n");
	
	offset += sprintf(dx_buff+offset, "------------------+");
	for(i=0; i<active_count; i++) offset += sprintf(dx_buff+offset, "%.*s+", PIVOT_COL_W+1, DASH80);
	offset += sprintf(dx_buff+offset, "\n");

	PRINT_PIVOT_ROW("SG Alloc", PCIE_SG_ALLOC_T);
	PRINT_PIVOT_ROW("User Pinning", PCIE_USER_MAP_T);
	PRINT_PIVOT_ROW("DMA Mapping", PCIE_DMA_MAP_T);
	PRINT_PIVOT_ROW("DMA Prep", PCIE_DMA_PREP_T);
	PRINT_PIVOT_ROW("Desc Gen", PCIE_DESC_GEN_T);
	
	offset += sprintf(dx_buff+offset, "------------------+");
	for(i=0; i<active_count; i++) offset += sprintf(dx_buff+offset, "%.*s+", PIVOT_COL_W+1, DASH80);
	offset += sprintf(dx_buff+offset, "\n");

	PRINT_PIVOT_ROW("DMA Transfer", PCIE_DMA_XFER_T);
	/* Sub-spans contained inside DMA Transfer above - not additive with it. */
	PRINT_PIVOT_ROW(" +LLI AcqWait", PCIE_LLI_ACQUIRE_T);
	PRINT_PIVOT_ROW(" +LLI Copy", PCIE_LLI_COPY_T);

	offset += sprintf(dx_buff+offset, "------------------+");
	for(i=0; i<active_count; i++) offset += sprintf(dx_buff+offset, "%.*s+", PIVOT_COL_W+1, DASH80);
	offset += sprintf(dx_buff+offset, "\n");

	PRINT_PIVOT_ROW("ISR Exec", PCIE_ISR_EXEC_T);
	PRINT_PIVOT_ROW("Wakeup Latency", PCIE_WAKEUP_LATENCY_T);
	PRINT_PIVOT_ROW("Post Process", PCIE_POST_PROCESS_T);
	PRINT_PIVOT_ROW("User Unmap", PCIE_USER_UNMAP_T);
	PRINT_PIVOT_ROW("SG Free", PCIE_SG_FREE_T);

	offset += sprintf(dx_buff+offset, "------------------+");
	for(i=0; i<active_count; i++) offset += sprintf(dx_buff+offset, "%.*s+", PIVOT_COL_W+1, DASH80);
	offset += sprintf(dx_buff+offset, "\n");

	/* dw_edma_sg_process scope (inner kernel DMA total: Map..Post) */
	PRINT_PIVOT_ROW("Kernel DMA", PCIE_KERNEL_DMA_TOTAL_T);

	offset += sprintf(dx_buff+offset, "==================+");
	for(i=0; i<active_count; i++) offset += sprintf(dx_buff+offset, "%.*s+", PIVOT_COL_W+1, EQUALS80);
	offset += sprintf(dx_buff+offset, "\n");

	PRINT_PIVOT_ROW("Total", PCIE_TOTAL_TIME_T);

	/* Other(gap) = Total.avg - sum(named stage avgs): time not attributed to any
	 * named stage (thread dispatch, mutex wait, doorbell, inter-stage gaps).
	 * avg-only; a large value means an uninstrumented phase dominates. */
	offset += sprintf(dx_buff+offset, " %-16s |", "Other(gap)");
	for (i = 0; i < active_count; i++) {
		dx_pcie_profiler_t *pp = g_pcie_prof[active_devs[i]][active_dmas[i]][active_chs[i]];
		uint64_t total = pp[PCIE_TOTAL_TIME_T].perf_avg_t;
		uint64_t sum = pp[PCIE_SG_ALLOC_T].perf_avg_t + pp[PCIE_USER_MAP_T].perf_avg_t +
			pp[PCIE_DMA_MAP_T].perf_avg_t + pp[PCIE_DMA_PREP_T].perf_avg_t +
			pp[PCIE_DESC_GEN_T].perf_avg_t + pp[PCIE_DMA_XFER_T].perf_avg_t +
			pp[PCIE_ISR_EXEC_T].perf_avg_t + pp[PCIE_WAKEUP_LATENCY_T].perf_avg_t +
			pp[PCIE_POST_PROCESS_T].perf_avg_t + pp[PCIE_USER_UNMAP_T].perf_avg_t +
			pp[PCIE_SG_FREE_T].perf_avg_t;
		char o_buf[24];

		if (pp[PCIE_TOTAL_TIME_T].count == 0)
			snprintf(o_buf, sizeof(o_buf), "N/A");
		else if (total > sum)
			sprint_time_compact(o_buf, total - sum);
		else
			snprintf(o_buf, sizeof(o_buf), "~0");
		offset += sprintf(dx_buff+offset, " %-*s|", PIVOT_COL_W, o_buf);
		DX_BUF_SIZE_CHECKER(offset);
	}
	offset += sprintf(dx_buff+offset, "\n");

	// Bandwidth Row (min/max derived from the DMA Transfer time extremes)
	offset += sprintf(dx_buff+offset, " %-16s |", "Bandwidth");
	for (i = 0; i < active_count; i++) {
		dx_pcie_profiler_t *p = g_pcie_prof[active_devs[i]][active_dmas[i]][active_chs[i]];
		uint64_t xfer_avg = p[PCIE_DMA_XFER_T].perf_avg_t;
		uint64_t xfer_min = p[PCIE_DMA_XFER_T].perf_min_t;
		uint64_t xfer_max = p[PCIE_DMA_XFER_T].perf_max_t;
		uint64_t size = p[PCIE_DMA_XFER_T].size;
		char bw_buf[32];

		if (p[PCIE_DMA_XFER_T].count == 0) {
			snprintf(bw_buf, sizeof(bw_buf), "N/A");
		} else {
			uint64_t bw_avg = (xfer_avg > 0) ? PCIE_GET_BW(size, xfer_avg) : 0;
			uint64_t bw_max = (xfer_min > 0) ? PCIE_GET_BW(size, xfer_min) : 0;
			uint64_t bw_min = (xfer_max > 0) ? PCIE_GET_BW(size, xfer_max) : 0;

			snprintf(bw_buf, sizeof(bw_buf), "%llu(%llu~%llu)MB/s", bw_avg, bw_min, bw_max);
		}
		offset += sprintf(dx_buff+offset, " %-*s|", PIVOT_COL_W, bw_buf);
		DX_BUF_SIZE_CHECKER(offset);
	}
	offset += sprintf(dx_buff+offset, "\n");
	offset += sprintf(dx_buff+offset, "================================================================================================================\n");

show_exit:
	if (offset > DX_DBG_PRT_MAX_BUFFER_SIZE) {
		pr_err("Please check buffer size (%d/%d)\n", offset, DX_DBG_PRT_MAX_BUFFER_SIZE);
		ret = NULL;
	} else {
		ret = dx_buff;
	}

out_free:
	kfree(active_devs);
	kfree(active_dmas);
	kfree(active_chs);
	return ret;
}

void dx_pcie_perf_clear_internal_stats(void)
{
	int dev;

	for (dev = 0; dev < DX_PCIE_PROF_DEV_MAX; dev++)
		dx_pcie_internal_stats_clear_one(&g_pcie_internal_stats[dev]);
}

void dx_pcie_perf_record_helper_acquire(int dev_n, int rounds,
					       int channel, bool success)
{
	struct dx_pcie_internal_stats *s;

	if (!READ_ONCE(g_perf_enabled) || !dx_pcie_perf_valid_dev(dev_n))
		return;

	if (rounds < 0)
		rounds = 0;

	s = &g_pcie_internal_stats[dev_n];
	atomic64_inc(&s->helper_acquire_cnt);
	atomic64_add(rounds, &s->helper_round_sum);
	dx_atomic64_update_max(&s->helper_round_max, rounds);

	if (!success) {
		atomic64_inc(&s->helper_acquire_fail);
		return;
	}

	if (channel == 2)
		atomic64_inc(&s->helper_ch2);
	else if (channel == 3)
		atomic64_inc(&s->helper_ch3);
}

void dx_pcie_perf_record_helper_llm_copy(int dev_n, uint64_t ns, int ret)
{
	struct dx_pcie_internal_stats *s;

	if (!READ_ONCE(g_perf_enabled) || !dx_pcie_perf_valid_dev(dev_n))
		return;

	s = &g_pcie_internal_stats[dev_n];
	atomic64_inc(&s->helper_llm_cnt);
	atomic64_add(ns, &s->helper_llm_ns_sum);
	dx_atomic64_update_max(&s->helper_llm_ns_max, ns);
	if (ret)
		atomic64_inc(&s->helper_llm_fail);
}

void dx_pcie_perf_record_pool_alloc(int dev_n, uint32_t requested,
					   uint32_t allocated, uint64_t ns)
{
	struct dx_pcie_internal_stats *s;

	if (!READ_ONCE(g_perf_enabled) || !dx_pcie_perf_valid_dev(dev_n))
		return;

	s = &g_pcie_internal_stats[dev_n];
	atomic64_inc(&s->pool_alloc_cnt);
	atomic64_add(allocated, &s->pool_burst_sum);
	atomic64_add(ns, &s->pool_alloc_ns_sum);
	dx_atomic64_update_max(&s->pool_alloc_ns_max, ns);
	if (allocated < requested)
		atomic64_inc(&s->pool_alloc_short);
}

char *show_pcie_internal_stats(void)
{
	int dev;
	int offset = 0;
	bool any = false;

	memset(dx_stats_buff, 0x00, sizeof(dx_stats_buff));
	offset += scnprintf(dx_stats_buff + offset,
		DX_STATS_PRT_MAX_BUFFER_SIZE - offset,
		"DMA internal telemetry (recorded only while perf_enable=1)\n");
	offset += scnprintf(dx_stats_buff + offset,
		DX_STATS_PRT_MAX_BUFFER_SIZE - offset,
		"dev | helper acq/fail avg_round max_round ch2/ch3 | lli cnt/fail avg_us max_us | pool cnt/short avg_burst avg_us max_us\n");
	offset += scnprintf(dx_stats_buff + offset,
		DX_STATS_PRT_MAX_BUFFER_SIZE - offset,
		"----+--------------------------------------------+-----------------------------+------------------------------------------\n");

	for (dev = 0; dev < DX_PCIE_PROF_DEV_MAX; dev++) {
		struct dx_pcie_internal_stats *s = &g_pcie_internal_stats[dev];
		uint64_t helper_cnt = atomic64_read(&s->helper_acquire_cnt);
		uint64_t lli_cnt = atomic64_read(&s->helper_llm_cnt);
		uint64_t pool_cnt = atomic64_read(&s->pool_alloc_cnt);
		uint64_t avg_round = helper_cnt ?
			div64_u64(atomic64_read(&s->helper_round_sum), helper_cnt) : 0;
		uint64_t avg_lli_us = lli_cnt ?
			div64_u64(atomic64_read(&s->helper_llm_ns_sum), lli_cnt * 1000) : 0;
		uint64_t avg_pool_burst = pool_cnt ?
			div64_u64(atomic64_read(&s->pool_burst_sum), pool_cnt) : 0;
		uint64_t avg_pool_us = pool_cnt ?
			div64_u64(atomic64_read(&s->pool_alloc_ns_sum), pool_cnt * 1000) : 0;

		if (!helper_cnt && !lli_cnt && !pool_cnt)
			continue;

		any = true;
		offset += scnprintf(dx_stats_buff + offset,
			DX_STATS_PRT_MAX_BUFFER_SIZE - offset,
			"%3d | %6llu/%-4llu %9llu %9llu %3llu/%-3llu | %5llu/%-4llu %6llu %6llu | %5llu/%-5llu %9llu %6llu %6llu\n",
			dev,
			helper_cnt,
			(uint64_t)atomic64_read(&s->helper_acquire_fail),
			avg_round,
			(uint64_t)atomic64_read(&s->helper_round_max),
			(uint64_t)atomic64_read(&s->helper_ch2),
			(uint64_t)atomic64_read(&s->helper_ch3),
			lli_cnt,
			(uint64_t)atomic64_read(&s->helper_llm_fail),
			avg_lli_us,
			div64_u64(atomic64_read(&s->helper_llm_ns_max), 1000),
			pool_cnt,
			(uint64_t)atomic64_read(&s->pool_alloc_short),
			avg_pool_burst,
			avg_pool_us,
			div64_u64(atomic64_read(&s->pool_alloc_ns_max), 1000));

		if (offset > DX_STATS_PRT_MAX_BUFFER_SIZE * 9 / 10)
			break;
	}

	if (!any)
		offset += scnprintf(dx_stats_buff + offset,
			DX_STATS_PRT_MAX_BUFFER_SIZE - offset,
			"No internal telemetry data found.\n");

	return dx_stats_buff;
}

#define DX_CONC_PRT_MAX_BUFFER_SIZE 8192
static char dx_conc_buff[DX_CONC_PRT_MAX_BUFFER_SIZE];

char *show_pcie_concurrency(void)
{
	int offset = 0;
	int xpeak = atomic_read(&g_dma_xfer_peak);
	int cpeak = atomic_read(&g_chan_depth_peak);

	memset(dx_conc_buff, 0x00, sizeof(dx_conc_buff));
	offset += scnprintf(dx_conc_buff + offset,
		DX_CONC_PRT_MAX_BUFFER_SIZE - offset,
		"DMA-transfer overlap peaks (perf_enable=1)\n");
	offset += scnprintf(dx_conc_buff + offset,
		DX_CONC_PRT_MAX_BUFFER_SIZE - offset,
		"  Peak cross-channel in-flight : %d\n", xpeak);
	offset += scnprintf(dx_conc_buff + offset,
		DX_CONC_PRT_MAX_BUFFER_SIZE - offset,
		"  Peak same-channel in-flight  : %d\n", cpeak);
	offset += scnprintf(dx_conc_buff + offset,
		DX_CONC_PRT_MAX_BUFFER_SIZE - offset,
		"Interpretation of a large DMA-transfer min/max spread:\n");
	offset += scnprintf(dx_conc_buff + offset,
		DX_CONC_PRT_MAX_BUFFER_SIZE - offset,
		"  cross-channel > 1 : multi-channel overlap (link bandwidth shared).\n");
	offset += scnprintf(dx_conc_buff + offset,
		DX_CONC_PRT_MAX_BUFFER_SIZE - offset,
		"  same-channel  > 1 : same channel serialises transfers (HW queuing).\n");
	offset += scnprintf(dx_conc_buff + offset,
		DX_CONC_PRT_MAX_BUFFER_SIZE - offset,
		"  both = 1          : NOT overlap -> SW (IRQ/scheduling) or a measurement\n"
		"                      artifact; drill down with perf_outliers / perf_floor_us.\n");

	return dx_conc_buff;
}

#define DX_OUTLIER_PRT_MAX_BUFFER_SIZE 8192
static char dx_outlier_buff[DX_OUTLIER_PRT_MAX_BUFFER_SIZE];

char *show_pcie_outliers(void)
{
	int i, type;
	int offset = 0;
	unsigned int total;
	uint64_t now = ktime_get_ns();

	memset(dx_outlier_buff, 0x00, sizeof(dx_outlier_buff));
	total = (unsigned int)atomic_read(&g_outlier_idx);

	offset += scnprintf(dx_outlier_buff + offset,
		DX_OUTLIER_PRT_MAX_BUFFER_SIZE - offset,
		"Per-stage outliers (perf_enable=1): %u total\n", total);
	offset += scnprintf(dx_outlier_buff + offset,
		DX_OUTLIER_PRT_MAX_BUFFER_SIZE - offset,
		"Slow capture > %u us | Fast capture < %u us (0=off) | fast/suspect: %u\n",
		dx_pcie_outlier_thresh_us(), dx_pcie_outlier_floor_us(),
		(unsigned int)atomic_read(&g_outlier_fast_cnt));

	/* Per-stage outlier counts: shows WHICH stage fluctuates the most. */
	offset += scnprintf(dx_outlier_buff + offset,
		DX_OUTLIER_PRT_MAX_BUFFER_SIZE - offset,
		"Outliers per stage:\n");
	for (type = 0; type < PCIE_PERF_MAX_T; type++) {
		unsigned int c = (unsigned int)atomic_read(&g_outlier_stage_cnt[type]);

		if (c == 0)
			continue;
		offset += scnprintf(dx_outlier_buff + offset,
			DX_OUTLIER_PRT_MAX_BUFFER_SIZE - offset,
			"  %-16s %u\n", get_pcie_type_string(type), c);
	}

	if (total == 0) {
		offset += scnprintf(dx_outlier_buff + offset,
			DX_OUTLIER_PRT_MAX_BUFFER_SIZE - offset,
			"No outliers captured. Lower perf_outlier_us (slow) or set perf_floor_us (fast).\n");
		return dx_outlier_buff;
	}

	/* Most recent samples with the context live at completion. conc = DMA
	 * transfers in flight; interpret: high conc => contention, random low-conc
	 * CPUs => scheduling/reclaim, time-clustered => system-wide event. FAST rows
	 * with an implausibly high MB/s are measurement artifacts (window did not
	 * cover the whole transfer), not real throughput.
	 */
	offset += scnprintf(dx_outlier_buff + offset,
		DX_OUTLIER_PRT_MAX_BUFFER_SIZE - offset,
		"Recent outliers (newest first):\n");
	offset += scnprintf(dx_outlier_buff + offset,
		DX_OUTLIER_PRT_MAX_BUFFER_SIZE - offset,
		"  kind | age_ms | ch id | cpu | conc | elapsed  | MB/s | stage\n");

	for (i = 0; i < DX_PCIE_OUTLIER_SHOW && i < (int)total; i++) {
		unsigned int slot = (total - 1 - i) % DX_PCIE_OUTLIER_MAX;
		struct dx_pcie_outlier *o = &g_outliers[slot];
		char t_el[24];
		uint64_t age_ms, bw;

		if (!READ_ONCE(o->valid))
			continue;

		sprint_time_compact(t_el, o->elapsed_ns);
		age_ms = (now > o->ts_ns) ? div64_u64(now - o->ts_ns, 1000000) : 0;
		bw = (o->size && o->elapsed_ns) ?
			PCIE_GET_BW(o->size, o->elapsed_ns) : 0;

		offset += scnprintf(dx_outlier_buff + offset,
			DX_OUTLIER_PRT_MAX_BUFFER_SIZE - offset,
			"  %-4s | %6llu | %u:%u:%u | %3u | %4u | %-8s | %5llu | %s\n",
			o->fast ? "FAST" : "slow",
			age_ms, o->dev, o->dma, o->ch, o->cpu, o->conc,
			t_el, bw, get_pcie_type_string(o->type));

		if (offset > DX_OUTLIER_PRT_MAX_BUFFER_SIZE * 9 / 10)
			break;
	}

	return dx_outlier_buff;
}

#define DX_TOP_PRT_MAX_BUFFER_SIZE 4096
static char dx_top_buff[DX_TOP_PRT_MAX_BUFFER_SIZE];

char *show_pcie_top(void)
{
	/* static (not on-stack): callers are serialized by perf_stats_lock */
	static struct dx_pcie_top_entry snap[DX_PCIE_TOP_DIRS][DX_PCIE_TOP_N];
	/* index 1 == eDMA read channel (H2C), index 0 == eDMA write channel (C2H) */
	static const int dir_order[DX_PCIE_TOP_DIRS] = { 1, 0 };
	static const char * const dir_name[DX_PCIE_TOP_DIRS] = {
		"DMA Write ch (C2H, host read)",
		"DMA Read ch  (H2C, host write)",
	};
	unsigned long flags;
	uint64_t now = ktime_get_ns();
	int offset = 0, d, i, j;

	spin_lock_irqsave(&g_top_lock, flags);
	memcpy(snap, g_top, sizeof(snap));
	spin_unlock_irqrestore(&g_top_lock, flags);

	memset(dx_top_buff, 0x00, sizeof(dx_top_buff));
	offset += scnprintf(dx_top_buff + offset,
		DX_TOP_PRT_MAX_BUFFER_SIZE - offset,
		"Top %d slowest transfers per direction by ioctl total latency (perf_enable=1)\n",
		DX_PCIE_TOP_N);
	offset += scnprintf(dx_top_buff + offset,
		DX_TOP_PRT_MAX_BUFFER_SIZE - offset,
		"breakdown(us) = SGalloc/Pin/Map/Prep/Desc/XFER/ISR/Wake/Post/Unmap/SGfree ; gap = total - sum(breakdown)\n");
	offset += scnprintf(dx_top_buff + offset,
		DX_TOP_PRT_MAX_BUFFER_SIZE - offset,
		"lli(us) = helper acquire wait / helper LLI copy ; both are already inside XFER\n");

	for (d = 0; d < DX_PCIE_TOP_DIRS; d++) {
		struct dx_pcie_top_entry *tbl = snap[dir_order[d]];
		bool any = false;

		/* selection sort, slowest first */
		for (i = 0; i < DX_PCIE_TOP_N - 1; i++) {
			int mx = i;

			for (j = i + 1; j < DX_PCIE_TOP_N; j++)
				if (tbl[j].total_ns > tbl[mx].total_ns)
					mx = j;
			if (mx != i) {
				struct dx_pcie_top_entry tmp = tbl[i];

				tbl[i] = tbl[mx];
				tbl[mx] = tmp;
			}
		}

		offset += scnprintf(dx_top_buff + offset,
			DX_TOP_PRT_MAX_BUFFER_SIZE - offset,
			"\n[%s]\n", dir_name[dir_order[d]]);
		offset += scnprintf(dx_top_buff + offset,
			DX_TOP_PRT_MAX_BUFFER_SIZE - offset,
			" # | age_ms | ch id | size     | total    | dominant       | breakdown(us) | gap(us) | lli(us)\n");

		for (i = 0; i < DX_PCIE_TOP_N; i++) {
			struct dx_pcie_top_entry *e = &tbl[i];
			char t_total[24], t_size[24];
			uint64_t age_ms, dom = 0, sum_stages, gap_ns;
			int k, dom_idx = 0;

			if (!e->valid || e->total_ns == 0)
				continue;
			any = true;

			sum_stages = 0;
			for (k = PCIE_SG_ALLOC_T; k <= PCIE_SG_FREE_T; k++) {
				sum_stages += e->stage_ns[k];
				if (e->stage_ns[k] > dom) {
					dom = e->stage_ns[k];
					dom_idx = k;
				}
			}
			gap_ns = (e->total_ns > sum_stages) ? e->total_ns - sum_stages : 0;
			sprint_time_compact(t_total, e->total_ns);
			sprint_size(t_size, e->size);
			age_ms = (now > e->ts_ns) ? div64_u64(now - e->ts_ns, 1000000) : 0;

			offset += scnprintf(dx_top_buff + offset,
				DX_TOP_PRT_MAX_BUFFER_SIZE - offset,
				"%2d | %6llu | %u:%u:%u | %-8s | %-8s | %-14s | %llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu | %llu | %llu/%llu\n",
				i + 1, age_ms, e->dev, e->dma, e->ch, t_size, t_total,
				get_pcie_type_string(dom_idx),
				div64_u64(e->stage_ns[PCIE_SG_ALLOC_T], 1000),
				div64_u64(e->stage_ns[PCIE_USER_MAP_T], 1000),
				div64_u64(e->stage_ns[PCIE_DMA_MAP_T], 1000),
				div64_u64(e->stage_ns[PCIE_DMA_PREP_T], 1000),
				div64_u64(e->stage_ns[PCIE_DESC_GEN_T], 1000),
				div64_u64(e->stage_ns[PCIE_DMA_XFER_T], 1000),
				div64_u64(e->stage_ns[PCIE_ISR_EXEC_T], 1000),
				div64_u64(e->stage_ns[PCIE_WAKEUP_LATENCY_T], 1000),
				div64_u64(e->stage_ns[PCIE_POST_PROCESS_T], 1000),
				div64_u64(e->stage_ns[PCIE_USER_UNMAP_T], 1000),
				div64_u64(e->stage_ns[PCIE_SG_FREE_T], 1000),
				div64_u64(gap_ns, 1000),
				div64_u64(e->stage_ns[PCIE_LLI_ACQUIRE_T], 1000),
				div64_u64(e->stage_ns[PCIE_LLI_COPY_T], 1000));

			if (offset > DX_TOP_PRT_MAX_BUFFER_SIZE * 9 / 10)
				break;
		}

		if (!any)
			offset += scnprintf(dx_top_buff + offset,
				DX_TOP_PRT_MAX_BUFFER_SIZE - offset,
				"No transfers recorded yet.\n");
	}

	return dx_top_buff;
}

/* part : [0 - all, 1 - partial] */
void clear_pcie_profile_info(int partial, int type_n, int dev_n, int dma_n, int ch_n)
{
	int dev, dma, ch, type;

	if (partial) {
		for (type = 0; type < PCIE_PERF_MAX_T; type++) {
			dx_pcie_profiler_t *p = &g_pcie_prof[dev_n][dma_n][ch_n][type];
			p->perf_max_t = 0;
			p->perf_min_t = 0xFFFFFFFF;
			p->perf_avg_t = 0;
			p->perf_sum_t = 0;
			p->count = 0;
			/* Do NOT clear pref_t, in_use, or size here to preserve active contexts */
		}
	} else {
		memset(g_pcie_prof, 0, sizeof(g_pcie_prof));
		dx_pcie_perf_clear_internal_stats();
		dx_pcie_conc_clear();
		dx_pcie_outlier_clear();
		dx_pcie_top_clear();
		for (dev = 0; dev < 16; dev++) {
			for (dma = 0; dma < 4; dma++) {
				for (ch = 0; ch < 2; ch++) {
					for (type = 0; type < PCIE_PERF_MAX_T; type++) {
						g_pcie_prof[dev][dma][ch][type].perf_min_t = 0xFFFFFFFF;
					}
				}
			}
		}
	}
}

inline void dx_pcie_start_profile(int type, uint64_t size, int dev_n, int dma_n, int ch_n)
{
	dx_pcie_profiler_t *p;

	if (!READ_ONCE(g_perf_enabled))
		return;

	p = &g_pcie_prof[dev_n][dma_n][ch_n][type];
	/* Transfer size changed: keeping the old samples would pair the new size
	 * with timings measured at the old one, so Bandwidth min/max go impossible.
	 * Toggle via sysfs perf_size_reset if mixed-size traffic needs a running avg. */
	if (type == PCIE_TOTAL_TIME_T && READ_ONCE(g_perf_size_reset) &&
	    p->size && p->size != size) {
		clear_pcie_profile_info(1, type, dev_n, dma_n, ch_n);
		p->size = size;	/* claim it here so concurrent starts do not re-clear */
	}
	if (type == PCIE_TOTAL_TIME_T) {
		int i;

		/* The perf_top breakdown snapshots each stage's last_t, so a stage
		 * this transfer skips would otherwise report the previous
		 * transfer's timing (e.g. Pin on a pre-registered buffer). Reset
		 * them so a skipped stage reads as 0. */
		for (i = 0; i < PCIE_PERF_MAX_T; i++) {
			if (i != PCIE_TOTAL_TIME_T)
				g_pcie_prof[dev_n][dma_n][ch_n][i].last_t = 0;
		}
	}
	p->in_use = 1;
	p->armed = 1;
	get_start_time(&p->pref_t);
	if (type == PCIE_DMA_XFER_T) {
		int cur = atomic_inc_return(&g_dma_xfer_inflight);
		int depth = atomic_inc_return(&g_chan_inflight[dev_n][dma_n][ch_n]);

		dx_atomic_update_max(&g_dma_xfer_peak, cur);
		dx_atomic_update_max(&g_chan_depth_peak, depth);
	}
}

inline void dx_pcie_end_profile(int type, uint64_t size, int dev_n, int dma_n, int ch_n)
{
	dx_pcie_profiler_t *p;
	uint64_t elapsed_t;

	if (!READ_ONCE(g_perf_enabled))
		return;

	p = &g_pcie_prof[dev_n][dma_n][ch_n][type];
	/* Only count a sample that a matching start armed. Guards against a shared
	 * or duplicate ISR ending the same transfer twice (the impossible-fast
	 * min artifact) and against unpaired ends after a mid-flight perf toggle. */
	if (!p->armed)
		return;
	p->armed = 0;
	elapsed_t = get_elapsed_time_ns(p->pref_t);
	p->last_t = elapsed_t;

	p->size = size;
	if (elapsed_t > p->perf_max_t) {
		p->perf_max_t = elapsed_t;
	}
	if (elapsed_t < p->perf_min_t) {
		p->perf_min_t = elapsed_t;
	}
	p->count++;
	p->perf_sum_t += elapsed_t;
	p->perf_avg_t = p->perf_sum_t / p->count;

	dx_pcie_outlier_maybe_record(type, dev_n, dma_n, ch_n, elapsed_t, size);

	if (type == PCIE_DMA_XFER_T) {
		atomic_dec_if_positive(&g_dma_xfer_inflight);
		atomic_dec_if_positive(&g_chan_inflight[dev_n][dma_n][ch_n]);
	} else if (type == PCIE_TOTAL_TIME_T) {
		dx_pcie_top_record(dev_n, dma_n, ch_n, size, elapsed_t);
	}
}

/*
 * Record a stage whose duration the caller already measured.
 *
 * Needed for spans that are entered from several contexts and may repeat
 * within one transfer (helper LLI copy runs per chunk, from thread and from
 * ISR continuation), where the start/end slot pairing cannot hold.
 */
void dx_pcie_record_stage(int type, int dev_n, int dma_n, int ch_n, uint64_t elapsed_t)
{
	dx_pcie_profiler_t *p;

	if (!READ_ONCE(g_perf_enabled))
		return;
	if (type < 0 || type >= PCIE_PERF_MAX_T ||
	    dev_n < 0 || dev_n >= 16 || dma_n < 0 || dma_n >= 4 ||
	    ch_n < 0 || ch_n >= 2)
		return;

	p = &g_pcie_prof[dev_n][dma_n][ch_n][type];
	p->in_use = 1;
	p->last_t = elapsed_t;
	if (elapsed_t > p->perf_max_t)
		p->perf_max_t = elapsed_t;
	if (elapsed_t < p->perf_min_t)
		p->perf_min_t = elapsed_t;
	p->count++;
	p->perf_sum_t += elapsed_t;
	p->perf_avg_t = p->perf_sum_t / p->count;
}
#else
#error "DMA_PERF_MEASURE must be defined — set unconditionally in Kbuild"
#endif /*DMA_PERF_MEASURE*/
