// SPDX-License-Identifier: GPL-2.0
/*
* Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
* DeepX eDMA PCIe driver
*
* Author: Taegyun An <atg@deepx.ai>
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/device.h>
#include <linux/msi.h>
#include <linux/irq.h>
#include "dx_bitfield_compat.h"	//DEEPX MODIFIED: 4.4 FIELD_* compat
#include <linux/aer.h>
#include <linux/delay.h>
#include "dx_edma.h"

#include "dw-edma-v0-core.h"
#include "dw-edma-core.h"
#include "dxrt_drv.h"
#include "dx_cdev.h"
#include "dx_util.h"
#include "dx_lib.h"
#include "dw-edma-thread.h"
#include "dx_dma_sysfs.h"
#include "dx_link_health.h"
#include "dx_message.h"
#include "version.h"

#ifdef RPI_DEBUG_BUILD
#include "rpi-gpio.h"
#endif

#ifdef RPI_BUILD
#define EP_IRQ_RPI_SHUTDOWN_OFFSET		    (0x18)
#endif

#ifdef DX_DEBUG_ENABLE /*DEEPX MODIFIED*/
	#ifdef pci_dbg
		#undef pci_dbg
		#define pci_dbg		pci_err
	#endif
#endif

#define DX_PCI_VENDOR_ID		0x1FF4
#define DX_PCI_LEGACY_DEVICE_ID		0x0
#define DX_M1_PCI_DEVICE_ID		0x0100
#define DX_M1M_PCI_DEVICE_ID		0x0110
#define DX_H1_PCI_DEVICE_ID		0x0101
#define DX_H1M_PCI_DEVICE_ID		0x0111
#define DX_PCI_SUB_VENDOR_ID		0x1FF4
#define DX_PCI_LEGACY_SUB_DEVICE_ID	0x0
#define DX_M1_PCI_SUB_DEVICE_ID		0x0100
#define DX_M1M_PCI_SUB_DEVICE_ID	0x0110
#define DX_H1_PCI_SUB_DEVICE_ID		0x0101
#define DX_H1M_PCI_SUB_DEVICE_ID	0x0111

#define USER_BAR_NUM					3

#define DW_PCIE_VSEC_DMA_ID				0x6
#define DW_PCIE_VSEC_DMA_BAR			GENMASK(10, 8)
#define DW_PCIE_VSEC_DMA_MAP			GENMASK(2, 0)
#define DW_PCIE_VSEC_DMA_WR_CH			GENMASK(9, 0)
#define DW_PCIE_VSEC_DMA_RD_CH			GENMASK(25, 16)

#define DW_BLOCK(a, b, c) \
	{ \
		.bar = a, \
		.off = b, \
		.sz = c, \
	},

#define DW_NPU_BLOCK(a, b, c, d) \
	{ \
		.bar = a, \
		.off = b, \
		.sz = c, \
		.ep_addr = d, \
	},

enum pci_barno {
	NO_BAR = -1,
	BAR_0,
	BAR_1,
	BAR_2,
	BAR_3,
	BAR_4,
	BAR_5,
};

enum pcie_if_mode_t {
    DX_PCIE_IF_MODE_0 = 0,
    DX_PCIE_IF_MODE_1 = 1,
    DX_PCIE_IF_MODE_2 = 2,
    DX_PCIE_IF_MODE_3 = 3,
};

struct dx_edma_block {
	enum pci_barno		bar;
	off_t				off;
	size_t				sz;
};

struct dx_ep_block {
	enum pci_barno		bar;
	off_t				off;
	size_t				sz;
	u64					ep_addr;
};

struct dw_edma_pcie_data {
	u16							version;
	u64							desc_addr; /* device phy address */
	/* eDMA registers location */
	struct dx_edma_block		rg;
	/* eDMA memory linked list location */
	struct dx_edma_block		ll_wr[EDMA_MAX_WR_CH];
	struct dx_edma_block		ll_rd[EDMA_MAX_RD_CH];
	/* Other */
	enum dx_edma_map_format		mf;
	u8				irqs;		/* Total irqs (included user irqs) */
	u8				dma_irqs;	/* dma irqs */
	u16				wr_ch_cnt;
	u16				rd_ch_cnt;
	/* iATU registers */
	struct dx_edma_block		iatu;
	/* User registers */
	u16							user_reg_cnt;
	struct dx_ep_block			users[USER_BAR_NUM];
	u64							download_region;
	u32							download_size;
	u64							booting_region[2];
};

/*  Total Size : BAR0_MEM_SIZE
-------------------------------------------------- << DESC_WR_BASE_OFFS
     DMA Write Descriptor  (Size : DESC_WR_RD_SIZE)
-------------------------------------------------- << DESC_RD_BASE_OFFS
     DMA Read Descriptor   (Size : DESC_WR_RD_SIZE)
-------------------------------------------------- */

#define BAR0_MEM_SIZE		(4*1024*1024) /* 4MB */
#define DESC_WR_BASE_OFFS	(0x00)
#define DESC_RD_BASE_OFFS	(0x200000)
#define DESC_WR_RD_SIZE		(0x100000)

#define BAR0_MEM_SIZE_M1		(8*1024*1024) /* 8MB */
#define DESC_WR_BASE_OFFS_M1	(0x00)
#ifdef SRAM_DESC_TABLE
	#undef DESC_WR_RD_SIZE
	#define DESC_WR_RD_SIZE			(0x1000)
	#define DESC_RD_BASE_OFFS_M1	(DESC_WR_RD_SIZE)
#else
	#define DESC_RD_BASE_OFFS_M1	(0x400000)
#endif /*SRAM_DESC_TABLE*/

