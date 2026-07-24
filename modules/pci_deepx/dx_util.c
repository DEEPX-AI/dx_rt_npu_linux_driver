// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#include <linux/pci.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/atomic.h>
#include <linux/math64.h>

#include "dx_util.h"
#include "dx_lib.h"
#include "dw-edma-thread.h"
#if IS_ENABLED(CONFIG_DX_AI_ACCEL_RT)
#include "dx_pcie_api.h"
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
#define PCI_REBAR_CTRL_BAR_IDX      7
#define PCI_REBAR_CTRL_BAR_SIZE     0x00001F00  /* BAR size */
#define PCI_REBAR_CTRL_BAR_SHIFT    8           /* shift for BAR size */
#endif

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
#define DX_BUF_SIZE_CHECKER(offset) do {\
	if (offset > DX_DBG_PRT_MAX_BUFFER_SIZE*0.9) goto show_exit;\
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
						 p[PCIE_POST_PROCESS_T].perf_avg_t;
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

	/* 3. Pivot View */
	offset += sprintf(dx_buff+offset, "================================================================================================================\n");
	offset += sprintf(dx_buff+offset, " Metric           |");
	for (i = 0; i < active_count; i++) {
		offset += sprintf(dx_buff+offset, " %d:%d:%d(%s)|", 
			active_devs[i], active_dmas[i], active_chs[i], active_chs[i]==0?"W":"R");
	}
	offset += sprintf(dx_buff+offset, "\n");
	offset += sprintf(dx_buff+offset, "================================================================================================================\n");

	// Helper macro for rows
	#define PRINT_PIVOT_ROW(title, type_idx) \
		offset += sprintf(dx_buff+offset, " %-16s |", title); \
		for (i = 0; i < active_count; i++) { \
			char t_buf[16]; \
			sprint_time(t_buf, g_pcie_prof[active_devs[i]][active_dmas[i]][active_chs[i]][type_idx].perf_avg_t); \
			offset += sprintf(dx_buff+offset, " %-9s|", t_buf); \
		} \
		offset += sprintf(dx_buff+offset, "\n");

	// Size Row
	offset += sprintf(dx_buff+offset, " %-16s |", "Size");
	for (i = 0; i < active_count; i++) {
		char t_buf[16];
		sprint_size(t_buf, g_pcie_prof[active_devs[i]][active_dmas[i]][active_chs[i]][PCIE_DMA_XFER_T].size);
		offset += sprintf(dx_buff+offset, " %-9s|", t_buf);
	}
	offset += sprintf(dx_buff+offset, "\n");

	// Count Row
	offset += sprintf(dx_buff+offset, " %-16s |", "Count");
	for (i = 0; i < active_count; i++) {
		offset += sprintf(dx_buff+offset, " %-9llu|", g_pcie_prof[active_devs[i]][active_dmas[i]][active_chs[i]][PCIE_DMA_XFER_T].count);
	}
	offset += sprintf(dx_buff+offset, "\n");
	
	offset += sprintf(dx_buff+offset, "------------------+");
	for(i=0; i<active_count; i++) offset += sprintf(dx_buff+offset, "----------+");
	offset += sprintf(dx_buff+offset, "\n");

	PRINT_PIVOT_ROW("SG Alloc", PCIE_SG_ALLOC_T);
	PRINT_PIVOT_ROW("User Pinning", PCIE_USER_MAP_T);
	PRINT_PIVOT_ROW("DMA Mapping", PCIE_DMA_MAP_T);
	PRINT_PIVOT_ROW("DMA Prep", PCIE_DMA_PREP_T);
	PRINT_PIVOT_ROW("Desc Gen", PCIE_DESC_GEN_T);
	
	offset += sprintf(dx_buff+offset, "------------------+");
	for(i=0; i<active_count; i++) offset += sprintf(dx_buff+offset, "----------+");
	offset += sprintf(dx_buff+offset, "\n");

	PRINT_PIVOT_ROW("DMA Transfer", PCIE_DMA_XFER_T);

	offset += sprintf(dx_buff+offset, "------------------+");
	for(i=0; i<active_count; i++) offset += sprintf(dx_buff+offset, "----------+");
	offset += sprintf(dx_buff+offset, "\n");

	PRINT_PIVOT_ROW("ISR Exec", PCIE_ISR_EXEC_T);
	PRINT_PIVOT_ROW("Wakeup Latency", PCIE_WAKEUP_LATENCY_T);
	PRINT_PIVOT_ROW("Post Process", PCIE_POST_PROCESS_T);

	offset += sprintf(dx_buff+offset, "==================+");
	for(i=0; i<active_count; i++) offset += sprintf(dx_buff+offset, "==========+");
	offset += sprintf(dx_buff+offset, "\n");

	PRINT_PIVOT_ROW("Total", PCIE_TOTAL_TIME_T);

	// Bandwidth Row
	offset += sprintf(dx_buff+offset, " %-16s |", "Bandwidth");
	for (i = 0; i < active_count; i++) {
		dx_pcie_profiler_t *p = g_pcie_prof[active_devs[i]][active_dmas[i]][active_chs[i]];
		uint64_t bw = (p[PCIE_DMA_XFER_T].perf_avg_t > 0) ? PCIE_GET_BW(p[PCIE_DMA_XFER_T].size, p[PCIE_DMA_XFER_T].perf_avg_t) : 0;
		offset += sprintf(dx_buff+offset, " %4llu MB/s|", bw);
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
	/* If the size is changed then clear the profiler datas only for data bandwidth */
	// if (p->size == 0) p->size = size;
	// if ( (type == PCIE_TOTAL_TIME_T) && (p->size != size) ) {
	// 	clear_pcie_profile_info(1, type, dev_n, dma_n, ch_n);
	// }
	p->in_use = 1;
	get_start_time(&p->pref_t);
}

