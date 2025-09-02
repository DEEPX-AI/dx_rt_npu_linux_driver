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

install_dx_rt_npu_linux_driver_via_source_build() {
    sudo apt update && sudo apt-get -y install pciutils kmod build-essential make linux-headers-6.11.0-17-generic
    
    pushd "${DRIVER_PATH}"
    # if .gitmodules file is exist, submodule init and update.
    if [ -f .gitmodules ]; then
        git submodule update --init --recursive
    fi
    popd
    
    pushd "${DRIVER_PATH}/modules"
    ./build.sh
    sudo ./build.sh -c install

    popd
}

install_dx_rt_npu_linux_driver_via_dkms() {
    local deb_pattern="${DRIVER_PATH}/release/latest/dxrt-driver-dkms*.deb"

    if compgen -G "$deb_pattern" > /dev/null; then
        pushd "${DRIVER_PATH}/release/latest" > /dev/null
        sudo apt install -y ./dxrt-driver-dkms*.deb
        popd > /dev/null
    else
        print_colored "DKMS package not found. Switching to source build installation." "WARNING"
        install_dx_rt_npu_linux_driver_via_source_build
    fi
}

install_dx_rt_npu_linux_driver() {
    DX_RT_DRIVER_INCLUDED=1

    # Add driver uninstall function for prevents conflicts
    ${DRIVER_PATH}/uninstall.sh --skip-reboot

    if [ "${USE_DRIVER_SOURCE_BUILD}" = "y" ]; then
        install_dx_rt_npu_linux_driver_via_source_build
    else
        install_dx_rt_npu_linux_driver_via_dkms
    fi
}

host_reboot() {
    print_colored "The 'dx_rt_npu_linux_driver' has been installed." "INFO"
    print_colored "To complete the installation, the system must be restarted."
    echo -e -n "${COLOR_BRIGHT_GREEN_ON_BLACK}  Would you like to reboot now? (y/n): ${COLOR_RESET}"
    read -r answer
    if [[ "$answer" == "y" || "$answer" == "Y" ]]; then
        echo "Start reboot..."
        sudo reboot now
    fi
}

main() {
    echo "Installing ${PROJECT_NAME} ..."

    install_dx_rt_npu_linux_driver
    if [ "$SKIP_REBOOT" -eq 0 ]; then
        host_reboot
    fi

    echo "Installing ${PROJECT_NAME} done"
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
