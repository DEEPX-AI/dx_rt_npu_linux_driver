// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#ifndef __DXRT_VERSION_H__
#define __DXRT_VERSION_H__

/* Helper macro for version prefix */
#ifndef RT_VERSION_SUFFIX
#define RT_VERSION_SUFFIX ""
#endif

#define DXRT_MODULE_VERSION         \
    __stringify(RT_VERSION_MAJOR) "." \
    __stringify(RT_VERSION_MINOR) "." \
    __stringify(RT_VERSION_PATCH) "-" \
    __stringify(RT_VERSION_SUFFIX)

/* Standard version number (for backward compatibility) */
#define DXRT_MOD_VERSION_NUMBER  \
    ((RT_VERSION_MAJOR)*1000 + (RT_VERSION_MINOR)*100 + RT_VERSION_PATCH)

struct dxrt_drv_info {
    unsigned int driver_version;
};

struct dxrt_drv_info_v2 {
    unsigned int driver_version;
    char driver_version_suffix[16];
    unsigned int reserved[16];
};

/* Version string length */
#define DXRT_VERSION_STRING_MAX_LEN     16

#endif /* ifndef __DXRT_VERSION_H__ */