/* DXNN V2 - m1 */
static const struct dw_edma_pcie_data dx_pcie_data_v3 = {
	.version			= 3,
#ifdef SRAM_DESC_TABLE
	.desc_addr			= 0xD3000000,
#else
	.desc_addr			= 0x640000000,
#endif
	/* eDMA registers location */
	.rg.bar				= BAR_2,
	.rg.off				= 0x00000000,	/*  0   Kbytes */
	.rg.sz				= 0x00000A00,	/*  2.5 Kbytes */
	/* eDMA memory linked list location */
#ifdef SRAM_DESC_TABLE
	.ll_wr = {
		DW_BLOCK(BAR_5, DESC_WR_BASE_OFFS_M1, 					DESC_WR_RD_SIZE)	/* Channel 0 */
		DW_BLOCK(BAR_5, DESC_WR_BASE_OFFS_M1+DESC_WR_RD_SIZE,	DESC_WR_RD_SIZE)	/* Channel 1 */
		DW_BLOCK(BAR_5, DESC_WR_BASE_OFFS_M1+DESC_WR_RD_SIZE*2,DESC_WR_RD_SIZE)	/* Channel 2 */
	},
	.ll_rd = {
		DW_BLOCK(BAR_5, DESC_RD_BASE_OFFS_M1,					DESC_WR_RD_SIZE)	/* Channel 0 */
		DW_BLOCK(BAR_5, DESC_RD_BASE_OFFS_M1+DESC_WR_RD_SIZE,	DESC_WR_RD_SIZE)	/* Channel 1 */
		DW_BLOCK(BAR_5, DESC_RD_BASE_OFFS_M1+DESC_WR_RD_SIZE*2,DESC_WR_RD_SIZE)	/* Channel 2 */
	},
#else
	.ll_wr = {
		DW_BLOCK(BAR_0, DESC_WR_BASE_OFFS_M1,					DESC_WR_RD_SIZE)	/* Channel 0 */
		DW_BLOCK(BAR_0, DESC_WR_BASE_OFFS_M1+DESC_WR_RD_SIZE,	DESC_WR_RD_SIZE)	/* Channel 1 */
		DW_BLOCK(BAR_0, DESC_WR_BASE_OFFS_M1+DESC_WR_RD_SIZE*2,DESC_WR_RD_SIZE)	/* Channel 2 */
		DW_BLOCK(BAR_0, DESC_WR_BASE_OFFS_M1+DESC_WR_RD_SIZE*3,DESC_WR_RD_SIZE)	/* Channel 3 */
	},
	.ll_rd = {
		DW_BLOCK(BAR_0, DESC_RD_BASE_OFFS_M1,					DESC_WR_RD_SIZE)	/* Channel 0 */
		DW_BLOCK(BAR_0, DESC_RD_BASE_OFFS_M1+DESC_WR_RD_SIZE,	DESC_WR_RD_SIZE)	/* Channel 1 */
		DW_BLOCK(BAR_0, DESC_RD_BASE_OFFS_M1+DESC_WR_RD_SIZE*2,DESC_WR_RD_SIZE)	/* Channel 2 */
		DW_BLOCK(BAR_0, DESC_RD_BASE_OFFS_M1+DESC_WR_RD_SIZE*3,DESC_WR_RD_SIZE)	/* Channel 3 */
	},
#endif
	/* Other */
	.mf					= DX_DMA_MF_HDMA_COMPAT,
	.dma_irqs			= 1,
	.wr_ch_cnt			= 4,
	.rd_ch_cnt			= 4,
	/* iATU registers */
	.iatu.bar			= BAR_2,
	.iatu.off			= 0x00000A00,
	.iatu.sz			= 0x00001000,
	/* user registers */
	.user_reg_cnt		= 3,
	.users = {
		DW_NPU_BLOCK(BAR_3, 0x00000000, 0x10000, 0xD3010000) /* MESSAGE RAM */
		DW_NPU_BLOCK(BAR_4, 0x00000000, 0x1000,  0xCC000000) /* Interface */
		DW_NPU_BLOCK(BAR_5, 0x00000000, 0x10000, 0xC2200000) /* Interface */
	},
	.download_region	= 0x63FF00000,
	.download_size		= 0x100000,
	.booting_region		= {0xD3000000, 0x600080000},
};

static void dx_pcie_set_pdata_by_rev(struct dw_edma_pcie_data *pdata, u8 rev, u8 prog)
{
	if (rev == 1) {
#ifndef SRAM_DESC_TABLE
		uint64_t high_addr;

		switch (prog) {
			case DX_PCIE_IF_MODE_0:
				high_addr = 6;
				break;
			case DX_PCIE_IF_MODE_1:
				high_addr = 4;
				break;
			case DX_PCIE_IF_MODE_2:
				high_addr = 3;
				break;
			default:
				high_addr = 1;
				break;
		}
		high_addr <<= 32;
		pdata->desc_addr       = (0x04000000 | high_addr);
		pdata->download_region = (0x03F00000 | high_addr);
#endif
		pdata->download_size   = 0x100000;
	}
}

static int dw_edma_pcie_irq_vector(struct device *dev, unsigned int nr)
{
	pci_dbg(to_pci_dev(dev), "[%s] nr:%d\n", __func__, nr);
	return pci_irq_vector(to_pci_dev(dev), nr);
}

static const struct dx_edma_core_ops dw_edma_pcie_core_ops = {
	.irq_vector = dw_edma_pcie_irq_vector,
};

