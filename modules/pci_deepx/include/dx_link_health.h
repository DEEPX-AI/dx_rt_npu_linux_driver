/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022-2026 DeepX, Inc. and/or its affiliates.
 * PCIe link health monitoring and auto-recovery.
 */
#ifndef _DX_LINK_HEALTH_H
#define _DX_LINK_HEALTH_H

#include "dw-edma-core.h"

/* --- Link health detection --- */

enum dx_link_state dx_pcie_check_link_health(struct dw_edma *dw);
int dx_pcie_validate_link(struct dw_edma *dw);

/* --- Quiesce / Recovery --- */

void dx_dma_quiesce_for_link_down(struct dw_edma *dw);
int dx_dma_full_reinit(struct dw_edma *dw);

/* --- Lifecycle (called from probe/remove) --- */

void dx_link_health_init(struct dw_edma *dw);
void dx_link_health_start(struct dw_edma *dw);
void dx_link_health_stop(struct dw_edma *dw);
void dx_link_health_set_enabled(struct dw_edma *dw, bool enable);
bool dx_link_health_is_enabled(struct dw_edma *dw);
void dx_link_health_set_default_enabled(bool enable);
bool dx_link_health_default_enabled(void);

#endif /* _DX_LINK_HEALTH_H */
