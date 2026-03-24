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

#define KVM_KERNEL_MAJ 4
#define KVM_KERNEL_MIN 12
#define KVM_KERNEL_PAT 0

#define PCIE_GET_BW(size, time)     ((size*1000*1000*1000)/(time*1024*1024))

typedef enum dx_pcie_perf_t {
    /* 1. SW Preparation */
    PCIE_SG_ALLOC_T                 = 0, /* [KERN] SG Table Allocation */
    PCIE_USER_MAP_T                 ,    /* [U->K] User Page Pinning (get_user_pages) */
    PCIE_DMA_MAP_T                  ,    /* [KERN] IOMMU Mapping (dma_map_sg) */
    PCIE_DMA_PREP_T                 ,    /* [KERN] DMA Prep (prep_slave_sg) */
    PCIE_DESC_GEN_T                 ,    /* [KERN] Descriptor Generation (write_chunk) */

    /* 2. HW Execution */
    PCIE_DMA_XFER_T                 ,    /* [HW]   Pure DMA Transfer (Doorbell ~ ISR) */

    /* 3. Completion */
    PCIE_ISR_EXEC_T                 ,    /* [ISR]  ISR Execution Time */
    PCIE_WAKEUP_LATENCY_T           ,    /* [K->U] Wakeup Latency (Callback ~ Thread Resume) */
    PCIE_POST_PROCESS_T             ,    /* [KERN] Post Processing (Unmap, etc) */

    /* 4. Total Scopes */
    PCIE_KERNEL_DMA_TOTAL_T         ,    /* [ALL]  dw_edma_sg_process Total */
    PCIE_TOTAL_TIME_T               ,    /* [ALL]  ioctl Total */

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
extern u16 dx_pci_read_msi_data(struct pci_dev *pdev);
extern int dx_pci_read_msi_msg(struct pci_dev *pdev, struct msi_msg *msg);
extern int dx_pci_write_msi_msg(struct pci_dev *pdev, struct msi_msg *msg);

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
