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
