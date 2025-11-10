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
STOP_TIMEOUT_SEC=${STOP_TIMEOUT_SEC:-1}
START_TIMEOUT_SEC=${START_TIMEOUT_SEC:-1}
SERVICE_DEBUG_JOURNAL_LINES=${SERVICE_DEBUG_JOURNAL_LINES:-60}

# Virtual DMA dependency configuration
# VDMA_CORE_NAME: preferred module name (default virt_dma). Alternate names tried: virt_dma, virt-dma
# SKIP_VDMA_CHECK=1 can bypass the check (not recommended for production) 
VDMA_CORE_NAME=${VDMA_CORE_NAME:-virt_dma}
SKIP_VDMA_CHECK=${SKIP_VDMA_CHECK:-0}

# --- Internal helpers for service control diagnostics ---
function _dxrt_service_dump_debug() {
    local phase=$1
    command -v systemctl &>/dev/null || return 0
    logerr "[dxrt.service][${phase}] Debug dump begin"
    systemctl show dxrt.service -p ActiveState -p SubState -p ExecMainCode -p ExecMainStatus -p MainPID 2>&1 | sed 's/^/  /'
    systemctl status dxrt.service 2>&1 | head -n 25 | sed 's/^/  /'
    journalctl -u dxrt.service -n ${SERVICE_DEBUG_JOURNAL_LINES} --no-pager 2>&1 | sed 's/^/  /'
    if command -v ps &>/dev/null; then
        local mpid
        mpid=$(systemctl show -p MainPID --value dxrt.service 2>/dev/null || echo 0)
        if [[ "$mpid" != "0" ]]; then
            logmsg "  /proc/${mpid} excerpt:"
            { tr '\0' ' ' < /proc/${mpid}/cmdline 2>/dev/null; echo; } | sed 's/^/    CMD: /'
            cat /proc/${mpid}/status 2>/dev/null | head -n 15 | sed 's/^/    /'
        fi
    fi
    if command -v lsof &>/dev/null; then
        lsof -p $(systemctl show -p MainPID --value dxrt.service 2>/dev/null || echo 0) 2>/dev/null | head -n 15 | sed 's/^/  LSOF: /'
    fi
    logerr "[dxrt.service][${phase}] Debug dump end"
}

function _wait_service_state() {
    local desired=$1   # active|inactive
    local timeout=$2   # seconds (can be fractional)
    local interval=0.2

    command -v systemctl &>/dev/null || return 0

    # Convert a (possibly fractional) seconds value to integer milliseconds.
    _to_ms() {
        # Avoid external bc; use bash string ops.
        local val="$1"
        # If already integer
        if [[ $val =~ ^[0-9]+$ ]]; then
            echo "$(( val * 1000 ))"
            return 0
        fi
        # If fractional (e.g. 1.23)
        if [[ $val =~ ^([0-9]+)\.([0-9]+)$ ]]; then
            local whole=${BASH_REMATCH[1]}
            local frac=${BASH_REMATCH[2]}
            # pad / trim frac to 3 digits for ms
            frac=${frac}000
            frac=${frac:0:3}
            echo "$(( whole * 1000 + 10#${frac} ))"
            return 0
        fi
        # Fallback: treat as 0
        echo 0
    }

    local timeout_ms=$(_to_ms "${timeout:-0}")
    local interval_ms=$(_to_ms "$interval")
    (( timeout_ms <= 0 )) && return 1

    # Monotonic time in ms (prefer /proc/uptime; fallback to date)
    _now_ms() {
        if [[ -r /proc/uptime ]]; then
            # /proc/uptime: "<seconds> <idle>" with seconds possibly fractional
            local up
            read -r up _ < /proc/uptime
            # Reuse _to_ms
            _to_ms "$up"
        else
            # date +%s%3N (not all date versions support %3N). Try nanoseconds then cut.
            if date +%s%3N >/dev/null 2>&1; then
                date +%s%3N
            else
                # Fallback seconds only *1000
                echo "$(( $(date +%s) * 1000 ))"
            fi
        fi
    }

    local start_ms=$(_now_ms)
    local now_ms elapsed_ms

    while :; do
        if systemctl is-active --quiet dxrt.service; then
            [[ $desired == "active" ]] && return 0
        else
            local st
            st=$(systemctl show -p ActiveState --value dxrt.service 2>/dev/null || echo unknown)
            if [[ $desired == "inactive" && ( $st == inactive || $st == failed ) ]]; then
                return 0
            fi
        fi

        now_ms=$(_now_ms)
        elapsed_ms=$(( now_ms - start_ms ))
        (( elapsed_ms >= timeout_ms )) && break
        # Sleep using seconds (interval may be fractional)
        sleep "$interval"
    done
    return 1
}

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
        logmsg "--> dxrt.service active. Attempting stop..."
        DXRT_SERVICE_WAS_ACTIVE=1
        local pre_pid
        pre_pid=$(systemctl show -p MainPID --value dxrt.service 2>/dev/null || echo 0)

        if ! sudo systemctl stop dxrt.service; then
            logerr "--> systemctl stop command failed immediately."
            _dxrt_service_dump_debug stop_fail_cmd
            exit 1
        fi
        if ! _wait_service_state inactive ${STOP_TIMEOUT_SEC}; then
            logerr "--> Service did not reach inactive within ${STOP_TIMEOUT_SEC}s"
            _dxrt_service_dump_debug stop_timeout
            # last resort: force kill
            if [[ -n "$pre_pid" && "$pre_pid" != "0" ]]; then
                logerr "--> Forcing kill -9 on old PID ${pre_pid}"
                sudo kill -9 "$pre_pid" 2>/dev/null || true
                sleep 1
                if systemctl is-active --quiet dxrt.service; then
                    logerr "--> Still active after forced kill. Aborting."
                    exit 1
                fi
            fi
        else
            logmsg "--> Service dxrt.service now inactive."
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
        systemctl reset-failed dxrt.service 2>/dev/null || true
        local start_rc=0
        if ! sudo systemctl start dxrt.service; then
            start_rc=$?
            logerr "--> systemctl start returned failure (rc=${start_rc})"
            _dxrt_service_dump_debug start_fail_cmd
            return
        fi
        if ! _wait_service_state active ${START_TIMEOUT_SEC}; then
            logerr "--> dxrt.service failed to become active within ${START_TIMEOUT_SEC}s"
            _dxrt_service_dump_debug start_timeout
            return
        fi
        # Stabilization: ensure it doesn't exit immediately after appearing active
        sleep 0.5
        if ! systemctl is-active --quiet dxrt.service; then
            logerr "--> dxrt.service became inactive shortly after start"
            _dxrt_service_dump_debug start_unstable
            return
        fi
        logmsg "--> dxrt.service restarted successfully (active and stable)."
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

    # Ensure virtual DMA dependency early (before stopping service / unloading modules)
    ensure_virtual_dma_dependency

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

    # 6. Attempt to restart service if it was active before.
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