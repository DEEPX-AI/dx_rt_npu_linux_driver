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

#include "dx_util.h"
#include "dx_lib.h"
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

void get_start_time(ktime_t *s)
{
	*s = ktime_get();
}

uint64_t get_elapsed_time_ns(ktime_t s)
{
	ktime_t e = ktime_get();
	return ktime_to_ns(ktime_sub(e, s));
}

const char* get_pcie_type_string(int type)
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

const char* get_pcie_ctx_string(int type)
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
	dx_pcie_profiler_t *p = &g_pcie_prof[dev_n][dma_n][ch_n][type];
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
	dx_pcie_profiler_t *p = &g_pcie_prof[dev_n][dma_n][ch_n][type];
	uint64_t elapsed_t = get_elapsed_time_ns(p->pref_t);

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
char *show_pcie_profile(void) { return NULL; }
void clear_pcie_profile_info(int partial, int type_n, int dev_n, int dma_n, int ch_n) { /*nothing*/ }
void dx_pcie_start_profile(int type, uint64_t size, int dev_n, int dma_n, int ch_n) { /*nothing*/ }
void dx_pcie_end_profile(int type, uint64_t size, int dev_n, int dma_n, int ch_n) { /*nothing*/ }
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

static LIST_HEAD(dx_dev_rcu_list);
// static DEFINE_SPINLOCK(dx_dev_rcu_lock);

#ifndef list_last_entry
#define list_last_entry(ptr, type, member) list_entry((ptr)->prev, type, member)
#endif

int dx_dev_list_add(struct dw_edma *dw)
{
	unsigned long flags;

	spin_lock_irqsave(&dx_dev_lock, flags);
	if (list_empty(&dx_dev_list)) {
		dw->idx = 0;
	} else {
		struct dw_edma *last;
		last = list_last_entry(&dx_dev_list, struct dw_edma, list_head);
		dw->idx = last->idx + 1;
	}
	list_add_tail(&dw->list_head, &dx_dev_list);
	spin_unlock_irqrestore(&dx_dev_lock, flags);

	dbg_init("deepx dma idx %d.\n", dw->idx);

	// spin_lock(&xdev_rcu_lock);
	// list_add_tail_rcu(&xdev->rcu_node, &xdev_rcu_list);
	// spin_unlock(&xdev_rcu_lock);

	return 0;
}
static int dx_dev_get_list_size(void)
{
	struct dw_edma *last;
	if (!list_empty(&dx_dev_list)) {
		last = list_last_entry(&dx_dev_list, struct dw_edma, list_head);
		return last->idx;
	} else {
		return -1;
	}
}
#undef list_last_entry

struct dw_edma *dx_dev_list_get(int dev_id)
{
	struct list_head *ptr;
	int size;
	struct dw_edma *dw = NULL;
	unsigned long flags;

	spin_lock_irqsave(&dx_dev_lock, flags);
	size = dx_dev_get_list_size();
	if (list_empty(&dx_dev_list))
		pr_err("[ERR] list is empty");
	else if (dev_id > size)
		pr_err("[ERR] dev_id is over than the size of list(%d/%d)", dev_id, size);

	list_for_each(ptr, &dx_dev_list){
		struct dw_edma *ptr_node = list_entry(ptr, struct dw_edma, list_head);
		if (ptr_node->idx == dev_id) {
			// printk("Deepx Pcie struct id number : %d\n", ptr_node->idx);
			dw = ptr_node;
			break;
		}
	}
	if (dw == NULL)
		pr_err("[ERR] not found deepx pcie struct");
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

	// spin_lock(&xdev_rcu_lock);
	// list_del_rcu(&xdev->rcu_node);
	// spin_unlock(&xdev_rcu_lock);
	// synchronize_rcu();
}

uint32_t dx_pcie_get_dev_num(void)
{
	return dx_dev_get_list_size() + 1;
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

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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