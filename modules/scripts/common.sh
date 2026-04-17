#!/bin/bash
#
# common.sh - Common utilities and constants
#
# This script provides logging functions and common constants used across
# the driver build and installation scripts.

# ============================================
# Logging Functions
# ============================================

function logerr() {
    echo -e "\033[1;31m$*\033[0m"
}

function logmsg() {
    echo -e "\033[0;33m$*\033[0m"
}

function logext() {
    echo -e "\033[1;31m$*\033[0m"
    exit 1
}

# ============================================
# Common Constants
# ============================================

# Module installation paths
MOD_MODPROBE_DEP="/lib/modules/$(uname -r)/modules.dep"
MOD_MODPROBE_CONF="/etc/modprobe.d"
MOD_INSTALL_DIR="/lib/modules/$(uname -r)/extra"
MOD_UPDATES_DIR="/lib/modules/$(uname -r)/updates"

# Supported devices and modules
SUPPORT_DEVICE=("m1" "v3")

declare -g -A SUPPORT_PCIE_MODULE=(
    ["deepx"]="$(pwd)/pci_deepx"
    ["xilinx"]="$(pwd)/pci_xilinx"
)

PCIE_DEPEND_DEVICE=("m1")

declare -g -A PCIE_MODPROBE_CONF=(
    ["deepx"]="$(pwd)/dx_dma.conf"
    ["xilinx"]="$(pwd)/xdma.conf"
)

# ============================================
# Global State Variables
# ============================================

# Service management
DXRT_SERVICE_WAS_ACTIVE=0
STOP_TIMEOUT_SEC=${STOP_TIMEOUT_SEC:-1}
START_TIMEOUT_SEC=${START_TIMEOUT_SEC:-1}
SERVICE_DEBUG_JOURNAL_LINES=${SERVICE_DEBUG_JOURNAL_LINES:-60}

# Virtual DMA dependency configuration
VDMA_CORE_NAME=${VDMA_CORE_NAME:-virt_dma}
SKIP_VDMA_CHECK=${SKIP_VDMA_CHECK:-0}
