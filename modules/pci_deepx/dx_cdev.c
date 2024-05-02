// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */
#include <linux/fs.h>

#include "dx_cdev.h"
#include "dx_sgdma_cdev.h"
#include "dx_cdev_ctrl.h"
#include "dx_lib.h"

static struct class *g_edma_class;

struct kmem_cache *cdev_cache;

static int xpdev_cnt;
#ifdef DX_DEBUG_FS
static int user_cnt = 0;
#endif

enum cdev_type {
	CHAR_USER,
	CHAR_EVENTS,
	CHAR_XDMA_H2C,
	CHAR_XDMA_C2H,
};

static const char * const devnode_names[] = {
	DX_DMA_NODE_NAME "%d_npu_%d",
	DX_DMA_NODE_NAME "%d_events_%d",
	DX_DMA_NODE_NAME "%d_h2c_%d",
	DX_DMA_NODE_NAME "%d_c2h_%d",
};

enum xpdev_flags_bits {
	XDF_CDEV_USER,
	XDF_CDEV_EVENT,
	XDF_CDEV_SG,
};

static inline void xpdev_flag_set(struct dx_dma_pci_dev *xpdev,
				enum xpdev_flags_bits fbit)
{
	xpdev->flags |= 1 << fbit;
}

static inline void xcdev_flag_clear(struct dx_dma_pci_dev *xpdev,
				enum xpdev_flags_bits fbit)
{
	xpdev->flags &= ~(1 << fbit);
}

static inline int xpdev_flag_test(struct dx_dma_pci_dev *xpdev,
				enum xpdev_flags_bits fbit)
{
	return xpdev->flags & (1 << fbit);
}

int dx_cdev_check(const char *fname, struct dx_dma_cdev *xcdev, bool check_engine)
{
	// struct dw_edma *dw;

	if (!xcdev || xcdev->magic != MAGIC_CHAR) {
		pr_info("%s, xcdev 0x%p, magic 0x%lx.\n",
			fname, xcdev, xcdev ? xcdev->magic : 0xFFFFFFFF);
		return -EINVAL;
	}

	// xdev = xcdev->xdev;
	// if (!xdev || xdev->magic != MAGIC_DEVICE) {
	// 	pr_info("%s, xdev 0x%p, magic 0x%lx.\n",
	// 		fname, xdev, xdev ? xdev->magic : 0xFFFFFFFF);
	// 	return -EINVAL;
	// }

	// if (check_engine) {
	// 	struct xdma_engine *engine = xcdev->engine;

	// 	if (!engine || engine->magic != MAGIC_ENGINE) {
	// 		pr_info("%s, engine 0x%p, magic 0x%lx.\n", fname,
	// 			engine, engine ? engine->magic : 0xFFFFFFFF);
	// 		return -EINVAL;
	// 	}
	// }

	return 0;
}

#ifdef __XDMA_SYSFS__
ssize_t xdma_dev_instance_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct xdma_pci_dev *xpdev =
		(struct xdma_pci_dev *)dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\t%d\n",
			xpdev->major, xpdev->xdev->idx);
}

static DEVICE_ATTR_RO(xdma_dev_instance);
#endif

#ifdef DX_DEBUG_FS
static int config_kobject(struct dw_edma *dw, struct dx_dma_cdev *xcdev, enum cdev_type type, u16 cnt)
{
	int rv = -EINVAL;

	switch (type) {
	case CHAR_XDMA_H2C:
	case CHAR_XDMA_C2H:
		rv = kobject_set_name(&xcdev->cdev.kobj, devnode_names[type],
			dw->idx, cnt);
		break;
	case CHAR_USER:
		rv = kobject_set_name(&xcdev->cdev.kobj, devnode_names[type],
			dw->idx, cnt);
		break;
	case CHAR_EVENTS:
		rv = kobject_set_name(&xcdev->cdev.kobj, devnode_names[type],
			dw->idx, cnt);
		break;
	default:
		pr_warn("%s: UNKNOWN type 0x%x.\n", __func__, type);
		break;
	}

	if (rv)
		pr_err("%s: type 0x%x, failed %d.\n", __func__, type, rv);
	return rv;
}
#endif

int char_open(struct inode *inode, struct file *file)
{
	struct dx_dma_cdev *xcdev;
	xcdev = container_of(inode->i_cdev, struct dx_dma_cdev, cdev);

	/* pointer to containing structure of the character device inode */
	dbg_sg("[%s] - %s\n", __func__, xcdev->sys_device->kobj.name);
	if (xcdev->magic != MAGIC_CHAR) {
		pr_err("xcdev 0x%p inode 0x%lx magic mismatch 0x%lx\n",
			xcdev, inode->i_ino, xcdev->magic);
		return -EINVAL;
	}
	/* create a reference to our char device in the opened file */
	file->private_data = xcdev;

	return 0;
}