inline void dx_pcie_end_profile(int type, uint64_t size, int dev_n, int dma_n, int ch_n)
{
	dx_pcie_profiler_t *p;
	uint64_t elapsed_t;

	if (!READ_ONCE(g_perf_enabled))
		return;

	p = &g_pcie_prof[dev_n][dma_n][ch_n][type];
	elapsed_t = get_elapsed_time_ns(p->pref_t);

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
}
#else
#error "DMA_PERF_MEASURE must be defined — set unconditionally in Kbuild"
#endif /*DMA_PERF_MEASURE*/

static int dx_pci_rebar_find_pos(struct pci_dev *pdev, int bar)
{
	unsigned int pos, nbars, i;
	u32 ctrl;

	pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_REBAR);
	if (!pos)
		return -ENOTSUPP;

	pci_read_config_dword(pdev, pos + PCI_REBAR_CTRL, &ctrl);
	nbars = (ctrl & PCI_REBAR_CTRL_NBAR_MASK) >>
		    PCI_REBAR_CTRL_NBAR_SHIFT;

	for (i = 0; i < nbars; i++, pos += 8) {
		int bar_idx;

		pci_read_config_dword(pdev, pos + PCI_REBAR_CTRL, &ctrl);
		bar_idx = ctrl & PCI_REBAR_CTRL_BAR_IDX;
		if (bar_idx == bar)
			return pos;
	}

	return -ENOENT;
}

int dx_pci_rebar_get_current_size(struct pci_dev *pdev, int bar)
{
	int pos;
	u32 ctrl;

	pos = dx_pci_rebar_find_pos(pdev, bar);
	if (pos < 0)
		return pos;

	pci_read_config_dword(pdev, pos + PCI_REBAR_CTRL, &ctrl);
	return (ctrl & PCI_REBAR_CTRL_BAR_SIZE) >> PCI_REBAR_CTRL_BAR_SHIFT;
}

u64 dx_pci_rebar_size_to_bytes(int size)
{
	return 1ULL << (size + 20);
}

static LIST_HEAD(dx_dev_list);
static DEFINE_SPINLOCK(dx_dev_lock);
static DEFINE_IDA(dx_dev_ida);

static LIST_HEAD(dx_dev_rcu_list);
// static DEFINE_SPINLOCK(dx_dev_rcu_lock);

#ifndef list_last_entry
#define list_last_entry(ptr, type, member) list_entry((ptr)->prev, type, member)
#endif

int dx_dev_list_add(struct dw_edma *dw)
{
	unsigned long flags;
	int id;

	id = ida_alloc_max(&dx_dev_ida, MAX_DEV_NUM - 1, GFP_KERNEL);
	if (id < 0) {
		pr_err("[ERR] idx allocation failed (max=%d)\n", MAX_DEV_NUM);
		return id;
	}

	spin_lock_irqsave(&dx_dev_lock, flags);
	dw->idx = id;
	list_add_tail(&dw->list_head, &dx_dev_list);
	spin_unlock_irqrestore(&dx_dev_lock, flags);

	dbg_init("deepx dma idx %d.\n", dw->idx);

	return 0;
}
static int dx_dev_get_list_size(void)
{
	struct list_head *ptr;
	int count = 0;

	list_for_each(ptr, &dx_dev_list)
		count++;
	return count;
}
#undef list_last_entry

struct dw_edma *dx_dev_list_get(int dev_id)
{
	struct list_head *ptr;
	struct dw_edma *dw = NULL;
	unsigned long flags;