static int dw_edma_pcie_mask_unused_msi_vectors(struct pci_dev *pdev,
						int used_irqs, int allocated_irqs)
{
	struct irq_data *irq_data;
	u32 expected_mask = 0;
	u32 mask;
	u16 control;
	int pos, mask_pos, max_irqs;
	int i, irq;

	if (allocated_irqs <= used_irqs)
		return 0;

	pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
	if (!pos) {
		pci_warn(pdev, "MSI capability not found for unused vector masking\n");
		return -ENODEV;
	}

	pci_read_config_word(pdev, pos + PCI_MSI_FLAGS, &control);
	if (!(control & PCI_MSI_FLAGS_MASKBIT)) {
		pci_warn(pdev, "MSI per-vector masking is not supported\n");
		return -EOPNOTSUPP;
	}
	max_irqs = 1 << FIELD_GET(PCI_MSI_FLAGS_QMASK, control);
	if (allocated_irqs > max_irqs)
		return -EINVAL;

	for (i = used_irqs; i < allocated_irqs; i++) {
		irq = pci_irq_vector(pdev, i);
		if (irq < 0)
			return irq;

		irq_data = irq_get_irq_data(irq);
		if (!irq_data)
			return -EINVAL;

		/*
		 * DEEPX MODIFIED: pci_msi_mask_irq() is EXPORT_SYMBOL_GPL from
		 * Linux 5.x onward.  On 4.4 it exists in msi.c but is not
		 * exported, so skip it there — the hardware MSI Mask Bits
		 * register is written in bulk by pci_write_config_dword() below.
		 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
		pci_msi_mask_irq(irq_data);
#endif
		expected_mask |= BIT(i);
	}
	for (i = allocated_irqs; i < max_irqs; i++)
		expected_mask |= BIT(i);

	mask_pos = (control & PCI_MSI_FLAGS_64BIT) ?
		PCI_MSI_MASK_64 : PCI_MSI_MASK_32;
	pci_read_config_dword(pdev, pos + mask_pos, &mask);
	mask |= expected_mask;
	pci_write_config_dword(pdev, pos + mask_pos, mask);
	pci_read_config_dword(pdev, pos + mask_pos, &mask);
	if ((mask & expected_mask) != expected_mask) {
		pci_warn(pdev,
			 "unused MSI vector mask readback failed (expected=0x%08x, current=0x%08x)\n",
			 expected_mask, mask);
		return -EIO;
	}

	pci_info(pdev, "masked unused MSI vectors [%d-%d] (mask=0x%08x)\n",
		 used_irqs, max_irqs - 1, mask);

	return 0;
}

static void dw_edma_pcie_get_vsec_dma_data(struct pci_dev *pdev,
					   struct dw_edma_pcie_data *pdata)
{
	u32 val, map;
	u16 vsec;
	u64 off;

	vsec = dx_pci_find_vsec_capability(pdev, DX_PCI_VENDOR_ID,
					DW_PCIE_VSEC_DMA_ID);
	if (!vsec)
		return;

	pci_read_config_dword(pdev, vsec + PCI_VNDR_HEADER, &val);
	if (PCI_VNDR_HEADER_REV(val) != 0x00 ||
	    PCI_VNDR_HEADER_LEN(val) != 0x18)
		return;

	pci_dbg(pdev, "Detected PCIe Vendor-Specific Extended Capability DMA\n");
	pci_read_config_dword(pdev, vsec + 0x8, &val);
	map = FIELD_GET(DW_PCIE_VSEC_DMA_MAP, val);
	if (map != DX_DMA_MF_EDMA_LEGACY &&
	    map != DX_DMA_MF_EDMA_UNROLL &&
	    map != DX_DMA_MF_HDMA_COMPAT)
		return;

	pdata->mf = map;
	pdata->rg.bar = FIELD_GET(DW_PCIE_VSEC_DMA_BAR, val);

	pci_read_config_dword(pdev, vsec + 0xc, &val);
	pdata->wr_ch_cnt = min_t(u16, pdata->wr_ch_cnt,
				 FIELD_GET(DW_PCIE_VSEC_DMA_WR_CH, val));
	pdata->rd_ch_cnt = min_t(u16, pdata->rd_ch_cnt,
				 FIELD_GET(DW_PCIE_VSEC_DMA_RD_CH, val));

	pci_read_config_dword(pdev, vsec + 0x14, &val);
	off = val;
	pci_read_config_dword(pdev, vsec + 0x10, &val);
	off <<= 32;
	off |= val;
	pdata->rg.off = off;

	/* Debug */
	pci_dbg(pdev, "pdata->mf        : 0x%x\n", pdata->mf);
	pci_dbg(pdev, "pdata->rg.bar    : 0x%x\n", pdata->rg.bar);
	pci_dbg(pdev, "pdata->wr_ch_cnt : 0x%x\n", pdata->wr_ch_cnt);
	pci_dbg(pdev, "pdata->rd_ch_cnt : 0x%x\n", pdata->rd_ch_cnt);
	pci_dbg(pdev, "pdata->rg.off    : 0x%lx\n", pdata->rg.off);
	pci_dbg(pdev, "Header Type      : 0x%x\n", pdev->hdr_type);
}

static int dx_dma_pcie_probe(struct pci_dev *pdev,
			      const struct pci_device_id *pid)
{
	struct dw_edma_pcie_data *pdata = (void *)pid->driver_data;
	struct dw_edma_pcie_data vsec_data;
	struct device *dev = &pdev->dev;
	struct dw_edma_chip *chip;
	struct dw_edma *dw;
	int err, nr_irqs;
	int i, mask, bar_size;
	int total_irqs, multi_irqs;
	u8 revision_id, prog_if;

	dbg_init("pdev : %p name[%s].\n", pdev, pci_name(pdev));

	/* Enable PCI device */
	err = pcim_enable_device(pdev);
	if (err) {
		pci_err(pdev, "enabling device failed\n");
		return err;
	}

	memcpy(&vsec_data, pdata, sizeof(struct dw_edma_pcie_data));

	/*
	 * Tries to find if exists a PCIe Vendor-Specific Extended Capability
	 * for the DMA, if one exists, then reconfigures it.
	 */
	dw_edma_pcie_get_vsec_dma_data(pdev, &vsec_data);
	/* Mapping PCI BAR regions */
	mask = BIT(vsec_data.rg.bar);
	for (i = 0; i < vsec_data.user_reg_cnt; i++) {
		mask |= BIT(vsec_data.users[i].bar);
	}
	for (i = 0; i < vsec_data.wr_ch_cnt; i++) {
		mask |= BIT(vsec_data.ll_wr[i].bar);
	}
	for (i = 0; i < vsec_data.rd_ch_cnt; i++) {
		mask |= BIT(vsec_data.ll_rd[i].bar);
	}
	err = pcim_iomap_regions(pdev, mask, pci_name(pdev));
	if (err) {
		pci_err(pdev, "eDMA BAR I/O remapping failed\n");
		return err;
	}

	pci_set_master(pdev);

	if ((dx_pci_read_revision_id(pdev, &revision_id) != 0) ||
		(dx_pci_read_program_if(pdev, &prog_if) != 0)) {
		return -ENODEV;
	}
	dx_pcie_set_pdata_by_rev(&vsec_data, revision_id, prog_if);

	/* DMA configuration */
	err = dma_set_mask(&pdev->dev, DMA_BIT_MASK(64));
	if (!err) {
		err = dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(64));
		if (err) {
			pci_err(pdev, "consistent DMA mask 64 set failed\n");
			return err;
		}
	} else {
		pci_err(pdev, "DMA mask 64 set failed\n");

		err = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
		if (err) {
			pci_err(pdev, "DMA mask 32 set failed\n");
			return err;
		}

		err = dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(32));
		if (err) {
			pci_err(pdev, "consistent DMA mask 32 set failed\n");
			return err;
		}
	}

	/* Data structure allocation */
	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	dw = devm_kzalloc(dev, sizeof(*dw), GFP_KERNEL);
	if (!dw)
		return -ENOMEM;

	/* Set number of IRQS */
	dw->dx_ver = pdata->version;
	pci_err(pdev, "dw->dx_ver: %d\n", dw->dx_ver);
	set_user_irq_vec_table(dw);
	total_irqs = vsec_data.dma_irqs + get_nr_user_irqs(dw);

	/* IRQs allocation */
	pci_dbg(pdev, "Total IRQ number with including npu handler: %d\n", total_irqs);
