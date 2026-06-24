#!/bin/bash
#
# sanity_check.sh - Driver sanity check script
#
# This script verifies that the DEEPX driver is properly installed and running.

# ============================================
# Configuration
# ============================================

LOG_DIR="result"
mkdir -p "$LOG_DIR"
TS="$(date +'%Y%m%d_%H%M%S')"
LOG_FILE="$LOG_DIR/sanity_check_result_${TS}.log"
DMESG_FILE="$LOG_DIR/dmesg_${TS}.log"
DMESG_FILTERED_FILE="$LOG_DIR/dmesg_filtered_${TS}.log"
PCIE_INFO_FILE="$LOG_DIR/pcie_${TS}.log"
SYSINFO_FILE="$LOG_DIR/sysinfo_${TS}.log"
DRVINFO_FILE="$LOG_DIR/drvinfo_${TS}.log"
NPUSTATE_FILE="$LOG_DIR/npustate_${TS}.log"
BUNDLE_FILE="$LOG_DIR/dxrt_issue_bundle_${TS}.tar.gz"

DX_VENDOR_ID="1ff4"
# Supported device IDs: M1_LEGACY=0x0000 M1=0x0100 M1M=0x0110 H1[M1]=0x0101 H1[M1M]=0x0111 
DX_SUPPORTED_DEV_IDS="0000 0100 0110 0101 0111"
RT_DRV_KO="dxrt_driver"
PCIE_DRV_KO="dx_dma"

pci_id=""

# ============================================
# Helper Functions
# ============================================

function ExtractString() {
    local input_string="$1"
    local temp

    if [[ $input_string == *:* && $input_string != *:*:* ]]; then
        temp="$input_string"
    elif [[ $input_string == *:*:* ]]; then
        temp=$(echo "$input_string" | awk -F':' '{print $2":"$3}')
    fi
    echo "$temp"
}

function GetPCIeId() {
    local vendor="$1"
    local ext_temp=''
    for dev_id in $DX_SUPPORTED_DEV_IDS; do
        local pattern="${vendor}:${dev_id}"
        local temp=$(lspci -n | grep "$pattern" | tr ' ' '\n' | grep "[0-9]:*\.")
        if [ "$temp" != "" ]; then
            for id in ${temp}; do
                ext_temp+=$(ExtractString "${id}")
                ext_temp+=" "
            done
        fi
    done
    pci_id+=$(echo $ext_temp)
}

# ============================================
# Check Functions
# ============================================

function SC_PCIeLinkUp() {
    echo "==== PCI Link-up Check ====" | tee -a "$LOG_FILE"
    local DEV_NUM=0
    for dev_id in $DX_SUPPORTED_DEV_IDS; do
        local cnt=$(lspci -n | grep -c "${DX_VENDOR_ID}:${dev_id}")
        DEV_NUM=$((DEV_NUM + cnt))
    done
    if [ "$DEV_NUM" -gt 0 ]; then
        echo "[OK] Supported DEEPX devices found. (num=$DEV_NUM)" | tee -a "$LOG_FILE"
        lspci -n -d ${DX_VENDOR_ID}: | while read -r line; do
            echo "  $line" | tee -a "$LOG_FILE"
        done
    else
        echo "[ERROR] No supported DEEPX devices found (vendor=$DX_VENDOR_ID, device=$DX_SUPPORTED_DEV_IDS)." | tee -a "$LOG_FILE"
        local ALL_DX=$(lspci -n | grep -c "$DX_VENDOR_ID")
        if [ "$ALL_DX" -gt 0 ]; then
            echo "[INFO] Other DEEPX vendor devices present but not supported by this driver:" | tee -a "$LOG_FILE"
            lspci -n -d ${DX_VENDOR_ID}: | while read -r line; do
                echo "  $line" | tee -a "$LOG_FILE"
            done
        fi
        return 1
    fi
    return 0
}