	if (dev_id < 0 || dev_id >= MAX_DEV_NUM) {
		pr_err("[ERR] dev_id %d out of range [0, %d)\n",
		       dev_id, MAX_DEV_NUM);
		return NULL;
	}

	spin_lock_irqsave(&dx_dev_lock, flags);
	list_for_each(ptr, &dx_dev_list) {
		struct dw_edma *ptr_node = list_entry(ptr, struct dw_edma, list_head);
		if (ptr_node->idx == dev_id) {
			dw = ptr_node;
			break;
		}
	}
	spin_unlock_irqrestore(&dx_dev_lock, flags);

	return dw;
}
EXPORT_SYMBOL_GPL(dx_dev_list_get);

void dx_dev_list_remove(struct dw_edma *dw)
{
	unsigned long flags;

	spin_lock_irqsave(&dx_dev_lock, flags);
	list_del(&dw->list_head);
	spin_unlock_irqrestore(&dx_dev_lock, flags);

	ida_free(&dx_dev_ida, dw->idx);
}

uint32_t dx_pcie_get_dev_num(void)
{
	unsigned long flags;
	int size;

	spin_lock_irqsave(&dx_dev_lock, flags);
	size = dx_dev_get_list_size();
	spin_unlock_irqrestore(&dx_dev_lock, flags);
	return size;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_dev_num);

u64 dx_pcie_get_download_region(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	return dw->download_region;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_download_region);

u32 dx_pcie_get_download_size(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	return dw->download_size;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_download_size);

/* will be modified in future(region datas is received from device) */
u64 dx_pcie_get_booting_region(int dev_id, int id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	return dw->booting_region[id];
}
EXPORT_SYMBOL_GPL(dx_pcie_get_booting_region);

/*
 * dx_pcie_test_and_clear_init_completed - Atomically test and clear init_completed flag
 * @dev_id: Device id
 * Returns: true if init was needed (flag was true), false otherwise
 *
 * This function atomically tests the init_completed flag and clears it if set.
 * Returns the previous value to prevent race conditions when multiple threads
 * try to initialize simultaneously. Only the first caller will get true.
 */
bool dx_pcie_test_and_clear_init_completed(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	unsigned long flags;
	bool was_set;

	if (!dw)
		return false;

	raw_spin_lock_irqsave(&dw->lock, flags);
	was_set = dw->init_completed;
	dw->init_completed = false;
	raw_spin_unlock_irqrestore(&dw->lock, flags);

	return was_set;
}
EXPORT_SYMBOL_GPL(dx_pcie_test_and_clear_init_completed);

/*
 * dx_pcie_set_init_completed - Set init_completed flag to trigger re-init
 * @dev_id: Device id
 *
 * Sets init_completed = true so that the next ioctl call will invoke
 * dxrt_device_init() to refresh device pointers (msg, dl, queues).
 * Called after PCIe SBR where FW was fully reset.
 */
void dx_pcie_set_init_completed(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	unsigned long flags;

	if (!dw)
		return;

	raw_spin_lock_irqsave(&dw->lock, flags);
	dw->init_completed = true;
	raw_spin_unlock_irqrestore(&dw->lock, flags);
}
EXPORT_SYMBOL_GPL(dx_pcie_set_init_completed);

/*
 * dx_pci_find_vsec_capability - Find a vendor-specific extended capability
 * @dev: PCI device to query
 * @vendor: Vendor ID for which capability is defined
 * @cap: Vendor-specific capability ID
 *
 * If @dev has Vendor ID @vendor, search for a VSEC capability with
 * VSEC ID @cap. If found, return the capability offset in
 * config space; otherwise return 0.
 */
u16 dx_pci_find_vsec_capability(struct pci_dev *dev, u16 vendor, int cap)
{
	u16 vsec = 0;
	u32 header;

	if (vendor != dev->vendor)
		return 0;

	while ((vsec = pci_find_next_ext_capability(dev, vsec,
						     PCI_EXT_CAP_ID_VNDR))) {
		if (pci_read_config_dword(dev, vsec + PCI_VNDR_HEADER,
					  &header) == PCIBIOS_SUCCESSFUL &&
		    PCI_VNDR_HEADER_ID(header) == cap)
			return vsec;
	}

	return 0;
}

/*
 * dx_pci_read_revision_id - Read the revision ID from the configuration space (offset 0x08)
 * @dev: PCI device
 * 
 * Return :
 *      0 : PASS
 */
int dx_pci_read_revision_id(struct pci_dev *dev, u8 *revision_id)
{
    int ret;
    ret = pci_read_config_byte(dev, PCI_REVISION_ID, revision_id);
    if (ret) {
        pr_err("Failed to read Revision ID\n");
        return ret;
    }
    pr_debug("PCIe Device Revision ID: 0x%02x\n", *revision_id);
    return 0;
}