/*
 * Called when the device goes from used to unused.
 */
int char_close(struct inode *inode, struct file *file)
{
	// struct xdma_dev *xdev;
	struct dx_dma_cdev *xcdev = (struct dx_dma_cdev *)file->private_data;

	dbg_sg("[%s] - %s\n", __func__, xcdev->sys_device->kobj.name);

	if (!xcdev) {
		pr_err("char device with inode 0x%lx xcdev NULL\n",
			inode->i_ino);
		return -EINVAL;
	}

	if (xcdev->magic != MAGIC_CHAR) {
		pr_err("xcdev 0x%p magic mismatch 0x%lx\n",
				xcdev, xcdev->magic);
		return -EINVAL;
	}

	// /* fetch device specific data stored earlier during open */
	// xdev = xcdev->xdev;
	// if (!xdev) {
	// 	pr_err("char device with inode 0x%lx xdev NULL\n",
	// 		inode->i_ino);
	// 	return -EINVAL;
	// }

	// if (xdev->magic != MAGIC_DEVICE) {
	// 	pr_err("xdev 0x%p magic mismatch 0x%lx\n", xdev, xdev->magic);
	// 	return -EINVAL;
	// }

	return 0;
}

/* create_xcdev() -- create a character device interface to data or control bus
 *
 * If at least one SG DMA engine is specified, the character device interface
 * is coupled to the SG DMA file operations which operate on the data bus. If
 * no engines are specified, the interface is coupled with the control bus.
 */
#ifdef DX_DEBUG_FS
static int create_sys_device(struct dx_dma_cdev *xcdev, enum cdev_type type, int idx)
{
	// struct xdma_dev *xdev = xcdev->xdev;
	int last_param = 0;

	if (type == CHAR_USER) {
		last_param = user_cnt++;
	} else {
		last_param = idx;
	}
	dbg_sg("Create device : %s.%d.%d\n", devnode_names[type], xcdev->xpdev->dw->idx, last_param);

	xcdev->sys_device = device_create(g_edma_class, &xcdev->xpdev->pdev->dev,
		xcdev->cdevno, NULL, devnode_names[type], xcdev->xpdev->dw->idx,
		last_param);

	if (!xcdev->sys_device) {
		pr_err("device_create(%s) failed\n", devnode_names[type]);
		return -1;
	}

	return 0;
}
#endif

static int destroy_xcdev(struct dx_dma_cdev *cdev)
{
	if (!cdev) {
		pr_warn("cdev NULL.\n");
		return -EINVAL;
	}
	if (cdev->magic != MAGIC_CHAR) {
		pr_warn("cdev 0x%p magic mismatch 0x%lx\n", cdev, cdev->magic);
		return -EINVAL;
	}

	if (!cdev->xpdev) {
		pr_err("xpdev NULL\n");
		return -EINVAL;
	}

	if (!g_edma_class) {
		pr_err("g_edma_class NULL\n");
		return -EINVAL;
	}

	if (!cdev->sys_device) {
		pr_err("cdev sys_device NULL\n");
		return -EINVAL;
	}

	if (cdev->sys_device)
		device_destroy(g_edma_class, cdev->cdevno);

	cdev_del(&cdev->cdev);

	return 0;
}