function SC_DevFile() {
    echo "==== Device File Check ====" | tee -a "$LOG_FILE"
    local DEVICE_PATTERN="/dev/dxrt*"
    local FOUND_DEVICES=false
    local ERROR_FOUND=0

    for DEV in $DEVICE_PATTERN; do
        [[ -e "$DEV" ]] || continue

        FOUND_DEVICES=true
        echo "[OK] $DEV exists." | tee -a "$LOG_FILE"

        if [[ -c "$DEV" ]]; then
            echo "[OK] $DEV is a character device." | tee -a "$LOG_FILE"
        else
            echo "[ERROR] $DEV is not a character device." | tee -a "$LOG_FILE"
            ERROR_FOUND=1
            continue
        fi

        PERMS=$(stat -c "%a" "$DEV")
        if [[ "$PERMS" == "666" ]]; then
            echo "[OK] $DEV has correct permissions (0666)." | tee -a "$LOG_FILE"
        else
            echo "[ERROR] $DEV has incorrect permissions: $PERMS (expected: 0666)" | tee -a "$LOG_FILE"
            ERROR_FOUND=1
        fi
    done

    if [[ "$FOUND_DEVICES" == false ]]; then
        echo "[ERROR] No devices found matching pattern: $DEVICE_PATTERN" | tee -a "$LOG_FILE"
        ERROR_FOUND=1
    fi

    return $ERROR_FOUND
}

function SC_DriverCheck() {
    echo "==== Kernel Module Check ====" | tee -a "$LOG_FILE"
    local ERROR_FOUND=0

    if lsmod | grep -q $RT_DRV_KO; then
        echo "[OK] $RT_DRV_KO module is loaded." | tee -a "$LOG_FILE"
    else
        echo "[ERROR] $RT_DRV_KO module is NOT loaded." | tee -a "$LOG_FILE"
        ERROR_FOUND=1
    fi

    if lsmod | grep -q $PCIE_DRV_KO; then
        echo "[OK] $PCIE_DRV_KO module is loaded." | tee -a "$LOG_FILE"
    else
        echo "[ERROR] $PCIE_DRV_KO module is NOT loaded." | tee -a "$LOG_FILE"
        ERROR_FOUND=1
    fi

    GetPCIeId $DX_VENDOR_ID
    for id in ${pci_id}; do
        if lspci -vvk -s ${id} | grep -q "Kernel driver in use: dx_dma_pcie"; then
            echo "[OK] PCIe ${id} driver probe is success." | tee -a "$LOG_FILE"
        else
            echo "[ERROR] PCIe ${id} driver probe is fail." | tee -a "$LOG_FILE"
            ERROR_FOUND=1
        fi
    done

    return $ERROR_FOUND
}

function SC_Legacy_Driver_Check() {
    echo "==== Legacy Driver Installation Check ====" | tee -a "$LOG_FILE"

    local DRIVER_FILE_PATH=/lib/modules/$(uname -r)/kernel/drivers/dxrt_driver.ko
    if [ -f $DRIVER_FILE_PATH ]; then
        echo "[INFO] $DRIVER_FILE_PATH ... OK" | tee -a "$LOG_FILE"
    else
        echo "[INFO] $DRIVER_FILE_PATH ... NONE" | tee -a "$LOG_FILE"
    fi

    local DMA_FILE_PATH=/lib/modules/$(uname -r)/kernel/drivers/dx_dma.ko
    if [ -f $DMA_FILE_PATH ]; then
        echo "[INFO] $DMA_FILE_PATH ... OK" | tee -a "$LOG_FILE"
    else
        echo "[INFO] $DMA_FILE_PATH ... NONE" | tee -a "$LOG_FILE"
    fi

    local DRIVER_EXTRA_PATH=/lib/modules/$(uname -r)/extra/rt/dxrt_driver.ko
    if [ -f $DRIVER_EXTRA_PATH ]; then
        echo "[INFO] $DRIVER_EXTRA_PATH ... OK" | tee -a "$LOG_FILE"
    else
        echo "[INFO] $DRIVER_EXTRA_PATH ... NONE" | tee -a "$LOG_FILE"
    fi

    local DMA_EXTRA_PATH=/lib/modules/$(uname -r)/extra/pci_deepx/dx_dma.ko
    if [ -f $DMA_EXTRA_PATH ]; then
        echo "[INFO] $DMA_EXTRA_PATH ... OK" | tee -a "$LOG_FILE"
    else
        echo "[INFO] $DMA_EXTRA_PATH ... NONE" | tee -a "$LOG_FILE"
    fi
}