#ifdef RPI_BUILD
    /* BCM2712 (RPi CM5) brcmstb MSI controller allocates MSI vectors with
     * unaligned base data (e.g. data=0xc with MME=3 gives base=8, not 0xc).
     * The EP RTL generates data=(base & ~mask)|vector per PCI spec, but the
     * host expects data=base+vector — mismatch silently drops NPU done MSIs.
     * Force single MSI: all events muxed via SRAM SW IRQ block (dx_sw_irq). */
    nr_irqs = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_MSI);
    pci_info(pdev, "RPi: forcing single MSI mode (brcmstb multi-MSI data misalignment)\n");
#else
	if (total_irqs > 1) {
		multi_irqs = 1;
		while (multi_irqs < total_irqs)
			multi_irqs <<= 1;

		nr_irqs = pci_alloc_irq_vectors(pdev, multi_irqs, multi_irqs,
					       PCI_IRQ_MSI);
		if (nr_irqs == multi_irqs) {
			err = dw_edma_pcie_mask_unused_msi_vectors(pdev,
								       total_irqs, nr_irqs);
			if (err) {
				pci_warn(pdev,
					 "M-MSI vector-mask Failed (err=%d), fallback to S-MSI\n",
					 err);
				pci_free_irq_vectors(pdev);
				nr_irqs = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_MSI);
			} else {
				pci_info(pdev, "M-MSI allocation Success (%dV allocated, %d required, base IRQ %d)\n",
					 nr_irqs, total_irqs, pci_irq_vector(pdev, 0));
			}
		} else {
			if (nr_irqs > 0) {
				pci_warn(pdev,
					 "M-MSI allocation Partial (%d/%dV, %d required), fallback to S-MSI\n",
					 nr_irqs, multi_irqs, total_irqs);
				pci_free_irq_vectors(pdev);
			} else {
				pci_warn(pdev,
					 "M-MSI allocation Failed (%dV, %d required, err=%d), fallback to S-MSI\n",
					 multi_irqs, total_irqs, nr_irqs);
			}
			nr_irqs = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_MSI);
		}
	} else {
		nr_irqs = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_MSI);
	}
