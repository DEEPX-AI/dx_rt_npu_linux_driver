#!/bin/bash
#
# BUILD_DEFAULT_DEVICE=<DEVICE>                  : -d [device]
# BUILD_DEFAULT_KERNEL_DIR=<KDIR>                  : -k [kernel dir], The directory where the kernel source is located
#                                                      default : /lib/modules/$(uname -r)/build)
# BUILD_DEFAULT_ARCH=<ARCH>                      : -a [arch], Target CPU architecture for cross-compilation, default : `uname -m`
# BUILD_DEFAULT_CROSS_COMPILE=<CROSS_COMPILE>  : -t [cross tool], cross compiler binary, e.g aarch64-linux-gnu-
# BUILD_DEFAULT_INSTALL_DIR=<INSTALL_MOD_PATH> : -i [install dir], module install directory
#

BUILD_DEFAULT_DEVICE="m1"
BUILD_DEFAULT_PCIE="deepx"
BUILD_DEFAULT_KERNEL_DIR=""
BUILD_DEFAULT_ARCH=""
BUILD_DEFAULT_CROSS_COMPILE=""
BUILD_DEFAULT_INSTALL_DIR=""
BUILD_DEFAULT_DBG=""
DXRT_SERVICE_WAS_ACTIVE=0

SUPPORT_DEVICE=("m1" "v3")

declare -A SUPPORT_PCIE_MODULE=(
    ["deepx"]="$(pwd)/pci_deepx"
    ["xilinx"]="$(pwd)/pci_xilinx"
)
PCIE_DEPEND_DEVICE=("m1")

declare -A PCIE_MODPROBE_CONF=(
    ["deepx"]="$(pwd)/dx_dma.conf"
    ["xilinx"]="$(pwd)/xdma.conf"
)

# module uninstall environment
MOD_MODPROBE_DEP="/lib/modules/$(uname -r)/modules.dep"
MOD_MODPROBE_CONF="/etc/modprobe.d"
MOD_INSTALL_DIR="/lib/modules/$(uname -r)/extra"

function logerr() { echo -e "\033[1;31m$*\033[0m"; }
function logmsg() { echo -e "\033[0;33m$*\033[0m"; }
function logext() {
    echo -e "\033[1;31m$*\033[0m"
    exit 1
}

function build_usage() {
    echo " Usage:"
    echo -e "\t$(basename "${0}") <options>"
    echo ""
    echo " options:"
    echo -e "\t-d, --device   [device]      select target device: ${SUPPORT_DEVICE[*]}"
    echo -e "\t-m, --module   [module]      select PCIe module: deepx"
    echo -e "\t-k, --kernel   [kernel dir]  'KERNEL_DIR=[kernel dir]', The directory where the kernel source is located"
    echo -e "\t                                         default: /lib/modules/$(uname -r)/build)"
    echo -e "\t-a, --arch     [arch]        set 'ARCH=[arch]' Target CPU architecture for cross-compilation, default: $(uname -m)"
    echo -e "\t-t, --compiler [cross tool]   'CROSS_COMPILE=[cross tool]' cross compiler binary, e.g aarch64-linux-gnu-"
    echo -e "\t-i, --install  [install dir] 'INSTALL_MOD_PATH=[install dir]', module install directory"
    echo -e "\t                                         install to: [install dir]/lib/modules/[KERNELRELEASE]/extra/"
    echo -e "\t-c, --command  [command]     clean | install | uninstall"
    echo -e "\t                                         - uninstall: Remove the module files installed on the host PC."
    echo -e "\t-j, --jops     [jobs]        set build jobs"
    echo -e "\t-f, --debug    [debug]       set debug feature [debugfs | log | all]"
    echo -e "\t-v, --verbose                build verbose (V=1)"
    echo -e "\t-h, --help                   show this help"
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
        -j | --jops)     _jops="-j${2}"; shift 2 ;;
        -v | --verbose)  _verbose="V=1"; shift ;;
        -f | --debug)    _debug="${2}"; shift 2 ;;
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