function SC_DKMS_Check() {
    echo "==== DKMS Driver Installation Check ====" | tee -a "$LOG_FILE"

    DKMS_STATUS=$(dkms status -m dxrt-driver-dkms 2>/dev/null)
    if [[ -n "$DKMS_STATUS" ]]; then
        IFS=$'\n'
        for line in $DKMS_STATUS; do
            echo "[INFO] $line" | tee -a "$LOG_FILE"
        done
        unset IFS 
    else
        echo "[INFO] DKMS package 'dxrt-driver-dkms' is not installed" | tee -a "$LOG_FILE"
    fi

    local DRIVER_FILE_PATH=/lib/modules/$(uname -r)/updates/dkms/dxrt_driver.ko
    if [ -f $DRIVER_FILE_PATH ]; then
        echo "[INFO] $DRIVER_FILE_PATH ... OK" | tee -a "$LOG_FILE"
    else
        echo "[INFO] $DRIVER_FILE_PATH ... NONE" | tee -a "$LOG_FILE"
    fi

    local DMA_FILE_PATH=/lib/modules/$(uname -r)/updates/dkms/dx_dma.ko
    if [ -f $DMA_FILE_PATH ]; then
        echo "[INFO] $DMA_FILE_PATH ... OK" | tee -a "$LOG_FILE"
    else
        echo "[INFO] $DMA_FILE_PATH ... NONE" | tee -a "$LOG_FILE"
    fi
}

function CaptureDmesg() {
    sudo dmesg > "$DMESG_FILE" 2>/dev/null
    echo "dmesg logs saved to: $DMESG_FILE" | tee -a "$LOG_FILE"
    # Filtered dmesg for quick scan in tickets
    sudo dmesg 2>/dev/null | grep -iE "dxrt|dx_dma|dx_dma_pcie|pcie|aer|npu|iommu" > "$DMESG_FILTERED_FILE"
    echo "filtered dmesg saved to: $DMESG_FILTERED_FILE" | tee -a "$LOG_FILE"
}

function CapturePCIeInfo() {
    : > "$PCIE_INFO_FILE"
    {
        echo "==== lspci topology (-tv) ===="
        lspci -tv 2>/dev/null
        echo ""
        echo "==== lspci -nn (DEEPX devices) ===="
        lspci -nn -d ${DX_VENDOR_ID}: 2>/dev/null
        echo ""
    } >> "$PCIE_INFO_FILE"
    for id in ${pci_id}; do
        {
            echo "============================================================"
            echo "==== PCIe device $id (lspci -vvv) ===="
            echo "============================================================"
            sudo lspci -vvv -s ${id} 2>/dev/null
            echo ""
            echo "==== Link / AER sysfs ($id) ===="
            local sysdir="/sys/bus/pci/devices/0000:${id}"
            for f in current_link_speed current_link_width max_link_speed max_link_width \
                     aer_dev_correctable aer_dev_fatal aer_dev_nonfatal; do
                if [[ -r "$sysdir/$f" ]]; then
                    echo "--- $f ---"
                    cat "$sysdir/$f" 2>/dev/null
                fi
            done
            echo ""
        } >> "$PCIE_INFO_FILE"
    done
    echo "PCIe information saved to: $PCIE_INFO_FILE" | tee -a "$LOG_FILE"
}

