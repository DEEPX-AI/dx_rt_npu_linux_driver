#!/bin/bash
#
# dependency_checker.sh - Dependency checking functions
#
# This script provides functions to check and ensure system dependencies
# are met before building or loading kernel modules.

# Source common utilities if not already loaded
if [ -z "$(type -t logmsg)" ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    source "${SCRIPT_DIR}/common.sh"
fi

# ============================================
# Virtual DMA Dependency
# ============================================

# Ensures that the virtual DMA dependency (CONFIG_DMA_VIRTUAL_CHANNELS) is satisfied.
# This can be built into the kernel (=y), available as a module (=m), or bundled with our driver.
# 
# Environment variables:
#   SKIP_VDMA_CHECK - Set to 1 to bypass this check (not recommended for production)
#   VDMA_CORE_NAME - Preferred module name (default: virt_dma)
#
# The check follows this logic:
#   1. If CONFIG_DMA_VIRTUAL_CHANNELS=y (built-in), we're done
#   2. If CONFIG_DMA_VIRTUAL_CHANNELS=m (module), ensure it's loaded
#   3. Otherwise, expect bundled virt-dma.c source to be compiled into our driver
function ensure_virtual_dma_dependency() {
    [[ "$SKIP_VDMA_CHECK" == "1" ]] && { logmsg "-> Skip virtual DMA check (SKIP_VDMA_CHECK=1)"; return 0; }

    local kcfg="/boot/config-$(uname -r)"
    local cfg_builtin=0 cfg_mod=0
    if [[ -f "$kcfg" ]]; then
        grep -Eq '^CONFIG_DMA_VIRTUAL_CHANNELS=y' "$kcfg" && cfg_builtin=1
        grep -Eq '^CONFIG_DMA_VIRTUAL_CHANNELS=m' "$kcfg" && cfg_mod=1
    else
        logmsg "-> Kernel config file not found ($kcfg). Attempting heuristic detection."
    fi

    if [[ $cfg_builtin -eq 1 ]]; then
        logmsg "-> CONFIG_DMA_VIRTUAL_CHANNELS=y (built-in)"
        return 0
    fi

    if [[ $cfg_mod -eq 1 ]]; then
        # Module case: need virt-dma.ko loaded before insmod of our driver modules (since insmod doesn't resolve deps).
        # Typical module filename is virt-dma.ko but lsmod shows 'virt_dma'. Support both name forms.
        if lsmod | grep -Eq '^virt_dma\b'; then
            logmsg "-> virt_dma module already loaded"
            return 0
        fi
        logmsg "-> Loading virt-dma module (CONFIG_DMA_VIRTUAL_CHANNELS=m)"
        if ! sudo modprobe virt-dma 2>/dev/null; then
            # Try alternative underscore name
            if ! sudo modprobe virt_dma 2>/dev/null; then
                # Last resort: locate file and insmod
                local vpath
                vpath=$(find /lib/modules/$(uname -r) -type f -name 'virt-dma.ko' -o -name 'virt_dma.ko' 2>/dev/null | head -n1 || true)
                if [[ -n "$vpath" ]]; then
                    if ! sudo insmod "$vpath"; then
                        logerr "-> Failed to load virt-dma module (path: $vpath)"; exit 1; fi
                else
                    logerr "-> Could not find virt-dma.ko (CONFIG_DMA_VIRTUAL_CHANNELS=m). Run 'sudo depmod -A' then retry."; exit 1
                fi
            fi
        fi
        if lsmod | grep -Eq '^virt_dma\b'; then
            logmsg "-> virt_dma module loaded"
            return 0
        else
            logerr "-> virt_dma module not visible in lsmod after load attempt"; exit 1
        fi
    fi

    # Neither builtin nor modular -> expect bundled source to be compiled into our driver.
    if [[ -f "pci_deepx/virt-dma.c" ]]; then
        logmsg "-> CONFIG_DMA_VIRTUAL_CHANNELS unset; using bundled virt-dma.c"
    else
        logerr "-> Missing both kernel CONFIG_DMA_VIRTUAL_CHANNELS and bundled pci_deepx/virt-dma.c"
        exit 1
    fi
}
