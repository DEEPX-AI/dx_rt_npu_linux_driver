#!/bin/bash
#
# install_utils.sh - Installation and uninstallation utilities
#
# This script provides functions for installing and uninstalling kernel modules,
# modprobe configurations, udev rules, and DKMS packages.

# Source common utilities if not already loaded
if [ -z "$(type -t logmsg)" ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    source "${SCRIPT_DIR}/common.sh"
fi

# ============================================
# Root Permission Check
# ============================================

# Check if running with root privileges
# Exits with error if not running as root
function check_root_permission() {
    if [[ ${EUID} -ne 0 ]]; then
        logerr "\n==================================================================="
        logerr "ERROR: This operation requires root privileges."
        logerr "==================================================================="
        logmsg ""
        logmsg "Please run with sudo:"
        logmsg "  sudo $0 $*"
        logmsg ""
        exit 1
    fi
}

# ============================================
# DKMS Package Conflict Check
# ============================================

# Check if DKMS package is installed and conflict with manual installation
# Exits with error if DKMS package is detected
function check_dkms_package_conflict() {
    if dpkg -l 2>/dev/null | grep -q "^ii.*dxrt-driver-dkms"; then
        logerr "\n==================================================================="
        logerr "ERROR: DKMS package 'dxrt-driver-dkms' is already installed."
        logerr "==================================================================="
        logmsg ""
        logmsg "This will conflict with manual installation."
        logmsg ""
        logmsg "Please remove the DKMS package first:"
        logmsg "  sudo ./build.sh -c uninstall-package"
        logmsg ""
        logmsg "Or use the package installation method:"
        logmsg "  ./build.sh -c install-package"
        logmsg ""
        exit 1
    fi
}

# Check if manual installation exists and conflict with DKMS package
# Exits with error if manual installation is detected
function check_manual_install_conflict() {
    local kernel_version="$(uname -r)"
    local modules_base="/lib/modules/${kernel_version}"
    
    # Directories where manual installation might exist (excluding DKMS)
    local check_dirs=(
        "${modules_base}/extra"
        "${modules_base}/updates"
    )
    
    # Check for manually installed modules by searching filesystem
    local found_modules=()
    
    for dir in "${check_dirs[@]}"; do
        if [[ ! -d "${dir}" ]]; then
            continue
        fi
        
        # Skip DKMS directory
        if [[ "${dir}" == *"/updates" ]] && [[ -d "${dir}/dkms" ]]; then
            # Check updates/ but exclude updates/dkms/
            while IFS= read -r -d '' module; do
                # Exclude files in dkms subdirectory
                if [[ "${module}" != *"/dkms/"* ]]; then
                    found_modules+=("${module}")
                fi
            done < <(find "${dir}" -name "dx_dma.ko" -o -name "dxrt_driver.ko" -type f -print0 2>/dev/null)
        else
            # For extra/ directory, check all files
            while IFS= read -r -d '' module; do
                found_modules+=("${module}")
            done < <(find "${dir}" -name "dx_dma.ko" -o -name "dxrt_driver.ko" -type f -print0 2>/dev/null)
        fi
    done
    
    # If any modules found, report error
    if [[ ${#found_modules[@]} -gt 0 ]]; then
        logerr "\n==================================================================="
        logerr "ERROR: Manual installation detected in /lib/modules/"
        logerr "==================================================================="
        logmsg ""
        logmsg "This will conflict with DKMS package installation."
        logmsg ""
        logmsg "Detected modules in:"
        for module in "${found_modules[@]}"; do
            logmsg "  ${module}"
        done
        logmsg ""
        logmsg "Please uninstall manually installed modules first:"
        logmsg "  sudo ./build.sh -c uninstall"
        logmsg ""
        logmsg "Then install via DKMS package:"
        logmsg "  sudo ./build.sh -c install-package"
        logmsg ""
        exit 1
    fi
}

# ============================================
# Module Uninstallation
# ============================================

# Uninstalls kernel modules from the system
# Removes module files from installation directories and updates module dependencies
# Global variables used:
#   SUPPORT_PCIE_MODULE - Associative array of PCIE modules
#   PCIE_MODPROBE_CONF - Associative array of modprobe configurations
#   MOD_INSTALL_DIR - Primary module installation directory
#   MOD_UPDATES_DIR - Alternate module installation directory
#   MOD_MODPROBE_CONF - Modprobe configuration directory
#   MOD_MODPROBE_DEP - Module dependency file
function uninstall_modules() {
    local mods=(${SUPPORT_PCIE_MODULE[*]} "rt")
    local install_dirs=(${MOD_INSTALL_DIR} ${MOD_UPDATES_DIR})

    for base_dir in "${install_dirs[@]}"; do
        [[ -d "${base_dir}" ]] || continue
        logmsg "\n *** Remove : ${base_dir} ***"
        for i in ${mods[*]}; do
            mod=$(basename ${i})
            target_dir="${base_dir}/${mod}"
            target_file="${base_dir}/${mod}.ko"

            if [[ -d "${target_dir}" ]]; then
                logmsg " $ sudo rm -rf ${target_dir}"
                sudo rm -rf "${target_dir}"
            fi

            if [[ -f "${target_file}" ]]; then
                logmsg " $ sudo rm -f ${target_file}"
                sudo rm -f "${target_file}"
            fi
        done
    done

    logmsg "\n *** Remove : ${MOD_MODPROBE_CONF} ***"
    for i in ${PCIE_MODPROBE_CONF[*]}; do
        mod=$(basename ${i})
        [[ ! -f "${MOD_MODPROBE_CONF}/${mod}" ]] && continue
        logmsg " $ sudo rm ${MOD_MODPROBE_CONF}/${mod}"
        sudo rm "${MOD_MODPROBE_CONF}/${mod}"
    done

    logmsg "\n *** Update : ${MOD_MODPROBE_DEP} ***"
    logmsg " $ sudo depmod"
    if ! eval sudo depmod; then
        logext " - FAILED"
    fi
}

# ============================================
# Module Installation Helpers
# ============================================

# Updates module dependencies using depmod
function update_depmod() {
    logmsg "\n *** Update : ${MOD_MODPROBE_DEP} ***"
    logmsg " $ sudo depmod -A"
    if ! eval sudo depmod -A; then
        logext " - FAILED"
    fi
}

# Installs modprobe configuration file
# Arguments:
#   $1 - modconf: Path to the modprobe configuration file to install
function install_modprobe_conf() {
    local modconf=$1
    if [[ -n ${modconf} ]]; then
        logmsg " $ sudo cp ${modconf} /etc/modprobe.d/"
        if ! eval sudo cp ${modconf} /etc/modprobe.d/; then
            logext " - FAILED"
        fi
    fi
}