function CollectSystemInfo() {
    echo "==== System Info Collection ====" | tee -a "$LOG_FILE"
    {
        echo "==== Date / Host ===="
        date
        echo "hostname: $(hostname)"
        echo "user    : $(whoami)"
        echo "uptime  : $(uptime)"
        echo ""
        echo "==== uname -a ===="
        uname -a
        echo ""
        echo "==== /etc/os-release ===="
        cat /etc/os-release 2>/dev/null
        echo ""
        echo "==== lscpu (head) ===="
        lscpu 2>/dev/null | head -25
        echo ""
        echo "==== Memory (free -h) ===="
        free -h 2>/dev/null
        echo ""
        echo "==== Kernel cmdline ===="
        cat /proc/cmdline 2>/dev/null
        echo ""
        echo "==== IOMMU status (dmesg) ===="
        sudo dmesg 2>/dev/null | grep -iE "iommu|dmar|amd-vi" | head -20
        echo ""
        echo "==== CPU governor ===="
        for f in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
            [[ -r "$f" ]] && echo "$f: $(cat "$f")"
        done | head -8
    } > "$SYSINFO_FILE" 2>&1
    echo "system info saved to: $SYSINFO_FILE" | tee -a "$LOG_FILE"
}

function CollectDriverInfo() {
    echo "==== Driver / FW Info Collection ====" | tee -a "$LOG_FILE"
    {
        echo "==== modinfo $RT_DRV_KO ===="
        modinfo $RT_DRV_KO 2>/dev/null
        echo ""
        echo "==== modinfo $PCIE_DRV_KO ===="
        modinfo $PCIE_DRV_KO 2>/dev/null
        echo ""
        echo "==== Module parameters: $RT_DRV_KO ===="
        local pdir="/sys/module/${RT_DRV_KO}/parameters"
        if [[ -d "$pdir" ]]; then
            for p in "$pdir"/*; do
                [[ -r "$p" ]] && echo "$(basename "$p") = $(cat "$p" 2>/dev/null)"
            done
        else
            echo "(module not loaded)"
        fi
        echo ""
        echo "==== Module parameters: $PCIE_DRV_KO ===="
        pdir="/sys/module/${PCIE_DRV_KO}/parameters"
        if [[ -d "$pdir" ]]; then
            for p in "$pdir"/*; do
                [[ -r "$p" ]] && echo "$(basename "$p") = $(cat "$p" 2>/dev/null)"
            done
        else
            echo "(module not loaded)"
        fi
        echo ""
        echo "==== lsmod (DEEPX related) ===="
        lsmod | grep -E "^(dxrt_driver|dx_dma|pcie_vnpu_dx)" 2>/dev/null
        echo ""
        echo "==== release.ver files ===="
        local sd="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.."
        for f in "$sd/release.ver" "$sd/modules/pci_deepx/release.ver" "$sd/modules/pci_xilinx/release.ver"; do
            [[ -r "$f" ]] && echo "$f: $(cat "$f")"
        done
    } > "$DRVINFO_FILE" 2>&1
    echo "driver info saved to: $DRVINFO_FILE" | tee -a "$LOG_FILE"
}

function CollectNPUState() {
    echo "==== NPU Runtime State Collection ====" | tee -a "$LOG_FILE"
    {
        echo "==== /sys/class/dxrt/* ===="
        for d in /sys/class/dxrt/dxrt*; do
            [[ -d "$d" ]] || continue
            echo "--- $d ---"
            ls -la "$d" 2>/dev/null
            if [[ -d "$d/recovery" ]]; then
                echo "--- $d/recovery ---"
                for f in "$d/recovery"/*; do
                    [[ -r "$f" ]] && echo "$(basename "$f") = $(cat "$f" 2>/dev/null | head -5)"
                done
            fi
        done
        echo ""
        echo "==== /proc/interrupts (DEEPX MSI) ===="
        grep -E "dxrt|dx_dma|pcie-vnpu" /proc/interrupts 2>/dev/null
        echo ""
        echo "==== /proc/devices (char) ===="
        awk '/^Character/{f=1;next}/^Block/{f=0}f' /proc/devices | grep -iE "dxrt|dx"
    } > "$NPUSTATE_FILE" 2>&1
    echo "npu state saved to: $NPUSTATE_FILE" | tee -a "$LOG_FILE"
}

function MakeBundle() {
    local files=()
    for f in "$LOG_FILE" "$DMESG_FILE" "$DMESG_FILTERED_FILE" "$PCIE_INFO_FILE" \
             "$SYSINFO_FILE" "$DRVINFO_FILE" "$NPUSTATE_FILE"; do
        [[ -f "$f" ]] && files+=("$f")
    done
    if [[ ${#files[@]} -eq 0 ]]; then
        echo "[WARN] No log files to bundle." | tee -a "$LOG_FILE"
        return 1
    fi
    tar czf "$BUNDLE_FILE" "${files[@]}" 2>/dev/null
    echo "" | tee -a "$LOG_FILE"
    echo "** Issue bundle created: $BUNDLE_FILE" | tee -a "$LOG_FILE"
    echo "** Attach this file when filing a ticket." | tee -a "$LOG_FILE"
}

function SC_PCIeTest() {
    echo "==== PCIe Communication Test (ioctl) ====" | tee -a "$LOG_FILE"
    
    # Resolve symlink to get actual script location
    local SCRIPT_PATH="${BASH_SOURCE[0]}"
    if [[ -L "$SCRIPT_PATH" ]]; then
        SCRIPT_PATH="$(readlink -f "$SCRIPT_PATH")"
    fi
    local SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_PATH")" && pwd)"
    local PCIE_TEST_BIN="$SCRIPT_DIR/tools/pcie_test"
    local TOOLS_DIR="$SCRIPT_DIR/tools"
    local ERROR_FOUND=0
   
    # Check if pcie_test binary exists, build if not
    if [[ ! -f "$PCIE_TEST_BIN" ]]; then
        echo "[INFO] pcie_test binary not found, building..." | tee -a "$LOG_FILE"
        
        # Check if tools directory exists
        if [[ ! -d "$TOOLS_DIR" ]]; then
            echo "[ERROR] Tools directory not found at: $TOOLS_DIR" | tee -a "$LOG_FILE"
            return 1
        fi
        
        # Build pcie_test
        echo "[INFO] Running: cd $TOOLS_DIR && make" | tee -a "$LOG_FILE"
        local BUILD_OUTPUT=$(cd "$TOOLS_DIR" && make 2>&1)
        local BUILD_STATUS=$?
        
        if [[ $BUILD_STATUS -ne 0 ]]; then
            echo "[ERROR] Failed to build pcie_test" | tee -a "$LOG_FILE"
            echo "$BUILD_OUTPUT" >> "$LOG_FILE"
            return 1
        fi
        
        # Verify build success
        if [[ ! -f "$PCIE_TEST_BIN" ]]; then
            echo "[ERROR] Build completed but binary not found at: $PCIE_TEST_BIN" | tee -a "$LOG_FILE"
            return 1
        fi
        
        echo "[OK] pcie_test built successfully" | tee -a "$LOG_FILE"
    else
        echo "[INFO] pcie_test binary already exists at: $PCIE_TEST_BIN" | tee -a "$LOG_FILE"
    fi
    
    # Check if binary is executable
    if [[ ! -x "$PCIE_TEST_BIN" ]]; then
        chmod +x "$PCIE_TEST_BIN"
    fi
    
    # Run pcie_test and capture output
    echo "[INFO] Running PCIe ioctl test (repeat=${REPEAT_COUNT})..." | tee -a "$LOG_FILE"

    local pass_cnt=0
    local fail_cnt=0
    local fail_attempts=""
    local last_exit=0
    local TEST_OUTPUT=""

    for ((i=1; i<=REPEAT_COUNT; i++)); do
        TEST_OUTPUT=$("$PCIE_TEST_BIN" 2>&1)
        last_exit=$?
        {
            echo "---- pcie_test attempt $i/$REPEAT_COUNT (exit=$last_exit) ----"
            echo "$TEST_OUTPUT"
        } >> "$LOG_FILE"
        if [[ $last_exit -eq 0 ]]; then
            pass_cnt=$((pass_cnt+1))
        else
            fail_cnt=$((fail_cnt+1))
            fail_attempts+="$i(exit=$last_exit) "
        fi
    done

    local TEST_EXIT_CODE=0
    if [[ $fail_cnt -gt 0 ]]; then
        TEST_EXIT_CODE=$last_exit
    fi

    # Check test results
    if [[ $TEST_EXIT_CODE -eq 0 ]]; then
        echo "[OK] PCIe communication test PASSED (${pass_cnt}/${REPEAT_COUNT})" | tee -a "$LOG_FILE"
    else
        # On failure, show last attempt output to screen
        echo "$TEST_OUTPUT"
        echo ""
        echo "[ERROR] PCIe communication test FAILED (${fail_cnt}/${REPEAT_COUNT}) - failed attempts: ${fail_attempts}" | tee -a "$LOG_FILE"
        echo "[ERROR] Last exit code: $TEST_EXIT_CODE" | tee -a "$LOG_FILE"
        
        # Detailed error based on exit code
        case $TEST_EXIT_CODE in
            1)
                echo "[ERROR] - Failed to open device /dev/dxrt0" | tee -a "$LOG_FILE"
                echo "[ERROR] - Driver may not be loaded or device file missing" | tee -a "$LOG_FILE"
                ;;
            2)
                echo "[ERROR] - IDENTIFY_DEVICE ioctl failed" | tee -a "$LOG_FILE"
                echo "[ERROR] - Device opened but cannot get device information" | tee -a "$LOG_FILE"
                ;;
            3)
                echo "[ERROR] - WRITE_MEM ioctl failed" | tee -a "$LOG_FILE"
                echo "[ERROR] - Cannot write to device memory" | tee -a "$LOG_FILE"
                ;;
            4)
                echo "[ERROR] - READ_MEM ioctl failed" | tee -a "$LOG_FILE"
                echo "[ERROR] - Cannot read from device memory" | tee -a "$LOG_FILE"
                ;;
            5)
                echo "[ERROR] - Data verification failed" | tee -a "$LOG_FILE"
                echo "[ERROR] - Memory read/write data mismatch" | tee -a "$LOG_FILE"
                ;;
            *)
                echo "[ERROR] - Unknown error occurred" | tee -a "$LOG_FILE"
                ;;
        esac
        
        ERROR_FOUND=1
    fi
    
    return $ERROR_FOUND
}

# ============================================
# Main
# ============================================

function show_usage() {
    echo "Usage: sudo ./sanity_check.sh [OPTIONS]"
    echo ""
    echo "This script checks the DEEPX driver installation and status,"
    echo "and collects context information useful for issue triage:"
    echo "  - PCIe device link-up"
    echo "  - Device files (/dev/dxrt*)"
    echo "  - Kernel modules (dxrt_driver, dx_dma)"
    echo "  - Driver installation (legacy, DKMS)"
    echo "  - PCIe ioctl communication test"
    echo "  - System info, driver/FW info, NPU runtime state (always collected)"
    echo "  - dmesg, PCIe detail, issue bundle (tar.gz) -- collected on FAIL"
    echo ""
    echo "Options:"
    echo "  -l, --collect-logs    Also collect dmesg/PCIe logs and create bundle"
    echo "                        on PASS (for internal development use)"
    echo "  -r, --repeat N        Repeat PCIe ioctl test N times to catch"
    echo "                        intermittent failures (default: 1)"
    echo "  -h, --help            Show this help message"
    echo ""
    echo "Results are saved to: $LOG_DIR/"
}

# Parse options
COLLECT_LOGS=0
REPEAT_COUNT=1
while [[ $# -gt 0 ]]; do
    case "$1" in
        help|-h|--help)
            show_usage
            exit 0
            ;;
        -l|--collect-logs)
            COLLECT_LOGS=1
            shift
            ;;
        -r|--repeat)
            shift
            if [[ -z "$1" || ! "$1" =~ ^[0-9]+$ || "$1" -lt 1 ]]; then
                echo "Error: --repeat requires a positive integer"
                exit 1
            fi
            REPEAT_COUNT=$1
            shift
            ;;
        --repeat=*)
            REPEAT_COUNT="${1#*=}"
            if [[ ! "$REPEAT_COUNT" =~ ^[0-9]+$ || "$REPEAT_COUNT" -lt 1 ]]; then
                echo "Error: --repeat requires a positive integer"
                exit 1
            fi
            shift
            ;;
        *)
            echo "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Check root permission
if [[ $(id -u) -ne 0 ]]; then
    echo "Error: Please run this script as root (use 'sudo')."
    exit 2
fi

# Start sanity check
echo "============================================================================" | tee "$LOG_FILE"
echo "==== Driver Sanity Check - $(date) ====" | tee -a "$LOG_FILE"
echo "Host        : $(hostname)" | tee -a "$LOG_FILE"
echo "User        : $(whoami)" | tee -a "$LOG_FILE"
echo "Kernel      : $(uname -r) ($(uname -m))" | tee -a "$LOG_FILE"
if [[ -r /etc/os-release ]]; then
    echo "OS          : $(. /etc/os-release; echo "$PRETTY_NAME")" | tee -a "$LOG_FILE"
fi
RT_VER=$(modinfo -F version $RT_DRV_KO 2>/dev/null)
DMA_VER=$(modinfo -F version $PCIE_DRV_KO 2>/dev/null)
echo "Driver ver  : ${RT_DRV_KO}=${RT_VER:-N/A}  ${PCIE_DRV_KO}=${DMA_VER:-N/A}" | tee -a "$LOG_FILE"
echo "Repeat      : ${REPEAT_COUNT}" | tee -a "$LOG_FILE"
echo "Log file    : $(pwd)/$LOG_FILE" | tee -a "$LOG_FILE"
echo "============================================================================" | tee -a "$LOG_FILE"
echo "" | tee -a "$LOG_FILE"

# Run all checks
SC_PCIeLinkUp
VENDOR_STATUS=$?

SC_DevFile
DEV_STATUS=$?

SC_DriverCheck
DRIVER_STATUS=$?

SC_Legacy_Driver_Check

SC_DKMS_Check

SC_PCIeTest
PCIE_TEST_STATUS=$?

# Always collect triage context (system / driver / NPU state)
echo "" | tee -a "$LOG_FILE"
echo "==== Issue Triage Context Collection ====" | tee -a "$LOG_FILE"
CollectSystemInfo
CollectDriverInfo
CollectNPUState

# Summary
echo "" | tee -a "$LOG_FILE"
echo "============================================================================" | tee -a "$LOG_FILE"

if [[ $VENDOR_STATUS -ne 0 || $DEV_STATUS -ne 0 || $DRIVER_STATUS -ne 0 || $PCIE_TEST_STATUS -ne 0 ]]; then
    echo "** Sanity check FAILED! Check logs at: $(pwd)/$LOG_FILE" | tee -a "$LOG_FILE"
    echo "** Please report this result to DEEPX with logs" | tee -a "$LOG_FILE"

    CaptureDmesg
    CapturePCIeInfo
    MakeBundle

    echo "============================================================================" | tee -a "$LOG_FILE"
    exit 1
else
    echo "** Sanity check PASSED!" | tee -a "$LOG_FILE"
    echo "** Driver is properly installed and running." | tee -a "$LOG_FILE"
    echo "** PCIe communication test: All ioctl commands verified successfully." | tee -a "$LOG_FILE"

    if [[ $COLLECT_LOGS -eq 1 ]]; then
        echo "** Collecting full logs (--collect-logs)" | tee -a "$LOG_FILE"
        CaptureDmesg
        CapturePCIeInfo
        MakeBundle
    fi

    echo "============================================================================" | tee -a "$LOG_FILE"
    exit 0
fi