/*
 * dx_pci_read_revision_id - Read the program if from the configuration space (offset 0x08)
 * @dev: PCI device
 * 
 * Return :
 *      0 : PASS
 */
int dx_pci_read_program_if(struct pci_dev *dev, u8 *prog_if)
{
    int ret;
    ret = pci_read_config_byte(dev, PCI_CLASS_PROG, prog_if);
    if (ret) {
        pr_err("Failed to read Revision ID\n");
        return ret;
    }
    pr_debug("PCIe Device Program IF: 0x%02x\n", *prog_if);
    return 0;
}

/*
 * dx_pci_read_msi_data - Read the msi data
 * @dev: PCI device
 * 
 * Return : msi data
 */
u16 dx_pci_read_msi_data(struct pci_dev *pdev)
{
	int pos;
	u16 control;
	u16 msi_data;
	u32 addr_low, addr_high;

	pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
	if (!pos) {
		pr_err("MSI capability not found\n");
		return 0;
	}

	pci_read_config_word(pdev, pos + PCI_MSI_FLAGS, &control);
	pci_read_config_dword(pdev, pos + PCI_MSI_ADDRESS_LO, &addr_low);
	if (control & PCI_MSI_FLAGS_64BIT) {
		pci_read_config_dword(pdev, pos + PCI_MSI_ADDRESS_HI, &addr_high);
		pci_read_config_word(pdev, pos + PCI_MSI_DATA_64, &msi_data);
	} else {
		addr_high = 0;
		pci_read_config_word(pdev, pos + PCI_MSI_DATA_32, &msi_data);
	}
	pr_debug("MSI Address: 0x%llx, Data: 0x%x\n",
			((u64)addr_high << 32) | addr_low, msi_data);

	return msi_data;
}

/*
 * dx_pci_read_msi_msg - Read complete MSI info from PCI config space
 * @pdev: PCI device
 * @msg: struct msi_msg to fill
 * 
 * Return: 0 on success, -1 on failure
 */
int dx_pci_read_msi_msg(struct pci_dev *pdev, struct msi_msg *msg)
{
	int pos;
	u16 control;

	if (!msg)
		return -1;

	pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
	if (!pos) {
		pr_err("MSI capability not found\n");
		return -1;
	}

	pci_read_config_word(pdev, pos + PCI_MSI_FLAGS, &control);
	pci_read_config_dword(pdev, pos + PCI_MSI_ADDRESS_LO, &msg->address_lo);
	if (control & PCI_MSI_FLAGS_64BIT) {
		pci_read_config_dword(pdev, pos + PCI_MSI_ADDRESS_HI, &msg->address_hi);
		pci_read_config_word(pdev, pos + PCI_MSI_DATA_64, (u16 *)&msg->data);
	} else {
		msg->address_hi = 0;
		pci_read_config_word(pdev, pos + PCI_MSI_DATA_32, (u16 *)&msg->data);
	}

	pr_debug("MSI from PCI config: addr=0x%x_%x, data=0x%x\n",
		msg->address_hi, msg->address_lo, msg->data);

	return 0;
}

/*
 * dx_pci_write_msi_msg - Write MSI address/data to PCI config space
 * @pdev: PCI device
 * @msg: struct msi_msg containing values to write
 * 
 * This directly programs the MSI capability in PCI config space.
 * Use with caution as it bypasses kernel's MSI management.
 * 
 * Return: 0 on success, -1 on failure
 */
int dx_pci_write_msi_msg(struct pci_dev *pdev, struct msi_msg *msg)
{
	int pos;
	u16 control;

	if (!msg)
		return -1;

	pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
	if (!pos) {
		pr_err("MSI capability not found\n");
		return -1;
	}

	pci_read_config_word(pdev, pos + PCI_MSI_FLAGS, &control);
	pci_write_config_dword(pdev, pos + PCI_MSI_ADDRESS_LO, msg->address_lo);
	if (control & PCI_MSI_FLAGS_64BIT) {
		pci_write_config_dword(pdev, pos + PCI_MSI_ADDRESS_HI, msg->address_hi);
		pci_write_config_word(pdev, pos + PCI_MSI_DATA_64, msg->data);
	} else {
		pci_write_config_word(pdev, pos + PCI_MSI_DATA_32, msg->data);
	}

	pr_info("dx_dma: Wrote MSI to PCI config: addr=0x%x_%x, data=0x%x\n",
		msg->address_hi, msg->address_lo, msg->data);

	return 0;
}
