// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#ifndef __DXRT_VERSION_H__
#define __DXRT_VERSION_H__

#define DXRT_MODULE_VERSION         \
    __stringify(RT_VERSION_MAJOR) "." \
    __stringify(RT_VERSION_MINOR) "." \
    __stringify(RT_VERSION_PATCH)

#define DXRT_MOD_VERSION_NUMBER  \
    ((RT_VERSION_MAJOR)*1000 + (RT_VERSION_MINOR)*100 + RT_VERSION_PATCH)

struct dxrt_drv_info {
    unsigned int driver_version;
};

#endif /* ifndef __DXRT_VERSION_H__ */