#ifdef DX_DEBUG_FS
static int create_xcdev(struct dx_dma_pci_dev *xpdev, struct dx_dma_cdev *xcdev,
			int bar, enum cdev_type type, u16 cnt)
{
	int rv;
	int minor;
	// struct xdma_dev *xdev = xpdev->xdev;
	dev_t dev;

	spin_lock_init(&xcdev->lock);
	/* new instance? */
	if (!xpdev->major) {
		/* allocate a dynamically allocated char device node */
		int rv = alloc_chrdev_region(&dev, DX_DMA_MINOR_BASE,
					DX_DMA_MINOR_COUNT, DX_DMA_NODE_NAME);

		if (rv) {
			pr_err("unable to allocate cdev region %d.\n", rv);
			return rv;
		}
		xpdev->major = MAJOR(dev);
	}

	/*
	 * do not register yet, create kobjects and name them,
	 */
	xcdev->magic = MAGIC_CHAR;
	xcdev->cdev.owner = THIS_MODULE;
	xcdev->xpdev = xpdev;
	xcdev->bar = bar;
	xcdev->npu_id = cnt;

	rv = config_kobject(xpdev->dw, xcdev, type, cnt);
	if (rv < 0)
		return rv;

	switch (type) {
	case CHAR_USER:
		/* minor number is type index for non-SGDMA interfaces */
		minor = type + cnt;
		dx_cdev_ctrl_init(xcdev);
		break;
	case CHAR_XDMA_H2C:
		minor = 32 + cnt;
		xcdev->write = false;
		dx_cdev_sgdma_init(xcdev);
		break;
	case CHAR_XDMA_C2H:
		minor = 36 + cnt;
		xcdev->write = true;
		dx_cdev_sgdma_init(xcdev);
		break;
	case CHAR_EVENTS:
		minor = 10 + cnt;
		dx_cdev_event_init(xcdev, cnt);
		break;
	default:
		pr_info("type 0x%x NOT supported.\n", type);
		return -EINVAL;
	}
	xcdev->cdevno = MKDEV(xpdev->major, minor);

	/* bring character device live */
	rv = cdev_add(&xcdev->cdev, xcdev->cdevno, 1);
	if (rv < 0) {
		pr_err("cdev_add failed %d, type 0x%x.\n", rv, type);
		goto unregister_region;
	}

	dbg_sg("xcdev 0x%p, %u:%u, %s, type 0x%x.\n",
		xcdev, xpdev->major, minor, xcdev->cdev.kobj.name, type);

	/* create device on our class */
	if (g_edma_class) {
		rv = create_sys_device(xcdev, type, cnt);
		if (rv < 0)
			goto del_cdev;
	}

	return 0;

del_cdev:
	cdev_del(&xcdev->cdev);
unregister_region:
	unregister_chrdev_region(xcdev->cdevno, DX_DMA_MINOR_COUNT);
	return rv;
}
#endif

void xpdev_destroy_interfaces(struct dx_dma_pci_dev *xpdev)
{
	struct dw_edma *dw = xpdev->dw;
	int i = 0;
	int rv;
#ifdef __XDMA_SYSFS__
	device_remove_file(&xpdev->pdev->dev, &dev_attr_xdma_dev_instance);
#endif

	if (xpdev_flag_test(xpdev, XDF_CDEV_SG)) {
		/* iterate over channels */
		for (i = 0; i < xpdev->h2c_channel_max; i++) {
			/* remove SG DMA character device */
			rv = destroy_xcdev(&xpdev->sgdma_h2c_cdev[i]);
			if (rv < 0)
				pr_err("Failed to destroy h2c xcdev %d error :0x%x\n",
						i, rv);
		}
		for (i = 0; i < xpdev->c2h_channel_max; i++) {
			rv = destroy_xcdev(&xpdev->sgdma_c2h_cdev[i]);
			if (rv < 0)
				pr_err("Failed to destroy c2h xcdev %d error 0x%x\n",
						i, rv);
		}
	}

	if (xpdev_flag_test(xpdev, XDF_CDEV_EVENT)) {
		for (i = 0; i < xpdev->user_max; i++) {
			rv = destroy_xcdev(&xpdev->events_cdev[i]);
			if (rv < 0)
				pr_err("Failed to destroy cdev event %d error 0x%x\n",
					i, rv);
		}
	}

	/* remove user character device */
	if (xpdev_flag_test(xpdev, XDF_CDEV_USER)) {
		for (i = 0; i < dw->user_bar_cnt; i++)	{
			rv = destroy_xcdev(&xpdev->user_cdev[i]);
			if (rv < 0)
				pr_err("Failed to destroy user cdev %d error 0x%x\n",
					i, rv);
		}
	}

	if (xpdev->major)
		unregister_chrdev_region(
				MKDEV(xpdev->major, DX_DMA_MINOR_BASE),
				DX_DMA_MINOR_COUNT);

	dbg_sg("xpdev_destroy_interfaces success!!\n");
}

static void xpdev_free(struct dx_dma_pci_dev *xpdev)
{
	xpdev_destroy_interfaces(xpdev);
	xpdev_cnt--;
	pr_info("xpdev 0x%p, cnt:%d dx_dma_device_close.\n", xpdev, xpdev_cnt);
	kfree(xpdev);
}

static struct dx_dma_pci_dev *xpdev_alloc(struct pci_dev *pdev, struct dw_edma *dw)
{
	struct dx_dma_pci_dev *xpdev = kmalloc(sizeof(*xpdev), GFP_KERNEL);

	if (!xpdev)
		return NULL;
	memset(xpdev, 0, sizeof(*xpdev));

