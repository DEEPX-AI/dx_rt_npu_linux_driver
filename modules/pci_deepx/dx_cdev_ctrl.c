// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */
#include <linux/fs.h>

#include "dx_cdev_ctrl.h"
#include "dw-edma-core.h"
#include "dx_lib.h"
#include "version.h"
#include "dx_util.h"
#include "dx_pcie_api.h"

#if ACCESS_OK_2_ARGS
#define xlx_access_ok(X, Y, Z) access_ok(Y, Z)
#else
#define xlx_access_ok(X, Y, Z) access_ok(X, Y, Z)
#endif

#ifndef VM_RESERVED
	#define VMEM_FLAGS (VM_IO | VM_DONTEXPAND | VM_DONTDUMP)
#else
	#define VMEM_FLAGS (VM_IO | VM_RESERVED)
#endif

int char_ctrl_open(struct inode *inode, struct file *file)
{
	struct dx_dma_cdev *xcdev;

	/* pointer to containing structure of the character device inode */
	xcdev = container_of(inode->i_cdev, struct dx_dma_cdev, cdev);
	dbg_sg("[%s] - %s\n", __func__, xcdev->sys_device->kobj.name);
	if (!xcdev)
		return -1;
	/* create a reference to our char device in the opened file */
	file->private_data = xcdev;
	return 0;
}

int char_ctrl_close(struct inode *inode, struct file *file)
{
#ifdef DX_DEBUG_ENABLE
	struct dx_dma_cdev *xcdev;

	/* pointer to containing structure of the character device inode */
	xcdev = container_of(inode->i_cdev, struct dx_dma_cdev, cdev);
	dbg_sg("[%s] - %s\n", __func__, xcdev->sys_device->kobj.name);
#endif
	return 0;
}

static void __iomem *xcdev_get_match_bar_addr(struct dx_dma_cdev *xcdev)
{
	struct dw_edma *dw = xcdev->xpdev->dw;
	void __iomem *bar_addr = NULL;
	int i;

	for (i = 0; i < dw->user_bar_cnt; i++) {
		if (xcdev->bar == dw->npu_region[i].bar_num) {
			bar_addr = dw->npu_region[i].vaddr;
			break;
		}
	}
	return bar_addr;
}
/*
 * character device file operations for control bus (through control bridge)
 */
static ssize_t char_ctrl_read(struct file *fp, char __user *buf, size_t count,
		loff_t *pos)
{
	struct dx_dma_cdev *xcdev = (struct dx_dma_cdev *)fp->private_data;
	void __iomem *reg;
	u32 w;
	int rv;

	rv = dx_cdev_check(__func__, xcdev, 0);
	if (rv < 0)
		return rv;

	/* only 32-bit aligned and 32-bit multiples */
	if (*pos & 3)
		return -EPROTO;
	/* first address is BAR base plus file position offset */
	reg = xcdev_get_match_bar_addr(xcdev);
	if (!reg)
		return -EBADSLT;
	reg += *pos;

	//w = read_register(reg);
	w = ioread32(reg);
	dbg_sg("%s(@%p, count=%ld, pos=%d) value = 0x%08x\n",
			__func__, reg, (long)count, (int)*pos, w);
	rv = copy_to_user(buf, &w, 4);
	if (rv)
		pr_info("Copy to userspace failed but continuing\n");

	*pos += 4;
	return 4;
}

static ssize_t char_ctrl_write(struct file *file, const char __user *buf,
			size_t count, loff_t *pos)
{
	struct dx_dma_cdev *xcdev = (struct dx_dma_cdev *)file->private_data;
	void __iomem *reg;
	u32 w;
	int rv;

	rv = dx_cdev_check(__func__, xcdev, 0);
	if (rv < 0)
		return rv;

	/* only 32-bit aligned and 32-bit multiples */
	if (*pos & 3)
		return -EPROTO;

	/* first address is BAR base plus file position offset */
	reg = xcdev_get_match_bar_addr(xcdev);
	if (!reg)
		return -EBADSLT;
	reg += *pos;

	rv = copy_from_user(&w, buf, 4);
	if (rv)
		pr_info("copy from user failed %d/4, but continuing.\n", rv);

	dbg_sg("%s(0x%08x @%p, count=%ld, pos=%d)\n",
			__func__, w, reg, (long)count, (int)*pos);
	//write_register(w, reg);
	iowrite32(w, reg);
	*pos += 4;
	return 4;
}

