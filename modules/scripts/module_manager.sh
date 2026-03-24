#!/bin/bash
#
# module_manager.sh - Kernel module management functions
#
# This script provides functions to manage kernel modules including
# loading, unloading, checking status, and handling user processes.

# Source common utilities if not already loaded
if [ -z "$(type -t logmsg)" ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    source "${SCRIPT_DIR}/common.sh"
fi

# Source service manager if not already loaded
if [ -z "$(type -t stop_dxrt_service)" ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    source "${SCRIPT_DIR}/service_manager.sh"
fi

# ============================================
# Module File Management
# ============================================

# Find a single .ko file in the specified directory
# Arguments:
#   $1 - module_path: Directory to search for .ko files
# Returns:
#   Prints the path to the .ko file if exactly one is found, empty string otherwise
function check_module_file() {
    local module_path=$1
    local ko_files=()
    if [ ! -d "$module_path" ]; then        
        echo ""
        return
    fi
    while IFS= read -r -d $'\0' file; do
        ko_files+=("$file")
    done < <(find "$module_path" -type f -name "*.ko" -print0)
    if [ ${#ko_files[@]} -eq 0 ]; then        
        echo ""
    elif [ ${#ko_files[@]} -eq 1 ]; then
        echo "${ko_files[0]}"
    else
        echo ""
    fi
}

# Check if a module is loaded successfully
# Arguments:
#   $1 - module_name: Name of the module to check
# Exits with error if module is not loaded
function check_module_install() {
    local name=$1
    check_module=$(lsmod | grep -wc $name)
    if [[ "$check_module" -ge "1" ]]; then
        logmsg "-> $1 loaded successfully!!"
    else
        logerr "-> Error: Failed to load module $1."
        exit 1
    fi
}

# ============================================
# User Process Management
# ============================================

# Checks for and kills user-space processes using a module's device files.
# It tries 'lsof' first, then falls back to 'fuser', and warns if neither is found.
# Arguments:
#   $1 - module_name: Name of the module whose devices to check
function check_and_kill_user_processes() {
    local module_name="$1"
    logmsg "-> Checking for user processes using '${module_name}'..."

    declare -A device_patterns=(
        ["dx_dma"]="/dev/dx-dma*"
        ["dxrt_driver"]="/dev/dxrt*" 
    )

    local pattern=${device_patterns[$module_name]}
    if [[ -z "$pattern" ]]; then
        logmsg "--> No device pattern defined for '${module_name}'. Skipping user process check."
        return
    fi
    
    local pids=""
    local tool_used=""

    if command -v lsof &> /dev/null; then
        tool_used="lsof"
        pids=$(lsof -t ${pattern} 2>/dev/null | xargs)
    elif command -v fuser &> /dev/null; then
        tool_used="fuser"
        pids=$(fuser ${pattern} 2>/dev/null | xargs)
    else
        logerr "--> WARNING: 'lsof' and 'fuser' command not found."
        logmsg "--> Skipping user process check. The following 'rmmod' may fail if the driver is in use."
        return
    fi

    if [[ -n "$pids" ]]; then
        logerr "--> Found user processes using '${module_name}' (via ${tool_used}):"
        ps -o pid,user,comm -p "${pids// /,}" | sed 's/^/   /'
        logmsg "--> Terminating these processes to allow module reload..."
        
        sudo kill -9 $pids
        sleep 1 # Give time for processes to terminate

        # Verify termination
        local remaining_pids=""
        if [[ "$tool_used" == "lsof" ]]; then
            remaining_pids=$(lsof -t ${pattern} 2>/dev/null | xargs)
        else
            remaining_pids=$(fuser ${pattern} 2>/dev/null | xargs)
        fi

        if [[ -n "$remaining_pids" ]]; then
            logerr "--> FAILED to terminate all processes. Manual intervention may be required."
            exit 1
        else
            logmsg "--> Processes terminated successfully."
        fi
    else
        logmsg "--> No running user processes found for '${module_name}' (via ${tool_used})."
    fi
}

# ============================================
# Driver Reload Preparation
# ============================================

# Prepares the system for module reload by stopping services and terminating user processes
# This should be called before reload_drivers_forcefully if needed
# Arguments:
#   $1 - pcie_module_name: Name of the PCIE module (e.g., "dx_dma")
#   $2 - rt_module_name: Name of the RT module (e.g., "dxrt_driver")
function prepare_for_module_reload() {
    local pcie_module_name="$1"
    local rt_module_name="$2"
    
    logmsg "\n*** Preparing system for module reload ***"
    
    # 1. Stop any running services that use the drivers
    stop_dxrt_service
    
    # 2. Kill any user-space applications using the devices
    # The order is important: check the higher-level module first
    check_and_kill_user_processes "${rt_module_name}"
    
    logmsg "-> System prepared for module reload"
}

# ============================================
# Driver Reload
# ============================================

# Reloads kernel modules by unloading old ones and loading new ones
# This function only handles module unload/load operations
# For a complete reload including service stop and process cleanup, 
# call prepare_for_module_reload() first
#
# Global variables used:
#   _module - Current PCIE module name (e.g., "deepx")
#   SUPPORT_PCIE_MODULE - Associative array mapping module names to paths
function reload_drivers_forcefully() {
    logmsg "\n*** Reloading driver modules ***"

    # Ensure virtual DMA dependency early
    # This function is provided by dependency_checker.sh
    if [ "$(type -t ensure_virtual_dma_dependency)" ]; then
        ensure_virtual_dma_dependency
    fi

    # 1. Find the module names from their directories
    local pcie_module_ko rt_module_ko
    pcie_module_ko=$(check_module_file "${SUPPORT_PCIE_MODULE[${_module}]}")
    pcie_module_name=$(basename "$pcie_module_ko" .ko)
    
    rt_module_ko=$(check_module_file "rt") # Assuming 'rt' is the directory name for the rt driver
    rt_module_name=$(basename "$rt_module_ko" .ko)

    # 2. Unload old modules, in reverse dependency order (dependent module first)
    logmsg "-> Unloading existing modules (if loaded)..."
    if lsmod | grep -qw "${rt_module_name}"; then
        logmsg "--> Removing module '${rt_module_name}'..."
        if ! sudo rmmod "${rt_module_name}"; then
            logerr "--> Failed to remove '${rt_module_name}'. Exiting."
            exit 1
        fi
    fi
    if lsmod | grep -qw "${pcie_module_name}"; then
        logmsg "--> Removing module '${pcie_module_name}'..."
        if ! sudo rmmod "${pcie_module_name}"; then
            logerr "--> Failed to remove '${pcie_module_name}'. Exiting."
            exit 1
        fi
    fi

    # 3. Load new modules, in correct dependency order (base module first)
    logmsg "-> Loading new modules..."
    logmsg "--> Loading module: '${pcie_module_ko}'"
    sudo insmod "${pcie_module_ko}"
    check_module_install "${pcie_module_name}"

    logmsg "--> Loading module: '${rt_module_ko}'"
    sudo insmod "${rt_module_ko}"
    check_module_install "${rt_module_name}"

    logmsg "-> Driver modules reloaded successfully"
}