#endif
	if (nr_irqs < 1) {
		pci_err(pdev, "S-MSI allocation Failed (1V, err=%d)\n", nr_irqs);
		return -EPERM;
	} else if (nr_irqs == 1) {
		pci_info(pdev, "S-MSI allocation Success (1V, IRQ %d)\n", pci_irq_vector(pdev, 0));
	}
	dw->event_irq_idx = get_nr_user_irqs(dw) - 1;
	pci_dbg(pdev, "Error irq index: %d\n", dw->event_irq_idx);

	/* Check BAR0 size */
	bar_size = dx_pci_rebar_get_current_size(pdev, 0);
	if (bar_size != -ENOTSUPP) { /* skip check logic */
		bar_size = dx_pci_rebar_size_to_bytes(bar_size);
		if (vsec_data.version == 2) {
			if (bar_size !=  BAR0_MEM_SIZE) {
				pci_err(pdev, "size of a BAR is not matched(%d)\n", bar_size);
				err = -ENOMEM;
				goto err_irq_vectors;
			}
		} else if (vsec_data.version == 3) {
			if (bar_size !=  BAR0_MEM_SIZE_M1) {
				pci_err(pdev, "size of a BAR is not matched(%d)\n", bar_size);
				err = -ENOMEM;
				goto err_irq_vectors;
			}
		}
	}

	/* Data structure initialization */
	chip->dw = dw;
	chip->dev = dev;

	dw->mf = vsec_data.mf;
	dw->nr_irqs = nr_irqs;
	dw->dma_irqs = vsec_data.dma_irqs;
	dw->ops = &dw_edma_pcie_core_ops;
	dw->wr_ch_cnt = vsec_data.wr_ch_cnt;
	dw->rd_ch_cnt = vsec_data.rd_ch_cnt;

	dw->rg_region.vaddr = pcim_iomap_table(pdev)[vsec_data.rg.bar];
	if (!dw->rg_region.vaddr) {
		err = -ENOMEM;
		goto err_irq_vectors;
	}

	dw->rg_region.vaddr += vsec_data.rg.off;
	dw->rg_region.paddr = pdev->resource[vsec_data.rg.bar].start;
	dw->rg_region.paddr += vsec_data.rg.off;
	dw->rg_region.sz = vsec_data.rg.sz;

	/* iATU */
	dw->dma_desc_base_addr = vsec_data.desc_addr;
	dw->dma_desc_base_bar  = pdev->resource[vsec_data.ll_wr[0].bar].start;
	dw->dma_desc_size      = bar_size;
	dw->dma_desc_bar_num   = vsec_data.ll_wr[0].bar;
	{
		struct dx_edma_region *iatu = &dw->iatu_region;
		iatu->vaddr = pcim_iomap_table(pdev)[vsec_data.iatu.bar];
		if (!iatu->vaddr) {
			pci_err(pdev, "iATU BAR#%d mapping fail!\n", vsec_data.iatu.bar);
			err = -ENOMEM;
			goto err_irq_vectors;
		}
		iatu->vaddr += vsec_data.iatu.off;
		iatu->paddr = pdev->resource[vsec_data.iatu.bar].start;
		iatu->paddr += vsec_data.iatu.off;
		iatu->sz = vsec_data.iatu.sz;
	}

	/* USER */
	dw->user_bar_cnt = vsec_data.user_reg_cnt;
	for (i = 0; i < dw->user_bar_cnt; i++) {
		struct dx_user_region *npu = &dw->npu_region[i];
		npu->vaddr = pcim_iomap_table(pdev)[vsec_data.users[i].bar];
		if (!npu->vaddr) {
			pci_err(pdev, "USER BAR#%d mapping Fail!\n", vsec_data.users[i].bar);
			err = -ENOMEM;
			goto err_irq_vectors;
		}
		npu->vaddr	+=	vsec_data.users[i].off;
		npu->paddr	=	pdev->resource[vsec_data.users[i].bar].start;
		npu->paddr	+=	vsec_data.users[i].off;
		npu->sz		=	vsec_data.users[i].sz;
		npu->bar_num=	vsec_data.users[i].bar;
		npu->ep_addr=	vsec_data.users[i].ep_addr;
	}

	for (i = 0; i < dw->wr_ch_cnt; i++) {
		struct dx_edma_region *ll_region = &dw->ll_region_wr[i];
		struct dx_edma_block *ll_block = &vsec_data.ll_wr[i];

		ll_region->vaddr = pcim_iomap_table(pdev)[ll_block->bar];
		if (!ll_region->vaddr) {
			err = -ENOMEM;
			goto err_irq_vectors;
		}

		ll_region->vaddr += ll_block->off;
		ll_region->paddr = dw->dma_desc_base_addr;
		ll_region->paddr += ll_block->off;
		ll_region->sz = ll_block->sz;

	}

	for (i = 0; i < dw->rd_ch_cnt; i++) {
		struct dx_edma_region *ll_region = &dw->ll_region_rd[i];
		struct dx_edma_block *ll_block = &vsec_data.ll_rd[i];

		ll_region->vaddr = pcim_iomap_table(pdev)[ll_block->bar];
		if (!ll_region->vaddr) {
			err = -ENOMEM;
			goto err_irq_vectors;
		}

		ll_region->vaddr += ll_block->off;
		ll_region->paddr = dw->dma_desc_base_addr;
		ll_region->paddr += ll_block->off;
		ll_region->sz = ll_block->sz;
	}
	/* Device Specific datas */
	dw->download_region = vsec_data.download_region;
	dw->download_size	= vsec_data.download_size;
	memcpy(dw->booting_region, vsec_data.booting_region, sizeof(vsec_data.booting_region));

	/* Debug info */
	pci_dbg(pdev, "Probe pdev:%p\n", pdev);
	if (dw->mf == DX_DMA_MF_EDMA_LEGACY)
		pci_dbg(pdev, "Version:\teDMA Port Logic (0x%x)\n", dw->mf);
	else if (dw->mf == DX_DMA_MF_EDMA_UNROLL)
		pci_dbg(pdev, "Version:\teDMA Unroll (0x%x)\n", dw->mf);
	else if (dw->mf == DX_DMA_MF_HDMA_COMPAT)
		pci_dbg(pdev, "Version:\tHDMA Compatible (0x%x)\n", dw->mf);
	else
		pci_dbg(pdev, "Version:\tUnknown (0x%x)\n", dw->mf);

	pci_dbg(pdev, "Descriptor Table:\tBAR=%d, addr=0x%llx, sz=0x%llx bytes, target_addr=0x%llx\n",
		dw->dma_desc_bar_num, dw->dma_desc_base_bar,
		dw->dma_desc_size, dw->dma_desc_base_addr);

	pci_dbg(pdev, "Registers:\tBAR=%u, off=0x%.8lx, sz=0x%zx bytes, addr(v=%p, p=%pa)\n",
		vsec_data.rg.bar, vsec_data.rg.off, vsec_data.rg.sz,
		dw->rg_region.vaddr, &dw->rg_region.paddr);

	for (i = 0; i < dw->wr_ch_cnt; i++) {
		pci_dbg(pdev, "L. List:\tWRITE CH%.2u, BAR=%u, off=0x%.8lx, sz=0x%zx bytes, addr(v=%p, p=%pa)\n",
			i, vsec_data.ll_wr[i].bar,
			vsec_data.ll_wr[i].off, dw->ll_region_wr[i].sz,
			dw->ll_region_wr[i].vaddr, &dw->ll_region_wr[i].paddr);
	}

	for (i = 0; i < dw->rd_ch_cnt; i++) {
		pci_dbg(pdev, "L. List:\tREAD CH%.2u, BAR=%u, off=0x%.8lx, sz=0x%zx bytes, addr(v=%p, p=%pa)\n",
			i, vsec_data.ll_rd[i].bar,
			vsec_data.ll_rd[i].off, dw->ll_region_rd[i].sz,
			dw->ll_region_rd[i].vaddr, &dw->ll_region_rd[i].paddr);
	}

	pci_dbg(pdev, "iATU:\t BAR=%u, off=0x%.8lx, sz=0x%zx bytes, addr(v=%p, p=%pa)\n",
		vsec_data.iatu.bar, vsec_data.iatu.off, vsec_data.iatu.sz,
		dw->iatu_region.vaddr, &dw->iatu_region.paddr);

	for (i = 0; i < dw->user_bar_cnt; i++) {
		pci_dbg(pdev, "USER%d:\t BAR=%u, off=0x%.8lx, sz=0x%zx bytes, addr(v=%p, p=%pa)\n",
			i, vsec_data.users[i].bar, vsec_data.users[i].off, vsec_data.users[i].sz,
			dw->npu_region[i].vaddr, &dw->npu_region[i].paddr);
	}

	pci_dbg(pdev, "Nr. IRQs:\t%u\n", dw->nr_irqs);

	{
		int msi_pos;
		u16 msi_control = 0;

		msi_pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
		if (!msi_pos) {
			pci_warn(pdev, "MSI capability not found\n");
		} else if (pci_read_config_word(pdev, msi_pos + PCI_MSI_FLAGS,
						 &msi_control)) {
			pci_warn(pdev, "failed to read MSI control register\n");
		} else {
			pci_info(pdev, "MSI config space: %s (control=0x%04x), pci_dev_msi_enabled=%s\n",
				 (msi_control & PCI_MSI_FLAGS_ENABLE) ? "Enable" : "Disable",
				 msi_control,
				 pci_dev_msi_enabled(pdev) ? "true" : "false");
		}
	}

	dw->irq = devm_kcalloc(dev, nr_irqs, sizeof(*dw->irq), GFP_KERNEL);
	if (!dw->irq) {
		err = -ENOMEM;
		goto err_irq_vectors;
	}

	/* Detect device number */
	err = dx_dev_list_add(chip->dw);
	if (err)
		goto err_irq_vectors;

	/* Initialize per-device mutexes before any code path can contend.
	 * dw is devm_kzalloc'd (zeroed) — without explicit mutex_init the
	 * wait_list head is NULL, which works on the uncontended fast-path
	 * but causes a NULL-pointer dereference on the slow-path. */
	mutex_init(&chip->dw->wr_lock);
	mutex_init(&chip->dw->rd_lock);

	/* Starting eDMA driver */
	err = dx_dma_probe(chip);
	if (err) {
		pci_err(pdev, "eDMA probe failed(%d)\n", err);
		goto err_dev_list_remove;
	}

	/* Saving data structure reference */
	pci_set_drvdata(pdev, chip);

	/* Create Cdev */
	err = xpdev_create_interfaces(chip);
	if (err)
		goto err_dma_remove;

	dw_edma_thread_init(chip->dw->idx);
	chip->dw->init_completed = true;

	/* Initialize link health monitoring before going LIVE */
	dx_link_health_init(chip->dw);

	atomic_set(&chip->dw->dev_state, DX_DEV_LIVE);

	/* Save PCI config state for link health recovery.
	 * pci_restore_state() in edma_restore uses this saved state
	 * to reprogram BARs, MSI, command register after link-down. */
	pci_save_state(pdev);

	/* Start health worker after LIVE — must be last */
	dx_link_health_start(chip->dw);

	pci_err(pdev, "[%s] Probe Done!!\n", __func__);

	return 0;

