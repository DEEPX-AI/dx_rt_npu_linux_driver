#!/bin/bash
#
# BUILD_DEFAULT_DEVICE=<DEVICE>                : -d [device]
# BUILD_DEFAULT_KERNEL_DIR=<KDIR>              : -k [kernel dir], The directory where the kernel source is located
#                                                           default : /lib/modules/$(uname -r)/build)
# BUILD_DEFAULT_ARCH=<ARCH>                    : -a [arch], Target CPU architecture for cross-compilation, default : `uname -m`
# BUILD_DEFAULT_CROSS_COMPILE=<CROSS_COMPILE>  : -t [cross tool], cross compiler binary, e.g aarch64-linux-gnu-
# BUILD_DEFAULT_INSTALL_DIR=<INSTALL_MOD_PATH> : -i [install dir], module install directory
#

BUILD_DEFAULT_DEVICE="m1a"
BUILD_DEFAULT_PCIE="deepx"
BUILD_DEFAULT_KERNEL_DIR=""
BUILD_DEFAULT_ARCH=""
BUILD_DEFAULT_CROSS_COMPILE=""
BUILD_DEFAULT_INSTALL_DIR=""
BUILD_DEFAULT_DBG=""

SUPPORT_DEVICE=("m1" "m1a" "l1" "l3")

declare -A SUPPORT_PCIE_MODULE=(
	["deepx"]="$(pwd)/pci_deepx"
	["xilinx"]="$(pwd)/pci_xilinx"
)
PCIE_DEPEND_DEVICE=("m1" "m1a")

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
	echo -e "\t-m, --module   [module]      select PCIe module: ${!SUPPORT_PCIE_MODULE[@]}"
	echo -e "\t-k, --kernel   [kernel dir]  'KERNEL_DIR=[kernel dir]', The directory where the kernel source is located"
	echo -e "\t                             default: /lib/modules/$(uname -r)/build)"
	echo -e "\t-a, --arch     [arch]        set 'ARCH=[arch]' Target CPU architecture for cross-compilation, default: $(uname -m)"
	echo -e "\t-t, --compiler [cross tool]  'CROSS_COMPILE=[cross tool]' cross compiler binary, e.g aarch64-linux-gnu-"
	echo -e "\t-i, --install  [install dir] 'INSTALL_MOD_PATH=[install dir]', module install directory"
	echo -e "\t                             install to: [install dir]/lib/modules/[KERNELRELEASE]/extra/"
	echo -e "\t-c, --command  [command]     clean | install | uninstall"
	echo -e "\t                             - uninstall: Remove the module files installed on the host PC."
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
		logmsg "- DEVICE        : ${_device}"
	fi
	if [[ -n ${_module} ]]; then
		logmsg "- PCIE          : ${_module}"
	fi
	if [[ -n ${_device} && -n ${_module} ]]; then
		logmsg "- MODULE CONF   : ${_modconf}"
	fi
	if [[ -n ${_arch} ]]; then
		logmsg "- ARCH          : ${_arch}"
	else
		logmsg "- ARCH (HOST)   : $(uname -m)"
	fi
	if [[ -n ${_compiler} ]]; then
		logmsg "- CROSS_COMPILE : ${_compiler}"
	fi
	if [[ -n ${_kerndir} ]]; then
		logmsg "- KERNEL_       : ${_kerndir}"
	else
		logmsg "- KERNEL        : /lib/modules/$(uname -r)/build"
	fi
	if [[ -n ${_destdir} ]]; then
		logmsg "- INSTALL       : ${_destdir}/lib/modules/[KERNELRELEASE]/extra/"
	else
		logmsg "- INSTALL       : /lib/modules/$(uname -r)/extra/"
	fi
	if [[ -n ${_debug} ]]; then
		logmsg "- DEBUG         : ${_debug}"
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
		logmsg " $ rm -rf ${MOD_INSTALL_DIR}/${mod}"
		rm -rf "${MOD_INSTALL_DIR}/${mod}"
	done

	logmsg "\n *** Remove : ${MOD_MODPROBE_CONF} ***"
	for i in ${PCIE_MODPROBE_CONF[*]}; do
		mod=$(basename ${i})
		[[ ! -f "${MOD_MODPROBE_CONF}/${mod}" ]] && continue
		logmsg " $ rm ${MOD_MODPROBE_CONF}/${mod}"
		rm "${MOD_MODPROBE_CONF}/${mod}"
	done

	logmsg "\n *** Update : ${MOD_MODPROBE_DEP} ***"
	logmsg " $ depmod"
	if ! eval depmod; then
		logext " - FAILED"
	fi
	exit 0
fi

# make build
logmsg "\n *** Build : ${_command} ***"
logmsg " $ make ${_args[*]}\n"
if ! eval make "${_args[*]}"; then
	logext " - FAILED"
else
	logmsg " - SUCCESS"
fi

# install
if [[ ${_command} == "install" ]]; then
	logmsg "\n *** Update : ${MOD_MODPROBE_DEP} ***"
	logmsg " $ depmod -A"
	if ! eval depmod -A; then
		logext " - FAILED"
	fi
	if [[ -n ${_modconf} ]]; then
		logmsg " $ cp ${_modconf} /etc/modprobe.d/"
		if ! eval cp ${_modconf} /etc/modprobe.d/; then
			logext " - FAILED"
		fi
	fi
	if [[ -z ${_compiler} ]]; then
		./install-udev_rule.sh
	fi
fi
