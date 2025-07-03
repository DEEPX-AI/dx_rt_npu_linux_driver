// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2023 DeepX, Inc. and/or its affiliates.
 * DeepX eDMA PCIe driver
 *
 * Author: Taegyun An <atg@deepx.ai>
 */

#ifndef DX_LIB_H
#define DX_LIB_H

#include <linux/types.h>
#include <linux/version.h>

#define ACCESS_OK_2_ARGS (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))

#ifdef DX_DEBUG_ENABLE
#define dbg_core    pr_err
#define dbg_fops	pr_err
#define dbg_perf	pr_err
#define dbg_sg		pr_err
#define dbg_tfr		pr_err
#define dbg_irq		pr_err
#define dbg_init	pr_err
#define dbg_desc	pr_err
#define dbg_msg 	pr_err
#else
/* disable debugging */
#define dbg_core(...)
#define dbg_fops(...)
#define dbg_perf(...)
#define dbg_sg(...)
#define dbg_tfr(...)
#define dbg_irq(...)
#define dbg_init(...)
#define dbg_desc(...)
#define dbg_msg(...)
#endif

#endif /* DX_LIB_H */
