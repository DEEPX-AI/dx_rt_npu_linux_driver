#!/bin/bash
#
# BUILD_DEFAULT_DEVICE=<DEVICE>                  : -d [device]
# BUILD_DEFAULT_KERNEL_DIR=<KDIR>                  : -k [kernel dir], The directory where the kernel source is located
#                                                      default : /lib/modules/$(uname -r)/build)
# BUILD_DEFAULT_ARCH=<ARCH>                      : -a [arch], Target CPU architecture for cross-compilation, default : `uname -m`
# BUILD_DEFAULT_CROSS_COMPILE=<CROSS_COMPILE>  : -t [cross tool], cross compiler binary, e.g aarch64-linux-gnu-
# BUILD_DEFAULT_INSTALL_DIR=<INSTALL_MOD_PATH> : -i [install dir], module install directory
#

# ============================================
# Import common scripts
# ============================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/scripts/common.sh"
source "${SCRIPT_DIR}/scripts/dependency_checker.sh"
source "${SCRIPT_DIR}/scripts/service_manager.sh"
source "${SCRIPT_DIR}/scripts/module_manager.sh"
source "${SCRIPT_DIR}/scripts/install_utils.sh"
source "${SCRIPT_DIR}/scripts/debian_builder.sh"

# ============================================
# Build Configuration
# ============================================
BUILD_DEFAULT_DEVICE="m1"
BUILD_DEFAULT_PCIE="deepx"
BUILD_DEFAULT_KERNEL_DIR=""
BUILD_DEFAULT_ARCH=""
BUILD_DEFAULT_CROSS_COMPILE=""
BUILD_DEFAULT_INSTALL_DIR=""
BUILD_DEFAULT_DBG=""
RELOAD_AFTER_INSTALL=0

# ============================================
# Usage and Argument Parsing
# ============================================

function build_usage() {
    echo " Usage:"
    echo -e "\t$(basename "${0}") <options>"
    echo ""
    echo " options:"
    echo -e "\t-d, --device   [device]      select target device: ${SUPPORT_DEVICE[*]}"
    echo -e "\t-m, --module   [module]      select PCIe module: deepx"
    echo -e "\t-k, --kernel   [kernel dir]  'KERNEL_DIR=[kernel dir]', The directory where the kernel source is located"
    echo -e "\t                             - default: /lib/modules/$(uname -r)/build)"
    echo -e "\t-a, --arch     [arch]        set 'ARCH=[arch]' Target CPU architecture for cross-compilation, default: $(uname -m)"
    echo -e "\t-t, --compiler [cross tool]  'CROSS_COMPILE=[cross tool]' cross compiler binary, e.g aarch64-linux-gnu-"
    echo -e "\t-i, --install  [install dir] 'INSTALL_MOD_PATH=[install dir]', module install directory"
    echo -e "\t                             - install to: [install dir]/lib/modules/[KERNELRELEASE]/extra/"
    echo -e "\t-c, --command  [command]     clean | install | uninstall | debian-package | install-package | uninstall-package"
    echo -e "\t                             - install: Install modules to system (requires sudo)"
    echo -e "\t                             - uninstall: Remove installed modules (requires sudo)"
    echo -e "\t                             - debian-package: Build Debian package (.deb)"
    echo -e "\t                             - install-package: Install Debian package (requires sudo)"
    echo -e "\t                             - uninstall-package: Uninstall Debian package (requires sudo)"
    echo -e "\t-p, --pkg-version [version]  Specify package version for install-package (default: latest)"
    echo -e "\t-j, --jops     [jobs]        set build jobs"
    echo -e "\t-f, --debug    [debug]       set debug feature [debugfs | log | all]"
    echo -e "\t-v, --verbose                build verbose (V=1)"
    echo -e "\t    --reload                 reload drivers after install (requires sudo)"
    echo -e "\t-h, --help                   show this help"
    echo ""
    echo " Examples:"
    echo -e "\t./build.sh                              # Build modules"
    echo -e "\tsudo ./build.sh -c install              # Install to system"
    echo -e "\t./build.sh -c debian-package            # Build .deb package"
    echo -e "\tsudo ./build.sh -c install-package      # Install .deb package"
    echo ""
}

_device=""
_module=""
_destdir="${BUILD_DEFAULT_INSTALL_DIR}"
_command=""
_kerndir="${BUILD_DEFAULT_KERNEL_DIR}"
_arch="${BUILD_DEFAULT_ARCH}"
_compiler="${BUILD_DEFAULT_CROSS_COMPILE}"
_verbose=""
_jops="-j$(grep -c processor /proc/cpuinfo)"
_debug=""
_modconf=""
_pkg_version=""
_args=()