err_dma_remove:
	pci_set_drvdata(pdev, NULL);
	dx_dma_remove(chip);

err_dev_list_remove:
	dx_dev_list_remove(chip->dw);

err_irq_vectors:
	pci_free_irq_vectors(pdev);
	return err;
}

static void dx_dma_pcie_remove(struct pci_dev *pdev)
{
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);
	int err;

	pci_dbg(pdev, "[%s]\n", __func__);

	/* Mark device as going away — callers from dxrt_driver
	 * (dx_sgdma_deinit, dx_pcie_reset_dma_channels) check this
	 * state before touching dw->wr_lock to avoid use-after-free. */
	atomic_set(&chip->dw->dev_state, DX_DEV_REMOVING);

	/* Stop health worker synchronously before resource teardown.
	 * cancel_delayed_work_sync waits for in-flight worker to finish. */
	dx_link_health_stop(chip->dw);

	/* Stopping eDMA driver */
	err = dx_dma_remove(chip);
	if (err)
		pci_warn(pdev, "can't remove device properly: %d\n", err);

	/* Remove Cdev */
	xpdev_release_interfaces(chip->dw->xpdev);

	/* Remove device list */
	dx_dev_list_remove(chip->dw);

	/* Freeing IRQs */
	pci_free_irq_vectors(pdev);

	dw_edma_thread_exit(chip->dw->idx);
}

static void dx_dma_quiesce_channels(struct dw_edma *dw);
static int dx_dma_reinit_hw(struct pci_dev *pdev, struct dw_edma *dw);

#ifdef CONFIG_PM_SLEEP
static int dx_pcie_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);

	pci_info(pdev, "PM suspend: quiescing DMA and pausing recovery\n");

	if (!chip || !chip->dw)
		return 0;

	atomic_set(&chip->dw->background_recovery_paused, 1);
	dx_dma_quiesce_channels(chip->dw);
	cancel_delayed_work_sync(&chip->dw->health_work);
	cancel_work_sync(&chip->dw->recovery_work);
	pci_save_state(pdev);

	return 0;
}

static int dx_pcie_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);
	int ret;

	pci_info(pdev, "PM resume: restoring PCI and eDMA state\n");

	if (!chip || !chip->dw)
		return 0;

	ret = dx_dma_reinit_hw(pdev, chip->dw);
	atomic_set(&chip->dw->background_recovery_paused, 0);
	if (ret) {
		pci_err(pdev,
			"PM resume: reinit_hw failed (%d), health worker will retry\n",
			ret);
		if (atomic_read(&chip->dw->link_health_enabled))
			schedule_delayed_work(&chip->dw->health_work, 0);
		return ret;
	}

	dx_link_health_start(chip->dw);
	pci_info(pdev, "PM resume: done\n");

	return 0;
}

static SIMPLE_DEV_PM_OPS(dx_pcie_pm_ops, dx_pcie_suspend, dx_pcie_resume);
#endif /* CONFIG_PM_SLEEP */

/*
 * dx_dma_quiesce_channels - Quiesce all DMA channels before reset
 *
 * Common helper for both AER error_detected and reset_prepare paths.
 * Sets dev_state to AER_RESET, bumps recovery_epoch, sets hw_err on
 * every channel and wakes any sleeping transfer threads so they exit
 * immediately with -EIO instead of waiting for timeout.
 */
static void dx_dma_quiesce_channels(struct dw_edma *dw)
{
	int i;

	/* Block new submissions */
	atomic_set(&dw->dev_state, DX_DEV_AER_RESET);

	/* Bump epoch so in-flight threads detect stale context */
	atomic_inc(&dw->recovery_epoch);

	/* Full barrier: epoch + dev_state visible before hw_err / wq reads */
	smp_mb();

	/* Stop shadow prebuild work before resetting channel state. */
	if (dw->shadow_wq)
		flush_workqueue(dw->shadow_wq);

	/* Stop eDMA engine HW */
	dw_edma_v0_core_off(dw);

	/* Wake all sleeping transfer threads via hw_err */
	for (i = 0; i < dw->wr_ch_cnt + dw->rd_ch_cnt; i++) {
		struct dw_edma_chan *chan = &dw->chan[i];
		wait_queue_head_t *wq;

		WRITE_ONCE(chan->hw_err, true);
		wq = READ_ONCE(chan->transfer_wq);
		if (wq)
			wake_up(wq);
	}

	/* Notify RT module so in-flight ioctls are woken with -ENODATA and
	 * response queues are cleared.  Without this, AER / sysfs-reset
	 * would not propagate into the RT module -> ioctl hang. */
	dx_pcie_notify_link_event(dw->idx, DX_PCIE_LINK_EV_DOWN);
}

