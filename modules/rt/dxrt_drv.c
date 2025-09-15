// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/mutex.h>

#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include "dxrt_drv.h"
#include "dxrt_version.h"

static struct dxrt_driver drv;

#if IS_ACCELERATOR
static int __init dxrt_driver_init(void)
{
    return dxrt_driver_cdev_init(&drv);
}

static void __exit dxrt_driver_exit(void)
{
    dxrt_driver_cdev_deinit(&drv);
}

module_init(dxrt_driver_init);
module_exit(dxrt_driver_exit);
#endif

#if IS_STANDALONE
static int dxrt_driver_probe(struct platform_device *pdev)
{
	int ret = 0;
    pr_debug( "%s\n", __func__);

	platform_set_drvdata(pdev, &drv);
	drv.pdev = pdev;

    ret = dxrt_driver_cdev_init(&drv);
    pr_info( "%s done: %d\n", __func__, ret);
    return ret;
}
static int dxrt_driver_remove(struct platform_device *pdev)
{
    pr_debug( "%s\n", __func__);
    dxrt_driver_cdev_deinit(&drv);
    pr_info( "%s done.\n", __func__);
    return 0;
}

static const struct of_device_id deepx_npu_of_match[] = {
	{ .compatible = "deepx,npu" },
	{ },
};

MODULE_DEVICE_TABLE(of, deepx_npu_of_match);

static struct platform_driver dxrt_drv = {
	.probe = dxrt_driver_probe,
	.remove = dxrt_driver_remove,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = deepx_npu_of_match,
	},
};
module_platform_driver(dxrt_drv);
#endif

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Taegyun An <atg@deepx.ai>");
MODULE_DESCRIPTION("Deepx Runtime Driver");
MODULE_VERSION(DXRT_MODULE_VERSION);