static long version_ioctl(struct dx_dma_cdev *xcdev, void __user *arg)
{
	struct dx_dma_ioc_info obj;
	struct dx_dma_pci_dev *xpdev = xcdev->xpdev;
	struct dw_edma *dw = xcdev->xpdev->dw;
	int rv;

	rv = copy_from_user((void *)&obj, arg, sizeof(struct dx_dma_ioc_info));
	if (rv) {
		pr_info("copy from user failed %d/%ld.\n",
			rv, sizeof(struct dx_dma_ioc_info));
		return -EFAULT;
	}
	memset(&obj, 0, sizeof(obj));
	obj.vendor = xpdev->pdev->vendor;
	obj.device = xpdev->pdev->device;
	obj.subsystem_vendor = xpdev->pdev->subsystem_vendor;
	obj.subsystem_device = xpdev->pdev->subsystem_device;
	obj.feature_id = dw->idx;
	obj.driver_version = DRV_MOD_VERSION_NUMBER;
	obj.domain = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
	obj.bus = PCI_BUS_NUM(pci_dev_id(xpdev->pdev));
	obj.dev = PCI_SLOT(pci_dev_id(xpdev->pdev));
	obj.func = PCI_FUNC(pci_dev_id(xpdev->pdev));
#else
	obj.bus = PCI_BUS_NUM(xpdev->pdev->devfn);
	obj.dev = PCI_SLOT(xpdev->pdev->devfn);
	obj.func = PCI_FUNC(xpdev->pdev->devfn);
#endif
	dbg_sg("vendor             : 0x%x\n",   obj.vendor);
	dbg_sg("device             : 0x%x\n",   obj.device);
	dbg_sg("subsystem_vendor   : 0x%x\n",   obj.subsystem_vendor);
	dbg_sg("subsystem_device   : 0x%x\n",   obj.subsystem_device);
	dbg_sg("dma_engine_version : 0x%x\n",   obj.dma_engine_version);
	dbg_sg("driver_version     : %d\n",     obj.driver_version);
	dbg_sg("feature_id         : 0x%llx\n", obj.feature_id);
	dbg_sg("domain             : 0x%x\n",   obj.domain);
	dbg_sg("bus                : 0x%x\n",   obj.bus);
	dbg_sg("dev                : 0x%x\n",   obj.dev);
	dbg_sg("func               : 0x%x\n",   obj.func);

	if (copy_to_user(arg, &obj, sizeof(struct dx_dma_ioc_info)))
		return -EFAULT;
	return 0;
}

static int dx_get_current_spd(struct dw_edma *dw)
{
	struct pci_dev *pci_dev = dw->pdev;
	u16 linkstat;
	if (pcie_capability_read_word(pci_dev, PCI_EXP_LNKSTA, &linkstat))
		return -EINVAL;
	return linkstat & PCI_EXP_LNKSTA_CLS;
}

