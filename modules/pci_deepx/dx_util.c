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
		case PCIE_DESC_SEND_T:
			type_str = "PCIE_DESC_SEND_T";
			break;
		case PCIE_SG_TABLE_ALLOC_T:
			type_str = "PCIE_SG_TABLE_ALLOC_T";
			break;
		case PCIE_USER_PG_TO_PHY_MAPPING_T:
			type_str = "PCIE_USER_PG_TO_PHY_MAPPING_T";
			break;
		case PCIE_KERNEL_EXEC_T:
			type_str = "PCIE_KERNEL_EXEC_T";
			break;
		case PCIE_THREAD_RUN_T:
			type_str = "PCIE_THREAD_RUN_T";
			break;
		case PCIE_DATA_BW_T:
			type_str = "PCIE_DATA_BW_T";
			break;
		case PCIE_INT_CB_CALL_T:
			type_str = "PCIE_INT_CB_CALL_T";
			break;
		case PCIE_CB_TO_WAKE_T:
			type_str = "PCIE_CB_TO_WAKE_T";
			break;
		case PCIE_PERF_MAX_T:
			type_str = "PCIE_PERF_MAX_T";
			break;
		default:
			type_str = "unknown";
			break;
	}
	return type_str;
}

#define DX_DBG_PRT_MAX_BUFFER_SIZE	40960
#define DX_BUF_SIZE_CHECKER(offset) do {\
	if (offset > DX_DBG_PRT_MAX_BUFFER_SIZE*0.9) goto show_exit;\
} while(0);
static char dx_buff[DX_DBG_PRT_MAX_BUFFER_SIZE];
char *show_pcie_profile(void)
{
	int dev = 0, dma = 0, ch = 0, type = 0;
	uint64_t size;
	int offset = 0;
	char *ret;

	memset(dx_buff, 0x00, sizeof(dx_buff));
	for (dev = 0; dev < 16; dev++) {
		for (dma = 0; dma < 4; dma++) {
			for (ch = 0; ch < 2; ch++) {
				if (g_pcie_prof[dev][dma][ch][PCIE_DATA_BW_T].in_use == 0) {
					continue;
				}
				// DX_BUF_SIZE_CHECKER(offset);
				offset += sprintf(dx_buff+offset, "DEV:DMA:CH (%d / %d / %d)(%s) - size:%llu\n",
					dev, dma, ch,
					ch==0 ? "W":"R",
					g_pcie_prof[dev][dma][ch][PCIE_DATA_BW_T].size);
				for (type = 0; type < PCIE_PERF_MAX_T; type++) {
						// DX_BUF_SIZE_CHECKER(offset);
						offset += sprintf(dx_buff+offset, "%-30s(%20lluns/%20lluns/%20lluns), count:%llu\n",
							get_pcie_type_string(type),
							g_pcie_prof[dev][dma][ch][type].perf_min_t,
							g_pcie_prof[dev][dma][ch][type].perf_avg_t,
							g_pcie_prof[dev][dma][ch][type].perf_max_t,
							g_pcie_prof[dev][dma][ch][type].count);
						if (type == PCIE_DATA_BW_T) {
							size = g_pcie_prof[dev][dma][ch][type].size;
							// DX_BUF_SIZE_CHECKER(offset);
							offset += sprintf(dx_buff+offset, "%-30s(%19lluMHz/%19lluMHz/%19lluMHz), count:%llu\n",
								get_pcie_type_string(type),
								PCIE_GET_BW(size, g_pcie_prof[dev][dma][ch][type].perf_max_t),
								PCIE_GET_BW(size, g_pcie_prof[dev][dma][ch][type].perf_avg_t),
								PCIE_GET_BW(size, g_pcie_prof[dev][dma][ch][type].perf_min_t),
								g_pcie_prof[dev][dma][ch][type].count);
						}
				}
				offset += sprintf(dx_buff+offset, "-------------------\n");
			}
		}
	}
// show_exit:
	if (offset > DX_DBG_PRT_MAX_BUFFER_SIZE) {
		pr_err("Please check buffer size (%d/%d)\n", offset, DX_DBG_PRT_MAX_BUFFER_SIZE);
		ret = NULL;
	} else {
		// pr_err("%s", dx_buff);
		ret = dx_buff;
	}
	return ret;
}

/* part : [0 - all, 1 - partial] */
void clear_pcie_profile_info(int partial, int type_n, int dev_n, int dma_n, int ch_n)
{
	int dev, dma, ch, type;

	if (partial) {
		memset(&g_pcie_prof[dev_n][dma_n][ch_n][0], 0x00, sizeof(dx_pcie_profiler_t)*PCIE_PERF_MAX_T);
		for (type = 0; type < PCIE_PERF_MAX_T; type++) {
			g_pcie_prof[dev_n][dma_n][ch_n][type].perf_min_t = 0xFFFFFFFF;
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
	if ( (type == PCIE_KERNEL_EXEC_T) && (p->size != size) ) {
		clear_pcie_profile_info(1, type, dev_n, dma_n, ch_n);
	}
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

bool dx_pcie_get_init_completed(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	return dw->init_completed;
}
EXPORT_SYMBOL_GPL(dx_pcie_get_init_completed);

void dx_pcie_set_init_completed(int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);
	dw->init_completed = false;
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