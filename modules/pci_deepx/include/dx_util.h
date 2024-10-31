// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef _DX_UTIL_H
#define _DX_UTIL_H

#include <linux/ktime.h>
#include "dw-edma-core.h"

#define PCIE_GET_BW(size, time)     ((size*1000*1000*1000)/(time*1024*1024))

typedef enum dx_pcie_perf_t {
    PCIE_DESC_SEND_T                = 0, /* description table sending time to device using PCIe*/
    PCIE_SG_TABLE_ALLOC_T           ,
    PCIE_USER_PG_TO_PHY_MAPPING_T   ,
    PCIE_KERNEL_EXEC_T              , /* the total execution time on kernel space */
    PCIE_THREAD_RUN_T               , /* dma thread running time */
    PCIE_INT_CB_CALL_T              , /* interrupt ~ callback */
    PCIE_CB_TO_WAKE_T               , /* callback ~ dma thread wake-up */
    PCIE_DATA_BW_T                  , /* PCIe bandwidth (doorbell ~ interrupt) */
    PCIE_PERF_MAX_T                 ,
} dx_pcie_perf_t;
typedef struct dx_pcie_profiler_t {
    uint8_t     in_use;
    uint64_t    perf_max_t;
    uint64_t    perf_min_t;
    uint64_t    perf_avg_t;
    uint64_t    perf_sum_t;
    ktime_t     pref_t;
    uint64_t    count;
    uint64_t    size;
} dx_pcie_profiler_t;

/* PCIE NUM / DMA CHANNEL NUM / READ/WRITE Channel*/
// extern dx_pcie_profiler_t g_pcie_prof[16][4][2][PCIE_PERF_MAX_T];

extern struct dw_edma *dx_dev_list_get(int dev_id);

extern char *show_pcie_profile(void);
extern void clear_pcie_profile_info(int partial, int type_n, int dev_n, int dma_n, int ch_n);
#if defined(DMA_PERF_MEASURE)
extern void dx_pcie_start_profile(int type, uint64_t size, int dev_n, int dma_n, int ch_n);
extern void dx_pcie_end_profile(int type, uint64_t size, int dev_n, int dma_n, int ch_n);
extern void get_start_time(ktime_t *s);
extern uint64_t get_elapsed_time_ns(ktime_t s);
#else
extern void dx_pcie_start_profile(int type, uint64_t size, int dev_n, int dma_n, int ch_n);
extern void dx_pcie_end_profile(int type, uint64_t size, int dev_n, int dma_n, int ch_n);
#endif

extern int dx_pci_rebar_get_current_size(struct pci_dev *pdev, int bar);
extern u64 dx_pci_rebar_size_to_bytes(int size);

extern int dx_dev_list_add(struct dw_edma *dw);
extern void dx_dev_list_remove(struct dw_edma *xdev);

extern u16 dx_pci_find_vsec_capability(struct pci_dev *dev, u16 vendor, int cap);
extern int dx_pci_read_revision_id(struct pci_dev *dev, u8 *revision_id);
extern int dx_pci_read_program_if(struct pci_dev *dev, u8 *prog_if);

#ifndef DEFINE_SHOW_ATTRIBUTE
#define DEFINE_SHOW_ATTRIBUTE(__name)                   \
static int __name ## _open(struct inode *inode, struct file *file)  \
{                                   \
    return single_open(file, __name ## _show, inode->i_private);    \
}                                   \
                                    \
static const struct file_operations __name ## _fops = {         \
    .owner      = THIS_MODULE,                  \
    .open       = __name ## _open,              \
    .read       = seq_read,                 \
    .llseek     = seq_lseek,                    \
    .release    = single_release,               \
}
#endif
#endif