static int dx_get_current_width(struct dw_edma *dw)
{
	struct pci_dev *pci_dev = dw->pdev;
	u16 linkstat;
	if (pcie_capability_read_word(pci_dev, PCI_EXP_LNKSTA, &linkstat))
		return -EINVAL;
	return (linkstat & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;
}

void dx_pcie_get_driver_info(struct deepx_pcie_info *info, int dev_id)
{
	struct dw_edma *dw = dx_dev_list_get(dev_id);

	info->driver_version = DRV_MOD_VERSION_NUMBER;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
	info->bus = PCI_BUS_NUM(pci_dev_id(dw->pdev));
	info->dev = PCI_SLOT(pci_dev_id(dw->pdev));
	info->func = PCI_FUNC(pci_dev_id(dw->pdev));
#else
	info->bus = PCI_BUS_NUM(dw->pdev->devfn);
	info->dev = PCI_SLOT(dw->pdev->devfn);
	info->func = PCI_FUNC(dw->pdev->devfn);
#endif
	info->speed = dx_get_current_spd(dw);
	info->width = dx_get_current_width(dw);
}
EXPORT_SYMBOL_GPL(dx_pcie_get_driver_info);

long char_ctrl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct dx_dma_cdev *xcdev = (struct dx_dma_cdev *)filp->private_data;
	struct dw_edma *dw = xcdev->xpdev->dw;
	struct dx_dma_ioc_base ioctl_obj;
	long result = 0;
	int rv;

	rv = dx_cdev_check(__func__, xcdev, 0);
	if (rv < 0)
		return rv;

	if (!dw) {
		pr_info("cmd %u, dw NULL.\n", cmd);
		return -EINVAL;
	}
	dbg_sg("cmd 0x%x, dw 0x%p, pdev 0x%p.\n", cmd, dw, dw->pdev);

	if (_IOC_TYPE(cmd) != DX_DMA_IOC_MAGIC) {
		pr_err("cmd %u, bad magic 0x%x/0x%x.\n",
			 cmd, _IOC_TYPE(cmd), DX_DMA_IOC_MAGIC);
		return -ENOTTY;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		result = !xlx_access_ok(VERIFY_WRITE, (void __user *)arg,
				_IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		result =  !xlx_access_ok(VERIFY_READ, (void __user *)arg,
				_IOC_SIZE(cmd));

	if (result) {
		pr_err("bad access %ld.\n", result);
		return -EFAULT;
	}

	switch (cmd) {
	case DX_DMA_IOCINFO:
		if (copy_from_user((void *)&ioctl_obj, (void __user *) arg,
			 sizeof(struct dx_dma_ioc_base))) {
			pr_err("copy_from_user failed.\n");
			return -EFAULT;
		}

		if (ioctl_obj.magic != DX_DMA_XCL_MAGIC) {
			pr_err("magic 0x%x !=  DX_DMA_XCL_MAGIC (0x%x).\n",
				ioctl_obj.magic, DX_DMA_XCL_MAGIC);
			return -ENOTTY;
		}
		return version_ioctl(xcdev, (void __user *)arg);
	// case DX_DMA_IOCOFFLINE:
	// 	xdma_device_offline(xdev->pdev, xdev);
	// 	break;
	// case DX_DMA_IOCONLINE:
	// 	xdma_device_online(xdev->pdev, xdev);
	// 	break;
	default:
		pr_err("UNKNOWN ioctl cmd 0x%x.\n", cmd);
		return -ENOTTY;
	}
	return 0;
}

/* maps the PCIe BAR into user space for memory-like access using mmap() */
int bridge_mmap(struct file *file, struct vm_area_struct *vma)
{
	// struct xdma_dev *xdev;
	struct dx_dma_cdev *xcdev = (struct dx_dma_cdev *)file->private_data;
	struct dw_edma *dw = xcdev->xpdev->dw;
	unsigned long off;
	unsigned long phys;
	unsigned long vsize;
	unsigned long psize;
	int rv;
	int bar_num = xcdev->bar;

	// rv = xcdev_check(__func__, xcdev, 0);
	// if (rv < 0)
	// 	return rv;
	// xdev = xcdev->xdev;

	off = vma->vm_pgoff << PAGE_SHIFT;
	/* BAR physical address */
	phys = pci_resource_start(dw->pdev, bar_num) + off;
	vsize = vma->vm_end - vma->vm_start;
	/* complete resource */
	psize = pci_resource_end(dw->pdev, bar_num) -
		pci_resource_start(dw->pdev, bar_num) + 1 - off;

	dbg_sg("mmap(): xcdev = 0x%p\n", xcdev);
	dbg_sg("mmap(): cdev->bar = %d\n", bar_num);
	dbg_sg("mmap(): dw = 0x%p\n", dw);
	dbg_sg("mmap(): pci_dev = 0x%p\n", dw->pdev);

	dbg_sg("off = 0x%lx\n", off);
	dbg_sg("start = 0x%llx\n",
		(unsigned long long)pci_resource_start(dw->pdev,
		bar_num));
	dbg_sg("phys = 0x%lx\n", phys);

	if (vsize > psize)
		return -EINVAL;
	/*
	 * pages must not be cached as this would result in cache line sized
	 * accesses to the end point
	 */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	/*
	 * prevent touching the pages (byte access) for swap-in,
	 * and prevent the pages from being swapped out
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0))
	vma->vm_flags |= VMEM_FLAGS;
#endif
	/* make MMIO accessible to user space */
	rv = io_remap_pfn_range(vma, vma->vm_start, phys >> PAGE_SHIFT,
			vsize, vma->vm_page_prot);
	dbg_sg("vma=0x%p, vma->vm_start=0x%lx, phys=0x%lx, size=%lu = %d\n",
		vma, vma->vm_start, phys >> PAGE_SHIFT, vsize, rv);

	if (rv)
		return -EAGAIN;
	return 0;
}

/*
 * character device file operations for control bus (through control bridge)
 */
static const struct file_operations ctrl_fops = {
	.owner = THIS_MODULE,
	.open = char_ctrl_open,
	.release = char_ctrl_close,
	.read = char_ctrl_read,
	.write = char_ctrl_write,
	.mmap = bridge_mmap,
	.unlocked_ioctl = char_ctrl_ioctl,
};

void dx_cdev_ctrl_init(struct dx_dma_cdev *xcdev)
{
	cdev_init(&xcdev->cdev, &ctrl_fops);
}
