#!/bin/bash
#########################################
# This script is not guaranteed to work #
#########################################

BUILD_DEFAULT_PCIE="deepx"

declare -A SUPPORT_PCIE_MODULE=(
	["deepx"]="$(pwd)/pci_deepx"
	["xilinx"]="$(pwd)/pci_xilinx"
)

function logmsg() { echo -e "\033[0;33m$*\033[0m"; }

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
    check_module=$(lsmod | grep -c $name)
    if [[ "$check_module" -ge "1" ]]; then
        echo "$1 loaded successfully!!"
    else
        echo "Error: $file_path does not exist."
        exit 1
    fi
}

module_insert() {
    module_path=$1
    module_path=$(readlink -f $module_path)
    echo "** Start loading module from [$module_path] **"
    module=$(check_module_file $module_path)
    if [ "$module" = "" ]; then    
        echo "Error: No module in $module_path."
        exit 1
    fi
    module_name=$(echo $(basename $module) | cut  -d '.' -f1)

    chk_module=$(lsmod | grep -c ${module_name})
    if [[ "$chk_module" -ge "1" ]]; then
        echo "$module_name is already loaded and remove it"
        input=$(lsmod | grep ${module_name})
        IFS=' ' read -r -a info <<< "$input"
        if [ -n "${info[3]}" ]; then
            sudo rmmod ${info[3]}
            echo "remove module : ${info[3]}"
        fi
        sudo rmmod $module_name
    fi

    ## Module load
    sudo insmod $module
    check_module_install $module_name
}

pcie_rescan() {
    curt_d=$(pwd)
    pcie_rescan_f="${curt_d}/../utils/script/pcie_rescan.sh"
    if [ -f "${pcie_rescan_f}" ]; then
        chmod +x ${pcie_rescan_f}
        sudo ${pcie_rescan_f}
    else
        echo "[ ${pcie_rescan_f} ] is not found!"
    fi
}

_module=""

function script_usage() {
	echo " Usage:"
	echo -e "\t$(basename "${0}") <options>"
	echo ""
	echo " options:"
	echo -e "\t-m [module]\t select PCIe module: ${!SUPPORT_PCIE_MODULE[@]}"
	echo ""
}

function script_args() {
	while getopts "m:h" opt; do
		case ${opt} in
		m) _module="${OPTARG}" ;;
		h)
			script_usage
			exit 0
			;;
		*) exit 1 ;;
		esac
	done
	if [[ -n ${_module} ]]; then
		if ! echo "${!SUPPORT_PCIE_MODULE[*]}" | grep -qw "${_module}"; then
			logext "Not found module: ${_module} !"
		fi
	fi
}

function print_args() {
	if [[ -n ${_module} ]]; then
		logmsg "- PCIE          : ${_module}"
	fi
}

# Main Script
script_args "${@}"
print_args

# set default build target
if [[ -z ${_module} ]]; then
	_module="${BUILD_DEFAULT_PCIE}"
fi

if [[ "$(id -u)" -ne "0" ]]; then
	echo "Error: Module insertion script should run as root"
	exit 1
fi

# Only for accelator mode device
module_insert ${SUPPORT_PCIE_MODULE[${_module}]}
module_insert rt
pcie_rescan