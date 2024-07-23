# SPDX-License-Identifier: GPL-2.0
# Makefile for DeepX AI accelerators config
#

export CONFIG_DX_AI_ACCEL_RT=m

ifeq ($(findstring m1,$(DEVICE)), m1)
ifeq ($(findstring m1a,$(DEVICE)), m1a)
export CONFIG_DX_AI_ACCEL_M1A=y
else
export CONFIG_DX_AI_ACCEL_M1=y
endif
export CONFIG_DX_AI_ACCEL_PCIE_DEEPX=m
    ifeq ($(PCIE),deepx)
    export CONFIG_DX_AI_ACCEL_PCIE_DEEPX=m
    export CONFIG_DX_AI_ACCEL_PCIE_XILINX=n
    else ifeq ($(PCIE),xilinx)
    export CONFIG_DX_AI_ACCEL_PCIE_DEEPX=n
    export CONFIG_DX_AI_ACCEL_PCIE_XILINX=m
    else
    $(error ERROR: Not support sub-module 'make DEVICE=$(DEVICE) PCIE=[deepx|xilinx]')
    endif
else ifeq ($(DEVICE),l1)
export CONFIG_DX_AI_ACCEL_L1=y
else ifeq ($(DEVICE),l3)
export CONFIG_DX_AI_ACCEL_L3=y
else
$(error ERROR: Not support device[$(DEVICE)] 'make DEVICE=[m1|m1a|l1|l3]')
endif