	xpdev->magic = MAGIC_DEVICE;
	xpdev->pdev = pdev;
	xpdev->user_max = 16;
	xpdev->h2c_channel_max = dw->rd_ch_cnt;
	xpdev->c2h_channel_max = dw->wr_ch_cnt;

	xpdev_cnt++;
	dbg_sg("xpedv allocation success(p:%p, cnt:%d\n", xpdev, xpdev_cnt);

	return xpdev;
}

int xpdev_create_interfaces(struct dw_edma_chip *chip)
{
	struct dx_dma_pci_dev *xpdev = NULL;
	struct pci_dev *pdev = chip->dw->pdev;
#ifdef DX_DEBUG_FS
	int rv = 0;
	int i;
#endif

	xpdev = xpdev_alloc(pdev, chip->dw);
	if (!xpdev)
		return -ENOMEM;
	xpdev->dw = chip->dw;
	chip->dw->xpdev = xpdev;

#ifdef DX_DEBUG_FS
	/* initialize events character device */
	// for (i = 0; i < xpdev->user_max; i++) {
	// 	rv = create_xcdev(xpdev, &xpdev->events_cdev[i], 0, CHAR_EVENTS, i);
	// 	if (rv < 0) {
	// 		pr_err("create char event %d failed, %d.\n", i, rv);
	// 		goto fail;
	// 	}
	// }
	// xpdev_flag_set(xpdev, XDF_CDEV_EVENT);

	/* iterate over channels */
	for (i = 0; i < xpdev->h2c_channel_max; i++) {
		rv = create_xcdev(xpdev, &xpdev->sgdma_h2c_cdev[i], 0, CHAR_XDMA_H2C, i);
		if (rv < 0) {
			pr_err("create char h2c %d failed, %d.\n", i, rv);
			goto fail;
		}
	}

	for (i = 0; i < xpdev->c2h_channel_max; i++) {
		rv = create_xcdev(xpdev, &xpdev->sgdma_c2h_cdev[i], 0, CHAR_XDMA_C2H, i);
		if (rv < 0) {
			pr_err("create char c2h %d failed, %d.\n", i, rv);
			goto fail;
		}
	}
	xpdev_flag_set(xpdev, XDF_CDEV_SG);

	/* initialize user character device */
	user_cnt = 0;
	for (i = 0; i < chip->dw->user_bar_cnt; i++)	{
		rv = create_xcdev(xpdev, &xpdev->user_cdev[i], chip->dw->npu_region[i].bar_num, CHAR_USER, i);
		if (rv < 0) {
			pr_err("create_char_%d (user_cdev) failed\n", i);
			goto fail;
		}
	}
	xpdev_flag_set(xpdev, XDF_CDEV_USER);
#endif
	dbg_sg("%s dx_dma%d, pdev 0x%p, xpdev 0x%p, dw 0x%p, usr %d, ch %d,%d.\n",
		dev_name(&pdev->dev), chip->dw->idx, pdev, xpdev, chip->dw,
		xpdev->user_max, xpdev->h2c_channel_max,
		xpdev->c2h_channel_max);

	return 0;

#ifdef DX_DEBUG_FS
fail:
	rv = -1;
	xpdev_destroy_interfaces(xpdev);
	return rv;
#endif
}
EXPORT_SYMBOL_GPL(xpdev_create_interfaces);

void xpdev_release_interfaces(struct dx_dma_pci_dev *xpdev)
{
	if (!xpdev)
		return;

	dbg_sg("Release - xpdev 0x%p.\n", xpdev);
	xpdev_free(xpdev);
}
EXPORT_SYMBOL_GPL(xpdev_release_interfaces);

int dx_cdev_init(void)
{
	g_edma_class = class_create(THIS_MODULE, DX_DMA_NODE_NAME);
	if (IS_ERR(g_edma_class)) {
		pr_err(DX_DMA_NODE_NAME ": failed to create class");
		return -EINVAL;
	}

	/* using kmem_cache_create to enable sequential cleanup */
	cdev_cache = kmem_cache_create("cdev_cache",
					sizeof(struct cdev_async_io), 0,
					SLAB_HWCACHE_ALIGN, NULL);

	if (!cdev_cache) {
		pr_info("memory allocation for cdev_cache failed. OOM\n");
		return -ENOMEM;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(dx_cdev_init);

void dx_cdev_cleanup(void)
{
	dbg_sg("%s: cdev_cache:%p, g_edma_class:%p\n", 
		__func__, cdev_cache, g_edma_class);

	if (cdev_cache)
		kmem_cache_destroy(cdev_cache);

	if (g_edma_class)
		class_destroy(g_edma_class);
}
EXPORT_SYMBOL_GPL(dx_cdev_cleanup);