function parse_args() {
    while [ "${1:-}" != "" ]; do
    case "$1" in
        -d | --device)   _device=${2}; shift 2 ;;
        -m | --module)   _module="${2}"; shift 2 ;;
        -k | --kernel)   _kerndir="${2}"; shift 2 ;;
        -a | --arch)     _arch="${2}"; shift 2 ;;
        -t | --compiler) _compiler="${2}"; shift 2 ;;
        -i | --install)  _destdir="${2}"; shift 2 ;;
        -c | --command)  _command="${2}"; shift 2 ;;
        -p | --pkg-version) _pkg_version="${2}"; shift 2 ;;
        -j | --jops)     _jops="-j${2}"; shift 2 ;;
        -v | --verbose)  _verbose="V=1"; shift ;;
        -f | --debug)    _debug="${2}"; shift 2 ;;
        --reload)        RELOAD_AFTER_INSTALL=1; shift ;;
        -h | --help)
            build_usage
            exit 0
            ;;
        *) exit 1 ;;
        esac
    done

    if [[ -n ${_device} ]]; then
        if ! echo "${SUPPORT_DEVICE[*]}" | grep -qw "${_device}"; then
            logext "Not support device: ${_device} !"
        fi
    fi

    if [[ -n ${_module} ]]; then
        if ! echo "${!SUPPORT_PCIE_MODULE[*]}" | grep -qw "${_module}"; then
            logext "Not found module: ${_module} !"
        fi
    fi

    if [[ -n ${_kerndir} && ! -d ${_kerndir} ]]; then
        logext "Not found kernel: ${_kerndir} !"
    fi
}

# ============================================
# Argument Setup and Display
# ============================================

function setup_args() {
    # Build only module driver
    if [[ -n ${_module} ]]; then
        if [[ -z ${_device} ]]; then
            _args=("-f $(realpath "${SUPPORT_PCIE_MODULE[${_module}]}")/Makefile")
        fi
    fi

    # set default build target
    if [[ -z ${_args[*]} ]]; then
        if [[ -z ${_device} ]]; then
            _device=${BUILD_DEFAULT_DEVICE}
        fi
        if [[ -z ${_module} ]]; then
            if echo "${PCIE_DEPEND_DEVICE[*]}" | grep -qw "${_device}"; then
                _module="${BUILD_DEFAULT_PCIE}"
            fi
        fi
    fi

    [[ -n ${_device} ]] && _args+=("DEVICE=${_device}")
    [[ -n ${_device} && -n ${_module} ]] && _args+=("PCIE=${_module}")
    [[ -n ${_kerndir} ]] && _args+=("KERNEL_DIR=$(realpath "${_kerndir}")")
    [[ -n ${_compiler} ]] && _args+=("CROSS_COMPILE=${_compiler}")
    [[ -n ${_destdir} ]] && _args+=("INSTALL_MOD_PATH=$(realpath "${_destdir}")")
    [[ -n ${_command} ]] && _args+=("${_command}")
    [[ -n ${_verbose} ]] && _args+=("${_verbose}")
    [[ -z ${_command} && -n ${_jops} ]] && _args+=("${_jops}")
    [[ -n ${_debug} ]] && _args+=("BUILD_DEFAULT_DBG=${_debug}")
    [[ -n ${_device} && -n ${_module} ]] && _modconf=${PCIE_MODPROBE_CONF[${_module}]}

    if [[ -n ${_arch} ]]; then
        # Fix 64bits architecture
        if [[ ${_arch} == "x86_64" ]]; then
            _args+=("ARCH=x86")
        elif [[ ${_arch} == "riscv64" ]]; then
            _args+=("ARCH=riscv")
        else
            _args+=("ARCH=${_arch}")
        fi
    fi
}

function print_args() {
    if [[ -n ${_device} ]]; then
        logmsg "- DEVICE          : ${_device}"
    fi
    if [[ -n ${_module} ]]; then
        logmsg "- PCIE            : ${_module}"
    fi
    if [[ -n ${_device} && -n ${_module} ]]; then
        logmsg "- MODULE CONF     : ${_modconf}"
    fi
    if [[ -n ${_arch} ]]; then
        logmsg "- ARCH            : ${_arch}"
    else
        logmsg "- ARCH (HOST)     : $(uname -m)"
    fi
    if [[ -n ${_compiler} ]]; then
        logmsg "- CROSS_COMPILE   : ${_compiler}"
    fi
    if [[ -n ${_kerndir} ]]; then
        logmsg "- KERNEL_DIR      : ${_kerndir}"
    else
        logmsg "- KERNEL_DIR      : /lib/modules/$(uname -r)/build"
    fi
    if [[ -n ${_destdir} ]]; then
        logmsg "- INSTALL PATH    : ${_destdir}/lib/modules/[KERNELRELEASE]/extra/"
    else
        logmsg "- INSTALL PATH    : /lib/modules/$(uname -r)/extra/"
    fi
    if [[ -n ${_debug} ]]; then
        logmsg "- DEBUG           : ${_debug}"
    fi
    if [[ ${RELOAD_AFTER_INSTALL} -eq 1 ]]; then
        logmsg "- RELOAD          : enabled (--reload)"
    fi
}