check_module_file() {
    local module_path=$1
    local ko_files=()
    if [ ! -d "$module_path" ]; then        
        echo ""
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

check_module_install() {
    local name=$1
    check_module=$(lsmod | grep -wc $name)
    if [[ "$check_module" -ge "1" ]]; then
        logmsg "-> $1 loaded successfully!!"
    else
        logerr "-> Error: Failed to load module $1."
        exit 1
    fi
}

# Checks for and stops the dxrt systemd service if it is running.
function stop_dxrt_service() {
    logmsg "-> Checking for dxrt.service..."
    # 'systemctl' might not exist on all systems, check first.
    if ! command -v systemctl &> /dev/null; then
        logmsg "--> 'systemctl' not found. Skipping service check."
        return
    fi

    # Use 'systemctl is-active' which is quiet and efficient for checks.
    if systemctl is-active --quiet dxrt.service; then
        logerr "--> Found active dxrt.service. Stopping it now..."
        DXRT_SERVICE_WAS_ACTIVE=1
        if ! sudo systemctl stop dxrt.service; then
            logerr "--> FAILED to stop dxrt.service. Manual intervention may be required."
            exit 1
        else
            logmsg "--> Service dxrt.service stopped successfully."
        fi
    else
        logmsg "--> Service dxrt.service is not active."
    fi
}

# Restart dxrt.service only if it was active before stopping.
function restart_dxrt_service_if_needed() {
    if [[ ${DXRT_SERVICE_WAS_ACTIVE} -eq 1 ]]; then
        if ! command -v systemctl &> /dev/null; then
            logmsg "--> 'systemctl' not found. Cannot restart dxrt.service."
            return
        fi
        logmsg "-> Restarting previously active dxrt.service..."
        if sudo systemctl start dxrt.service; then
            if systemctl is-active --quiet dxrt.service; then
                logmsg "--> dxrt.service restarted successfully."
            else
                logerr "--> dxrt.service failed to become active after start."
            fi
        else
            logerr "--> Failed to start dxrt.service."
        fi
    else
        logmsg "-> dxrt.service was not active before install; not starting."
    fi
}

# Checks for and kills user-space processes using a module's device files.
# It now tries 'lsof', then falls back to 'fuser', and warns if neither is found.
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


uninstall_dx_rt_npu_linux_driver_via_dkms() {
    local pacakge_name="dxrt-driver-dkms"
    echo "Uninstalling dkms $pacakge_name ..."

    if dpkg -l | grep -qw "$pacakge_name"; then
        echo "dkms $pacakge_name package is installed. Uninstalling..."
        sudo apt purge -y "$pacakge_name" || {
            echo "Failed to uninstall dkms package. Exiting..."
            exit 1
        }
    else
        echo "dkms $pacakge_name package is not installed. Skipping..."
    fi

    echo "Uninstalling dkms $pacakge_name completed."
}

# This function handles the entire process of safely reloading the kernel modules.
function reload_drivers_forcefully() {
    logmsg "\n*** Preparing to forcefully reload drivers ***"

    # 1. Find the module names from their directories
    local pcie_module_ko rt_module_ko
    pcie_module_ko=$(check_module_file "${SUPPORT_PCIE_MODULE[${_module}]}")
    pcie_module_name=$(basename "$pcie_module_ko" .ko)
    
    rt_module_ko=$(check_module_file "rt") # Assuming 'rt' is the directory name for the rt driver
    rt_module_name=$(basename "$rt_module_ko" .ko)

    # 2. Stop any running services that use the drivers.
    stop_dxrt_service

    # 3. Kill any user-space applications using the devices.
    # The order is important: check the higher-level module first.
    check_and_kill_user_processes "${rt_module_name}"

    # 4. Unload old modules, in reverse dependency order (dependent module first).
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

    # 5. Load new modules, in correct dependency order (base module first).
    logmsg "-> Loading new modules..."
    logmsg "--> Loading module: '${pcie_module_ko}'"
    sudo insmod "${pcie_module_ko}"
    check_module_install "${pcie_module_name}"

    logmsg "--> Loading module: '${rt_module_ko}'"
    sudo insmod "${rt_module_ko}"
    check_module_install "${rt_module_name}"
    
    logmsg "\n*** Driver reload process completed. ***"

    # Attempt to restart service if it was active before.
    restart_dxrt_service_if_needed
}


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
}

parse_args "${@}"
setup_args
print_args

# uninstall modules in host PC
if [[ ${_command} == "uninstall" ]]; then
    logmsg "\n *** Remove : ${MOD_INSTALL_DIR} ***"
    mods=(${SUPPORT_PCIE_MODULE[*]} "rt")
    for i in ${mods[*]}; do
        mod=$(basename ${i})
        [[ ! -d "${MOD_INSTALL_DIR}/${mod}" ]] && continue
        logmsg " $ sudo rm -rf ${MOD_INSTALL_DIR}/${mod}"
        sudo rm -rf "${MOD_INSTALL_DIR}/${mod}"
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
    logmsg "\n *** Update : ${MOD_MODPROBE_DEP} ***"
    logmsg " $ sudo depmod -A"
    if ! eval sudo depmod -A; then
        logext " - FAILED"
    fi
    if [[ -n ${_modconf} ]]; then
        logmsg " $ sudo cp ${_modconf} /etc/modprobe.d/"
        if ! eval sudo cp ${_modconf} /etc/modprobe.d/; then
            logext " - FAILED"
        fi
    fi
    if [[ -z ${_compiler} ]]; then
        ./install-udev_rule.sh
    fi
    # Check dkms
    uninstall_dx_rt_npu_linux_driver_via_dkms

    # Only perform reload for local builds (not cross-compiling)
    if [[ -z ${_compiler} ]]; then
        reload_drivers_forcefully
    fi
fi

