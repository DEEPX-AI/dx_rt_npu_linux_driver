#!/bin/bash
SCRIPT_DIR=$(realpath "$(dirname "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_DIR")
DOWNLOAD_DIR="$SCRIPT_DIR/download"
PROJECT_NAME=$(basename "$SCRIPT_DIR")
VENV_PATH="$PROJECT_ROOT/venv-$PROJECT_NAME"
DRIVER_PATH="$PROJECT_ROOT"

pushd "$PROJECT_ROOT" >&2

# color env settings
source ${PROJECT_ROOT}/scripts/color_env.sh
source ${PROJECT_ROOT}/scripts/common_util.sh

ENABLE_DEBUG_LOGS=0
SKIP_REBOOT=0

show_help() {
    echo -e "Usage: ${COLOR_CYAN}$(basename "$0") [OPTIONS]${COLOR_RESET}"
    echo -e ""
    echo -e "Options:"
    echo -e "  ${COLOR_GREEN}[--skip-reboot]${COLOR_RESET}                       Skip reboot after uninstall"
    echo -e ""
    echo -e "  ${COLOR_GREEN}[-v|--verbose]${COLOR_RESET}                        Enable verbose (debug) logging"
    echo -e "  ${COLOR_GREEN}[-h|--help]${COLOR_RESET}                           Display this help message and exit"
    echo -e ""
    
    if [ "$1" == "error" ] && [[ ! -n "$2" ]]; then
        print_colored_v2 "ERROR" "Invalid or missing arguments."
        exit 1
    elif [ "$1" == "error" ] && [[ -n "$2" ]]; then
        print_colored_v2 "ERROR" "$2"
        exit 1
    elif [[ "$1" == "warn" ]] && [[ -n "$2" ]]; then
        print_colored_v2 "WARNING" "$2"
        return 0
    fi
    exit 0
}

uninstall_dx_rt_npu_linux_driver_via_dkms() {
    local pacakge_name="dxrt-driver-dkms"
    print_colored_v2 "INFO" "Uninstalling dkms $pacakge_name ..."

    if dpkg -l | grep -qw "$pacakge_name"; then
        print_colored_v2 "INFO" "dkms $pacakge_name package is installed. Uninstalling..."
        sudo apt purge -y "$pacakge_name" || {
            print_colored_v2 "FAIL" "Failed to uninstall dkms package. Exiting..."
            exit 1
        }
    else
        print_colored_v2 "SKIP" "dkms $pacakge_name package is not installed. Skipping..."
    fi

    print_colored_v2 "SUCCESS" "Uninstalling dkms $pacakge_name completed."
}

uninstall_dx_rt_npu_linux_driver_via_source_build() {
    print_colored_v2 "INFO" "Uninstalling dx_rt_npu_linux_driver via source build ..."

    pushd "${DRIVER_PATH}/modules"
    sudo ./build.sh -c clean || {
        print_colored_v2 "FAIL" "Failed to clean the dx_rt_npu_linux_driver. Exiting..."
        exit 1
    }
    sudo ./build.sh -c uninstall || {
        print_colored_v2 "FAIL" "Failed to uninstall the dx_rt_npu_linux_driver. Exiting..."
        exit 1
    }
    popd

    print_colored_v2 "SUCCESS" "Uninstalling dx_rt_npu_linux_driver via source build completed."
}

uninstall_dx_rt_npu_linux_driver() {
    print_colored_v2 "INFO" "Uninstalling dx_rt_npu_linux_driver..."
    
    uninstall_dx_rt_npu_linux_driver_via_dkms
    uninstall_dx_rt_npu_linux_driver_via_source_build

    print_colored_v2 "SUCCESS" "Uninstalling dx_rt_npu_linux_driver completed."
}

host_reboot() {
    print_colored "The 'dx_rt_npu_linux_driver' has been uninstalled." "INFO"
    print_colored "To complete the uninstallation, the system must be restarted."
    echo -e -n "${COLOR_BRIGHT_GREEN_ON_BLACK}  Would you like to reboot now? (y/n): ${COLOR_RESET}"
    read -r answer
    if [[ "$answer" == "y" || "$answer" == "Y" ]]; then
        echo "Start reboot..."
        sudo reboot now
    fi
}

main() {
    echo "Uninstalling ${PROJECT_NAME} ..."

    uninstall_dx_rt_npu_linux_driver
    if [ "$SKIP_REBOOT" -eq 0 ]; then
        host_reboot
    fi

    echo "Uninstalling ${PROJECT_NAME} done"
}

# parse args
for i in "$@"; do
    case "$1" in
        --skip-reboot)
            SKIP_REBOOT=1
            ;;
        -v|--verbose)
            ENABLE_DEBUG_LOGS=1
            ;;
        -h|--help)
            show_help
            ;;
        *)
            show_help "error" "Invalid option '$1'"
            ;;
    esac
    shift
done

main

popd >&2

exit 0
