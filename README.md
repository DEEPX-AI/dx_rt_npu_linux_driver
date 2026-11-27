<div align="center">

# DEEPX NPU Linux Driver

**Linux kernel driver for DEEPX NPUs.**

[![Release](https://img.shields.io/github/v/release/DEEPX-AI/dx_rt_npu_linux_driver)](https://github.com/DEEPX-AI/dx_rt_npu_linux_driver/releases)

</div>

It builds the NPU driver (`dxrt_driver`) and the PCIe driver (`dx_dma`) that
[DX-RT](https://github.com/DEEPX-AI/dx_rt) uses to reach the NPU. DX-RT v3.5.0 requires driver v2.7.0 or later.
It is part of [DX-Runtime](https://github.com/DEEPX-AI/dx-runtime), the target-side bundle of
[DX-AllSuite](https://github.com/DEEPX-AI/dx-all-suite). The DX-Runtime installer and the APT package
`dxrt-driver-dkms` install this driver for you; use this repository to build it from source or for a custom kernel.

## What You Need

| | |
|---|---|
| **NPU** | DX-M1, DX-M1M or DX-H1 Quattro (see [Supported Products and PCI IDs](#supported-products-and-pci-ids)) |
| **OS** | Linux with kernel headers for the running kernel (`linux-headers-$(uname -r)`) |
| **Build tools** | `build-essential`; for Debian packages also `dkms dpkg-dev debhelper rsync` |
| **DX-RT** | v3.5.0 requires this driver v2.7.0 or later |

## Quick Start

```shell
# Install build dependencies
sudo apt-get update
sudo apt-get install -y git build-essential linux-headers-$(uname -r) pciutils

# Get the source
git clone https://github.com/DEEPX-AI/dx_rt_npu_linux_driver.git
cd dx_rt_npu_linux_driver

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

If `lspci -d 1ff4:` prints nothing, the PCIe device may not be linked up or enumerated correctly.

## Supported Products and PCI IDs

The table below lists officially released/supported products.

| Device ID String | Device ID | Mapped Product | Status |
| ---------------- | --------- | -------------- | ------ |
| DX_M1 | `0x0000` | DX-M1, DX-M1 M.2 Module | Legacy |
| M1 [Series] | `0x0100` | DX-M1, DX-M1 M.2 Module | - |
| M1M [Series] | `0x0110` | DX-M1M, DX-M1M M.2 Module | - |
| M1 [H1] | `0x0101` | DX-H1 Quattro | - |

Example `lspci` output after updating the PCI ID database (`sudo update-pciids`):

```shell
0b:00.0 Processing accelerators: DEEPX Co., Ltd. DX_M1
0b:00.0 Processing accelerators: DEEPX Co., Ltd. M1 [Series]
0b:00.0 Processing accelerators: DEEPX Co., Ltd. M1M [Series]
0b:00.0 Processing accelerators: DEEPX Co., Ltd. M1 [H1]
```

<details>
<summary><b>Build Options: build.sh, Makefile, Repository Layout</b></summary>

**Repository layout**

```text
modules/
  build.sh        Recommended build/install wrapper
  Makefile        Advanced/manual Kbuild entry
  rt/             dxrt_driver.ko
  pci_deepx/      dx_dma.ko
debian/           Debian packaging files
release/          Prebuilt DKMS packages by version (latest -> newest)
sanity/           Driver sanity check scripts and tools
```

**build.sh (recommended)**

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

**Makefile (advanced)**

Use Makefile directly only when you need low-level Kbuild control.

```shell
cd ./modules

# Build
make DEVICE=m1 PCIE=deepx

# Install
sudo make DEVICE=m1 PCIE=deepx install

# Clean
make DEVICE=m1 PCIE=deepx clean
```

Cross-compile example:

```shell
cd ./modules
make DEVICE=m1 PCIE=deepx \
    ARCH=arm64 \
    CROSS_COMPILE=<path>/aarch64-linux-gnu- \
    KERNEL_DIR=<kernel-build-dir>
```

</details>

<details>
<summary><b>Debian Package (DKMS)</b></summary>

Build a DKMS-based Debian package (needs `dkms dpkg-dev debhelper rsync`):

```shell
cd ./modules
./build.sh -c debian-package
```

Package output:

```text
../release/<VERSION>/dxrt-driver-dkms_<VERSION>-2_all.deb
```

Prebuilt packages for each release are already in `release/`, so you can install one without building.
Install the latest or a specific package version:

```shell
cd ./modules
sudo ./build.sh -c install-package
sudo ./build.sh -c install-package -p 2.7.0
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

</details>

<details>
<summary><b>Module Loading and Sanity Check</b></summary>

**Module loading**

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

**Sanity check**

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
`dxrt_issue_bundle_*.tar.gz` if present.

</details>

## Documentation & Support

- Changes in each version: [Release Notes](./RELEASE_NOTES.md)
- For technical questions, please ask in the DEEPX Developer Community: https://community.deepx.ai/c/support/5

## License

See [LICENSE](./LICENSE) for terms of use.

Copyright © DEEPX Co., Ltd.