/*
 * dx_dma_reinit_hw - Reinitialize PCIe + DMA HW after reset
 *
 * Delegates to dx_dma_full_reinit() which does the full register
 * reprogramming sequence (PCI restore, iATU, engine_en, IMWR,
 * ch_pwr_en, MSI cache, link validation) and notifies the RT
 * module via LINK_EV_UP.  This keeps AER / sysfs-reset and
 * link-health paths in sync.
 */
static int dx_dma_reinit_hw(struct pci_dev *pdev, struct dw_edma *dw)
{
	(void)pdev;
	return dx_dma_full_reinit(dw);
}

static pci_ers_result_t dx_dma_pcie_error_detected(struct pci_dev *pdev,
						 pci_channel_state_t error)
{
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);

	pci_err(pdev, ">> %s: error=%d\n", __func__, error);

	if (!chip || !chip->dw)
		return PCI_ERS_RESULT_DISCONNECT;

	atomic_set(&chip->dw->background_recovery_paused, 1);
	if (error == pci_channel_io_normal) {
		dx_dma_quiesce_channels(chip->dw);
	} else {
		dx_dma_quiesce_for_link_down(chip->dw);
	}

	/* Drain background recovery after AER_RESET is visible so workers
	 * cannot re-arm themselves during reset handling. */
	cancel_delayed_work_sync(&chip->dw->health_work);
	cancel_work_sync(&chip->dw->recovery_work);

	if (error == pci_channel_io_perm_failure)
		return PCI_ERS_RESULT_DISCONNECT;

	return PCI_ERS_RESULT_NEED_RESET;
}

static pci_ers_result_t dx_dma_pcie_error_slot_reset(struct pci_dev *pdev)
{
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);

	pci_err(pdev, ">> %s\n", __func__);

	if (!chip || !chip->dw)
		return PCI_ERS_RESULT_DISCONNECT;

	/* Re-enable the device after slot reset */
	if (pci_enable_device(pdev)) {
		pci_err(pdev, "Cannot re-enable device after reset\n");
		return PCI_ERS_RESULT_DISCONNECT;
	}

	if (dx_dma_reinit_hw(pdev, chip->dw) < 0) {
		pci_err(pdev, "reinit_hw failed after slot_reset\n");
		return PCI_ERS_RESULT_DISCONNECT;
	}

	return PCI_ERS_RESULT_RECOVERED;
}

static void dx_dma_pcie_error_resume(struct pci_dev *pdev)
{
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);

	pci_err(pdev, ">> %s\n", __func__);

	if (!chip || !chip->dw)
		return;

	/*
	 * reinit_hw already set dev_state = LIVE in slot_reset.
	 * resume is a no-op unless slot_reset was not called.
	 */
	if (atomic_read(&chip->dw->dev_state) != DX_DEV_LIVE)
		atomic_set(&chip->dw->dev_state, DX_DEV_LIVE);
	atomic_set(&chip->dw->background_recovery_paused, 0);

	/* Restart health worker after AER recovery if user enabled it. */
	dx_link_health_start(chip->dw);
}

/*
 * reset_prepare / reset_done — called by the kernel's pci_reset_function()
 * path, e.g. when userspace writes to /sys/bus/pci/devices/XXXX/reset.
 * Unlike AER callbacks, these are invoked for any kernel-initiated reset
 * (FLR, bus reset via sysfs, etc.) regardless of whether AER is enabled.
 */
static void dx_dma_pcie_reset_prepare(struct pci_dev *pdev)
{
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);

	pci_err(pdev, ">> %s\n", __func__);

	if (!chip || !chip->dw)
		return;

	atomic_set(&chip->dw->background_recovery_paused, 1);
	dx_dma_quiesce_channels(chip->dw);
	cancel_delayed_work_sync(&chip->dw->health_work);
	cancel_work_sync(&chip->dw->recovery_work);
	pci_save_state(pdev);
}

static void dx_dma_pcie_reset_done(struct pci_dev *pdev)
{
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);

	pci_err(pdev, ">> %s\n", __func__);

	if (!chip || !chip->dw)
		return;

	if (dx_dma_reinit_hw(pdev, chip->dw) < 0)
		pci_err(pdev, "reinit_hw failed after reset_done\n");
	atomic_set(&chip->dw->background_recovery_paused, 0);

	/* Restart health worker after reset recovery if user enabled it. */
	dx_link_health_start(chip->dw);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
/*
 * Pre-4.13 kernels (e.g. 4.4 LTS) don't have the split
 * reset_prepare()/reset_done() callbacks in struct pci_error_handlers -
 * they only have a single reset_notify(dev, prepare) callback. Dispatch
 * to the same prepare/done implementations so behavior is identical.
 */
static void dx_dma_pcie_reset_notify(struct pci_dev *pdev, bool prepare)
{
	if (prepare)
		dx_dma_pcie_reset_prepare(pdev);
	else
		dx_dma_pcie_reset_done(pdev);
}
#endif

static pci_ers_result_t dx_dma_pcie_error_mmio_enabled(struct pci_dev *pdev) __attribute__((unused));
static pci_ers_result_t dx_dma_pcie_error_mmio_enabled(struct pci_dev *pdev)
{
	/*
	 * Unregistered (kernel default .mmio_enabled == NULL returns
	 * PCI_ERS_RESULT_RECOVERED already).  Retained for future
	 * diagnostic hook-in; re-add to dx_dma_err_handler if needed.
	 */
	(void)pdev;
	return PCI_ERS_RESULT_RECOVERED;
}

#ifdef CONFIG_PCI_IOV
static int dx_dma_pcie_sriov_configure(struct pci_dev *pdev, int num_vfs)
{
	int ret, max_vfs;
	pci_err(pdev, ">> %s : %d\n", __func__, num_vfs);

	if (num_vfs == 0) {
		pci_disable_sriov(pdev);
		return 0;
	}

	max_vfs = pci_sriov_get_totalvfs(pdev);
	if (num_vfs > max_vfs)
		return -EINVAL;

	ret = pci_enable_sriov(pdev, num_vfs);
	if (ret) {
		return ret;
	}
	return num_vfs;
}
#endif

