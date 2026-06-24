# DEEPX NPU Linux Driver

Linux kernel driver for DEEPX NPU accelerator devices.

## Download

Clone the repository:

```shell
git clone https://github.com/DEEPX-AI/dx_rt_npu_linux_driver.git
cd dx_rt_npu_linux_driver
```

## Quick Start

```shell
# Check whether a DEEPX PCIe device is detected
lspci -d 1ff4:

# Build kernel modules
cd ./modules
./build.sh

# Install and reload drivers
sudo ./build.sh -c install --reload

# Run sanity check
cd ..
sudo ./sanity_check.sh
```

## Prerequisites

Install the basic packages required to build Linux kernel modules:

```shell
sudo apt-get update
sudo apt-get install -y git build-essential linux-headers-$(uname -r) pciutils
```

For Debian package builds, also install:

```shell
sudo apt-get install -y dkms dpkg-dev debhelper rsync
```

For PCIe accelerator devices, verify that the device is visible:

```shell
lspci -d 1ff4:
```

If there is no output, the PCIe device may not be linked up or enumerated correctly.

To display DEEPX device names in `lspci` on Ubuntu, update the PCI ID database:

```shell
sudo update-pciids
```

## Supported Products and PCI IDs

The table below lists officially released/supported products.

| Device ID String | Device ID | Mapped Product | Status |
| ---------------- | --------- | -------------- | ------ |
| DX_M1 | `0x0000` | DX-M1, DX-M1 M.2 Module | Legacy |
| M1 [Series] | `0x0100` | DX-M1, DX-M1 M.2 Module | - |
| M1M [Series] | `0x0110` | DX-M1M, DX-M1M M.2 Module | - |
| M1 [H1] | `0x0101` | DX-H1 Quattro | - |

Example `lspci` output after updating PCI IDs:

```shell
0b:00.0 Processing accelerators: DEEPX Co., Ltd. DX_M1
0b:00.0 Processing accelerators: DEEPX Co., Ltd. M1 [Series]
0b:00.0 Processing accelerators: DEEPX Co., Ltd. M1M [Series]
0b:00.0 Processing accelerators: DEEPX Co., Ltd. M1 [H1]
```

## Repository Layout

```text
modules/
  build.sh        Recommended build/install wrapper
  Makefile        Advanced/manual Kbuild entry
  rt/             dxrt_driver.ko
  pci_deepx/      dx_dma.ko
debian/           Debian packaging files
sanity/           Driver sanity check scripts and tools
```

## Build and Install

### Recommended: build.sh

`modules/build.sh` is the recommended build and install interface. Run it from
the `modules/` directory.

```shell
cd ./modules

# Build modules
./build.sh

# Clean build outputs
./build.sh -c clean

# Install modules
sudo ./build.sh -c install

# Install and reload drivers
sudo ./build.sh -c install --reload

# Uninstall modules
sudo ./build.sh -c uninstall
```

Common options:

| Option | Description |
| ------ | ----------- |
| `-d, --device` | Select target device, default: `m1` |
| `-m, --module` | Select PCIe module, default: `deepx` |
| `-k, --kernel` | Kernel build directory |
| `-a, --arch` | Target architecture for cross compilation |
| `-t, --compiler` | Cross compiler prefix |
| `-i, --install` | Install path |
| `-c, --command` | `clean`, `install`, `uninstall`, `debian-package`, `install-package`, `uninstall-package` |
| `-p, --pkg-version` | Debian package version to install |
| `-j, --jops` | Parallel build jobs |
| `-f, --debug` | Enable debug feature (`debugfs`, `log`, or `all`) |
| `-v, --verbose` | Verbose build output |
| `--reload` | Reload drivers after install |

For the full option list:

```shell
cd ./modules
./build.sh --help
```

`--reload` stops related services/processes, unloads existing modules, and loads the newly installed modules. Use it only when it is safe to interrupt running applications.

### Advanced: Makefile

Use Makefile directly only when you need low-level Kbuild control.

```shell
cd modules

# Build
make DEVICE=m1 PCIE=deepx

# Install
sudo make DEVICE=m1 PCIE=deepx install

# Clean
make DEVICE=m1 PCIE=deepx clean
```

Cross-compile example:

```shell
cd modules
make DEVICE=m1 PCIE=deepx \
    ARCH=arm64 \
    CROSS_COMPILE=<path>/aarch64-linux-gnu- \
    KERNEL_DIR=<kernel-build-dir>
```

## Debian Package

Build a DKMS-based Debian package:

```shell
cd ./modules
./build.sh -c debian-package
```

Package output:

```text
../release/<VERSION>/dxrt-driver-dkms_<VERSION>-2_all.deb
```

Install the latest or a specific package version:

```shell
cd ./modules
sudo ./build.sh -c install-package
sudo ./build.sh -c install-package -p 2.1.0
```

After installation, DKMS builds the modules for the current kernel, checks the
`virt_dma` dependency, loads `virt_dma`, `dx_dma`, and `dxrt_driver`, and rebuilds
the modules automatically on kernel updates.

Remove the DKMS package:

```shell
cd ./modules
sudo ./build.sh -c uninstall-package
sudo dpkg --purge dxrt-driver-dkms
```

DKMS package installation conflicts with manual installation. If DKMS package is
installed, remove it before using manual install:

```shell
sudo dpkg -r dxrt-driver-dkms
```

## Module Loading

Manual installation installs modules under `/lib/modules/$(KERNELRELEASE)/extra/`.
After manual changes, update module dependencies and load the PCIe driver:

```shell
sudo depmod -A
sudo modprobe dx_dma
sudo modprobe dxrt_driver
```

Manual install also installs `dx_dma.conf`, which declares `softdep dx_dma post:
dxrt_driver`. If that configuration is present, `modprobe dx_dma` loads
`dxrt_driver` afterward. To be explicit, run both commands. The `--reload`
option handles module unload/load automatically after install.

## Driver Sanity Check

Run the sanity check from the project root:

```shell
sudo ./sanity_check.sh
```

This wrapper script calls `sanity/sanity.sh` and verifies:

1. PCIe link-up for DEEPX devices (`vendor ID 1ff4`)
2. `/dev/dxrt*` device files
3. `dxrt_driver` and `dx_dma` kernel modules
4. Driver installation state
5. PCIe ioctl communication

Results are saved to `sanity/result/`.

On failure, provide the generated files in `sanity/result/` to DEEPX support,
including `sanity_check_result_*.log`, `dmesg_*.log`, `pcie_*.log`, and
`dxrt_issue_bundle_*.tar.gz` if present
