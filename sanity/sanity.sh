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
LOG_FILE="$LOG_DIR/sanity_check_result_$(date +'%Y%m%d_%H%M%S').log"
DMESG_FILE="$LOG_DIR/dmesg_$(date +'%Y%m%d_%H%M%S').log"
PCIE_INFO_FILE="$LOG_DIR/pcie_$(date +'%Y%m%d_%H%M%S').log"

DX_VENDOR_ID="1ff4"
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
    local temp=$(lspci -n | grep "$1" | tr ' ' '\n' | grep "[0-9]:*\.")
    local ext_temp=''
    if [ "$temp" != "" ]; then
        for id in ${temp}; do
            ext_temp+=$(ExtractString "${id}")
            ext_temp+=" "
        done
        pci_id+=$(echo $ext_temp)
    fi
}

# ============================================
# Check Functions
# ============================================

function SC_PCIeLinkUp() {
    echo "==== PCI Link-up Check ====" | tee -a "$LOG_FILE"
    local DEV_NUM=$(lspci -n | grep -c "$DX_VENDOR_ID")
    if [ "$DEV_NUM" -gt 0 ]; then
        echo "[OK] Vendor ID $DX_VENDOR_ID is present in the PCI devices. (num=$DEV_NUM)" | tee -a "$LOG_FILE"
    else
        echo "[ERROR] Vendor ID $DX_VENDOR_ID is NOT found in the PCI devices." | tee -a "$LOG_FILE"
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
    sudo dmesg > "$DMESG_FILE"
    echo "dmesg logs saved to: $DMESG_FILE" | tee -a "$LOG_FILE"
}

function CapturePCIeInfo() {
    touch $PCIE_INFO_FILE
    for id in ${pci_id}; do
        sudo lspci -vvv -s ${id} >> "$PCIE_INFO_FILE"
    done
    echo "PCIe information saved to: $PCIE_INFO_FILE" | tee -a "$LOG_FILE"
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
    echo "[INFO] Running PCIe ioctl test..." | tee -a "$LOG_FILE"
    
    # Capture output and exit code properly
    local TEST_OUTPUT
    TEST_OUTPUT=$("$PCIE_TEST_BIN" 2>&1)
    local TEST_EXIT_CODE=$?
    
    # Always save full output to log file
    echo "$TEST_OUTPUT" >> "$LOG_FILE"
    
    # Check test results
    if [[ $TEST_EXIT_CODE -eq 0 ]]; then
        echo "[OK] PCIe communication test PASSED" | tee -a "$LOG_FILE"
    else
        # On failure, show detailed output to screen
        echo "$TEST_OUTPUT"
        echo ""
        echo "[ERROR] PCIe communication test FAILED (exit code: $TEST_EXIT_CODE)" | tee -a "$LOG_FILE"
        
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
    echo "Usage: sudo ./sanity_check.sh"
    echo ""
    echo "This script checks the DEEPX driver installation and status:"
    echo "  - PCIe device link-up"
    echo "  - Device files (/dev/dxrt*)"
    echo "  - Kernel modules (dxrt_driver, dx_dma)"
    echo "  - Driver installation (legacy, DKMS)"
    echo ""
    echo "Results are saved to: $LOG_DIR/"
}

# Check if help is requested
if [[ "$1" == "help" || "$1" == "-h" || "$1" == "--help" ]]; then
    show_usage
    exit 0
fi

# Check root permission
if [[ $(id -u) -ne 0 ]]; then
    echo "Error: Please run this script as root (use 'sudo')."
    exit 2
fi

# Start sanity check
echo "============================================================================" | tee -a "$LOG_FILE"
echo "==== Driver Sanity Check - $(date) ====" | tee "$LOG_FILE"
echo "Log file location: $(pwd)/$LOG_FILE" | tee -a "$LOG_FILE"
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

# Summary
echo "" | tee -a "$LOG_FILE"
echo "============================================================================" | tee -a "$LOG_FILE"

if [[ $VENDOR_STATUS -ne 0 || $DEV_STATUS -ne 0 || $DRIVER_STATUS -ne 0 || $PCIE_TEST_STATUS -ne 0 ]]; then
    echo "** Sanity check FAILED! Check logs at: $(pwd)/$LOG_FILE" | tee -a "$LOG_FILE"
    echo "** Please report this result to DEEPX with logs" | tee -a "$LOG_FILE"
    
    CaptureDmesg
    CapturePCIeInfo
    
    echo "============================================================================" | tee -a "$LOG_FILE"
    exit 1
else
    echo "** Sanity check PASSED!" | tee -a "$LOG_FILE"
    echo "** Driver is properly installed and running." | tee -a "$LOG_FILE"
    echo "** PCIe communication test: All ioctl commands verified successfully." | tee -a "$LOG_FILE"
    echo "============================================================================" | tee -a "$LOG_FILE"
    exit 0
fi