static void dx_dma_pcie_shutdown(struct pci_dev *pdev)
{
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);
	struct dw_edma *dw;
	enum dx_link_state link_state;
	u16 command = 0;
	int ret;
	bool mmio_safe;

	if (!chip || !chip->dw) {
		pci_err(pdev, "Invalid chip data during shutdown\n");
		return;
	}

	dw = chip->dw;
	pci_info(pdev, "shutdown: quiescing DMA and stopping background workers\n");
	WRITE_ONCE(dw->shutting_down, true);
	atomic_set(&dw->background_recovery_paused, 1);
	dx_link_health_stop(dw);
	link_state = dx_pcie_check_link_health(dw);
	ret = pci_read_config_word(pdev, PCI_COMMAND, &command);
	mmio_safe = link_state == DX_LINK_UP &&
		    ret == PCIBIOS_SUCCESSFUL &&
		    (command & PCI_COMMAND_MEMORY);
	if (mmio_safe)
		dx_dma_quiesce_channels(dw);
	else
		dx_dma_quiesce_for_link_down(dw);

	#ifdef RPI_BUILD
	if (mmio_safe && dw->dx_ver == 3) {
		if (dw->dx_msg && dw->dx_msg->notify) {
			void __iomem *notify = (u8 __iomem *)dw->dx_msg->notify +
				EP_IRQ_RPI_SHUTDOWN_OFFSET;

			writel(1, notify);
			readl(notify);
		} else {
			pci_warn(pdev, "RPI shutdown notify skipped: dx_msg not initialized\n");
		}
	} else if (!mmio_safe) {
		pci_warn(pdev,
			 "RPI shutdown notify skipped: MMIO unsafe (link_state=%d, command=0x%04x, ret=%d)\n",
			 link_state, command, ret);
	}
	pci_err(pdev, ">> %s: RPI Shutdown\n", __func__);
	#endif
	pci_clear_master(pdev);
	pci_disable_device(pdev);
	pci_err(pdev, ">> %s: Standard Shutdown\n", __func__);
}

static const struct pci_device_id dx_dma_pcie_id_table[] = {
	/* TODO: deprecation in 261231 */
	{ PCI_DEVICE(DX_PCI_VENDOR_ID, 
		DX_PCI_LEGACY_DEVICE_ID
		), .driver_data = (kernel_ulong_t)(&dx_pcie_data_v3) },
	{ PCI_DEVICE_SUB(DX_PCI_VENDOR_ID, 
		DX_PCI_LEGACY_DEVICE_ID, 
		DX_PCI_SUB_VENDOR_ID, 
		DX_PCI_LEGACY_SUB_DEVICE_ID
		), .driver_data = (kernel_ulong_t)(&dx_pcie_data_v3) },
	/* M1 / M1M / H1[M1] / H1[M1M] */
	{ PCI_DEVICE_SUB(DX_PCI_VENDOR_ID, 
		DX_M1_PCI_DEVICE_ID, 
		DX_PCI_SUB_VENDOR_ID, 
		DX_M1_PCI_SUB_DEVICE_ID
		), .driver_data = (kernel_ulong_t)(&dx_pcie_data_v3) },
	{ PCI_DEVICE_SUB(DX_PCI_VENDOR_ID, 
		DX_M1M_PCI_DEVICE_ID, 
		DX_PCI_SUB_VENDOR_ID, 
		DX_M1M_PCI_SUB_DEVICE_ID
		), .driver_data = (kernel_ulong_t)(&dx_pcie_data_v3) },
	{ PCI_DEVICE_SUB(DX_PCI_VENDOR_ID, 
		DX_H1_PCI_DEVICE_ID, 
		DX_PCI_SUB_VENDOR_ID, 
		DX_H1_PCI_SUB_DEVICE_ID
		), .driver_data = (kernel_ulong_t)(&dx_pcie_data_v3) },
	{ PCI_DEVICE_SUB(DX_PCI_VENDOR_ID, 
		DX_H1M_PCI_DEVICE_ID, 
		DX_PCI_SUB_VENDOR_ID, 
		DX_H1M_PCI_SUB_DEVICE_ID
		), .driver_data = (kernel_ulong_t)(&dx_pcie_data_v3) },
	{ }
};
MODULE_DEVICE_TABLE(pci, dx_dma_pcie_id_table);

static const struct pci_error_handlers dx_dma_err_handler = {
	.error_detected = dx_dma_pcie_error_detected,
	.slot_reset 	= dx_dma_pcie_error_slot_reset,
	.resume 	= dx_dma_pcie_error_resume,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0))
	.reset_prepare	= dx_dma_pcie_reset_prepare,
	.reset_done	= dx_dma_pcie_reset_done,
#else
	.reset_notify	= dx_dma_pcie_reset_notify,
#endif
};

static struct pci_driver dx_dma_pcie_driver = {
	.name		= "dx_dma_pcie",
	.id_table	= dx_dma_pcie_id_table,
	.probe		= dx_dma_pcie_probe,
	.remove		= dx_dma_pcie_remove,
	.shutdown	= dx_dma_pcie_shutdown,
	.err_handler	= &dx_dma_err_handler,
#ifdef CONFIG_PM_SLEEP
	.driver		= {
		.pm	= &dx_pcie_pm_ops,
	},
#endif
#ifdef CONFIG_PCI_IOV
	.sriov_configure = dx_dma_pcie_sriov_configure,
#endif
};

static int dx_dma_mod_init(void)
{
	int rv;

	dbg_init("pci_register_driver.\n");
	rv = dx_cdev_init();
	if (rv < 0)
		return rv;
	dw_edma_thread_probe();

	rv = pci_register_driver(&dx_dma_pcie_driver);
	if (rv < 0) {
		dx_cdev_cleanup();
		return rv;
	}

	dx_dma_sysfs_create(&dx_dma_pcie_driver);
	return 0;
}

static void dx_dma_mod_exit(void)
{
	/* unregister this driver from the PCI bus driver */
	dbg_init("pci_unregister_driver.\n");
	dx_dma_sysfs_remove(&dx_dma_pcie_driver);
	pci_unregister_driver(&dx_dma_pcie_driver);
	dx_cdev_cleanup();
}

module_init(dx_dma_mod_init);
module_exit(dx_dma_mod_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("DeepX eDMA PCIe driver");
MODULE_AUTHOR("Taegyun An <atg@deepx.ai>");
MODULE_VERSION(DRV_MODULE_VERSION);
