// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#ifndef __DXRT_VERSION_H__
#define __DXRT_VERSION_H__

#define DXRT_MOD_MAJOR		1
#define DXRT_MOD_MINOR		0
#define DXRT_MOD_PATCHLEVEL	0

#define DXRT_MODULE_VERSION         \
    __stringify(DXRT_MOD_MAJOR) "." \
    __stringify(DXRT_MOD_MINOR) "." \
    __stringify(DXRT_MOD_PATCHLEVEL)

#define DXRT_MOD_VERSION_NUMBER  \
    ((DXRT_MOD_MAJOR)*1000 + (DXRT_MOD_MINOR)*100 + DXRT_MOD_PATCHLEVEL)

struct dxrt_drv_info {
    unsigned int driver_version;
};

#endif /* ifndef __DXRT_VERSION_H__ */