# ============================================
# Helper Functions
# ============================================

# Reload drivers after build or install
# Uses module_manager.sh functions for complete reload process
function reload_drivers() {
    logmsg "\n *** Reload drivers ***"
    
    # Get module names for preparation
    pcie_module_ko=$(check_module_file "${SUPPORT_PCIE_MODULE[${_module}]}")
    pcie_module_name=$(basename "$pcie_module_ko" .ko)
    rt_module_ko=$(check_module_file "rt")
    rt_module_name=$(basename "$rt_module_ko" .ko)
    
    # Prepare system for module reload (stop service, kill processes)
    prepare_for_module_reload "${pcie_module_name}" "${rt_module_name}"
    
    # Perform the actual module reload
    reload_drivers_forcefully
}

# ============================================
# Main Execution
# ============================================

parse_args "${@}"
setup_args
print_args

# Check for cross-compilation conflicts with install/package commands
if [[ -n ${_compiler} || -n ${_arch} ]]; then
    # Cross-compilation detected
    if [[ ${_command} == "install" || ${_command} == "uninstall" ]]; then
        logerr "\n==================================================================="
        logerr "ERROR: Cannot ${_command} modules when cross-compiling"
        logerr "==================================================================="
        logmsg ""
        logmsg "Cross-compilation detected:"
        [[ -n ${_arch} ]] && logmsg "  ARCH: ${_arch}"
        [[ -n ${_compiler} ]] && logmsg "  CROSS_COMPILE: ${_compiler}"
        logmsg ""
        logmsg "Install/uninstall operations require native compilation."
        logmsg "For cross-compiled modules:"
        logmsg "  1. Build for target: ./build.sh -a <arch> -t <toolchain>"
        logmsg "  2. Copy modules to target system"
        logmsg "  3. Install on target: ./build.sh -c install"
        logmsg ""
        exit 1
    fi
    
    if [[ ${_command} == "install-package" || ${_command} == "uninstall-package" ]]; then
        logerr "\n==================================================================="
        logerr "ERROR: Cannot ${_command} when cross-compiling"
        logerr "==================================================================="
        logmsg ""
        logmsg "Debian package install/uninstall requires native build."
        logmsg ""
        logmsg "Note: You can build debian package with cross-compilation,"
        logmsg "      but installation must be done on the target system."
        logmsg ""
        exit 1
    fi
    
    if [[ ${RELOAD_AFTER_INSTALL} -eq 1 ]]; then
        logerr "\n==================================================================="
        logerr "ERROR: --reload option not supported for cross-compilation"
        logerr "==================================================================="
        logmsg ""
        logmsg "Module reload requires native build."
        logmsg ""
        exit 1
    fi
fi

# Build Debian package
if [[ ${_command} == "debian-package" ]]; then
    build_debian_package
    exit $?
fi

# Install Debian package
if [[ ${_command} == "install-package" ]]; then
    check_root_permission "$@"
    install_debian_package "${_pkg_version}"
    exit $?
fi

# Uninstall Debian package
if [[ ${_command} == "uninstall-package" ]]; then
    check_root_permission "$@"
    uninstall_debian_package
    exit $?
fi

# uninstall modules in host PC
if [[ ${_command} == "uninstall" ]]; then
    check_root_permission "$@"
    uninstall_modules
    exit 0
fi

# reload only (without build/install)
if [[ ${RELOAD_AFTER_INSTALL} -eq 1 && -z ${_command} ]]; then
    check_root_permission "$@"
    reload_drivers
    exit 0
fi

# make build
logmsg "\n *** Build : ${_command} ***"
if [[ ${_command} == "sparse" ]]; then
    logmsg " $ make sparse ${_args[*]}\n"
    if ! eval make sparse "${_args[*]}"; then
        logext " - FAILED"
    else
        logmsg " - SUCCESS"
    fi
else
    logmsg " $ make ${_args[*]}\n"
    if ! eval make "${_args[*]}"; then
        logext " - FAILED"
    else
        logmsg " - SUCCESS"
    fi
fi

# install
if [[ ${_command} == "install" ]]; then
    # Check root permission
    check_root_permission "$@"
    
    # Check for DKMS package conflict
    check_dkms_package_conflict
    
    update_depmod
    install_modprobe_conf "${_modconf}"

    # Only perform reload for local builds (not cross-compiling)
    if [[ -z ${_compiler} ]]; then
        if [[ ${RELOAD_AFTER_INSTALL} -eq 1 ]]; then
            reload_drivers
        else
            logmsg "-> Module installed. Reboot or manually load modules to activate."
            logmsg "   To load now: modprobe dx_dma && modprobe dxrt_driver"
        fi
    fi
fi