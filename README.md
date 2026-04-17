# DEEPX Runtime NPU Linux driver
RT AI Accelerator(NPU) linux driver

## Download
This repository contains a submodule.
```console
$ git clone --recurse-submodules git@gh.deepx.ai:deepx/dx_rt_npu_linux_driver.git
```
***Update submodules***
```console
$ git submodule update --remote
```
## Prerequisites (Only Accelator Device with PCIe)
```
You need to check whether the accelerator device is recognized properly using the lspci command as shown below.
If there is no output from the command, there is a problem with PCIe Link-up.
Please contact to DeepX
$ lspci -vn | grep 1ff4
0b:00.0 1200: 1ff4:0000

Additionally, if you want to display the DeepX name in lspci, you can update the PCI DB using the following command.(Only Ubuntu)
$ sudo update-pciids
$ lspci
...
0b:00.0 Processing accelerators: DEEPX Co., Ltd. DX_M1
```

## Structure
Support Linux kernel Kbuild system.

```console
- .gitmodules

- [modules]
    |
    - device.mk
    - kbuild
    - Makefile
    - build.sh (main build script - refactored and modularized)
    - [rt]
        - Kbuild
    - [pci_deepx] : submodule
        - Kbuild
- [debian]
    - control, rules, postinst, prerm (Debian packaging files)

```
***device.mk***
- set up configuration for supported devices, you can select device with 'DEVICE=[device]' macro
```console
$ make DEVICE=[device]
```
- For a device like m1, you need to select a submodule(PCIe) that has a dependency on m1.
```console
$ make DEVICE=m1 PCIE=deepx
```

| DEVICE | CONFIG                                        |
|--------|-----------------------------------------------|
| m1     | CONFIG_DX_AI_ACCEL_M1=y                       |
|        | submodule: CONFIG_DX_AI_ACCEL_PCIE_DEEPX      |

- For a device like v3, set DEVICE to v3.
```console
$ make DEVICE=v3
```

| DEVICE | CONFIG                                        |
|--------|-----------------------------------------------|
| v3     | CONFIG_DX_AI_STAND_V3=y                       |

***Kbuild***
- Support Linux kernel Kbuild system.

***Makefile***
- This repository contains a Makefie. Makefile has the following Parameters:

| Parameter Name   | Description                                    |
|------------------|------------------------------------------------|
| DEVICE           | m1 refer to 'device.mk'                  |
| KERNEL_DIR       | Kernel Build Directory                         |
| INSTALL_MOD_PATH | [PATH]/lib/modules/$(KERNELRELEASE)/extra/     |
| ARCH             | Architecture Name                              |
| CROSS_COMPILE    | Cross Compiler                                 |

***build.sh***
- This is a shell script that supports building. This script runs the Makefile using the entered options.
- **Refactored and modularized** - functionality split into multiple scripts in `modules/scripts/` directory for better maintainability
```console
 Usage:
	build.sh <options>

 options:
	-d, --device   [device]      select target device: m1
	-m, --module   [module]      select PCIe module: deepx
	-k, --kernel   [kernel dir]  'KERNEL_DIR=[kernel dir]', The directory where the kernel source is located
	                             - default: /lib/modules/$(uname -r)/build)
	-a, --arch     [arch]        set 'ARCH=[arch]' Target CPU architecture for cross-compilation, default: $(uname -m)
	-t, --compiler [cross tool]  'CROSS_COMPILE=[cross tool]' cross compiler binary, e.g aarch64-linux-gnu-
	-i, --install  [install dir] 'INSTALL_MOD_PATH=[install dir]', module install directory
	                             - install to: [install dir]/lib/modules/[KERNELRELEASE]/extra/
	-c, --command  [command]     clean | install | uninstall | debian-package | install-package | uninstall-package
	                             - uninstall: Remove the module files installed on the host PC.
	                             - debian-package: Build Debian package (.deb)
	                             - install-package: Install the built Debian package
	                             - uninstall-package: Uninstall the Debian package
	-p, --pkg-version [version]  Specify package version for install-package (default: latest)
	-j, --jops     [jobs]        set build jobs
	-f, --debug    [debug]       set debug feature [debugfs | log | all]
	-v, --verbose                build verbose (V=1)
	    --reload                 reload drivers after install (force unload and reload)
	-h, --help                   show this help
```

***Modules***
- modules/rt -> dxrt_driver.ko
- modules/pci_deepx -> dx_dma.ko

## Build
You can build with the make(Makefile) or build.sh script.

### Compile : Makefile
If your target system is capable of self-compiling the Linux Kernel module, you can compile it with:

***build***
```console
e.g $ cd modules
e.g $ make DEVICE=m1 PCIE=deepx
```

***clean***
```console
e.g $ cd modules
e.g $ make DEVICE=m1 PCIE=deepx clean
```

***install***
- installed to : /lib/modules/$(KERNELRELEASE)/extra/
```console
e.g $ cd modules
e.g $ make DEVICE=m1 PCIE=deepx install
```

### Cross Compile : Makefile
If you have a cross-compilation environment for target system, you can compile with:

***build***
```console
e.g $ cd modules
e.g $ make DEVICE=m1 PCIE=deepx \
      ARCH=arm64 CROSS_COMPILE=<path>/aarch64-linux-gnu- \
	  KERNEL_DIR=/home/fpga/src/linux-5.15.120-generic
```

***clean***
```console
e.g $ cd modules
e.g $ make DEVICE=m1 PCIE=deepx \
      ARCH=arm64 CROSS_COMPILE=<path>/aarch64-linux-gnu- \
	  KERNEL_DIR=/home/fpga/src/linux-5.15.120-generic \
	  clean
```

***install***
- installed to : [path]/lib/modules/$(KERNELRELEASE)/extra/
```console
e.g $ cd modules
e.g $ make DEVICE=m1 PCIE=deepx \
      ARCH=arm64 CROSS_COMPILE=<path>/aarch64-linux-gnu- \
	  KERNEL_DIR=/home/fpga/src/linux-5.15.120-generic \
	  INSTALL_MOD_PATH=<path> install
```
### Compile : build.sh
If your target system is capable of self-compiling the Linux Kernel module, you can compile it with:
- defulat target device is 'm1' and submodule is 'deepx', you can select target device '-d [device]'

***build***
- Build m1 with submodule deepx : make DEVICE=m1 PCIE=deepx
```console
e.g $ ./build.sh
```
***clean***
```console
e.g $ ./build.sh -c clean
```

***install***
- installed to : /lib/modules/$(KERNELRELEASE)/extra/
- **Note**: By default, modules are installed but NOT automatically loaded. Use `--reload` flag to force reload.
```console
e.g $ sudo ./build.sh -c install
```

***install with reload***
- installed to : /lib/modules/$(KERNELRELEASE)/extra/
- automatically stops service, kills processes, unloads old modules, and loads new modules
```console
e.g $ sudo ./build.sh -c install --reload
```

***uninstall***
- Remove all installed modules files
```console
e.g $ sudo ./build.sh -c uninstall
```

### Cross Compile : build.sh
If you have a cross-compilation environment for target system, you can compile with:
- defulat target device is 'm1' and submodule is 'deepx', you can select target device '-d [device]'
- Cross compilation can be built by specifying options when executing the script or modifying the script file.

***modify script***
- $ vi build.sh
```console
BUILD_DEFAULT_DEVICE="m1"
BUILD_DEFAULT_PCIE="deepx"
BUILD_DEFAULT_KERNEL_DIR=""
BUILD_DEFAULT_ARCH=""
BUILD_DEFAULT_CROSS_COMPILE=""
BUILD_DEFAULT_INSTALL_DIR=""
```
- 'BUILD_DEFAULT_DEVICE' is equivalent to the '-d [device]' option.
- 'BUILD_DEFAULT_PCIE' is equivalent to the '-m [module]' option.
- 'BUILD_DEFAULT_KERNEL_DIR' is equivalent to the '-k [kernel dir]' option.
- 'BUILD_DEFAULT_ARCH' is equivalent to the '-a [arch]' option.
- 'BUILD_DEFAULT_CROSS_COMPILE' is equivalent to the '-t [cross tool]' option.
- 'BUILD_DEFAULT_INSTALL_DIR' is equivalent to the '-i [install dir]' option.

***build***
```console
e.g $ ./build.sh -a arm64 \
      -t <path>/aarch64-linux-gnu- \
	  -k /home/fpga/src/linux-5.15.120-generic
```

***clean***
```console
e.g $ ./build.sh -a arm64 \
      -t <path>/aarch64-linux-gnu- \
	  -k /home/fpga/src/linux-5.15.120-generic -c clean
```

***install***
- installed to : [path]/lib/modules/$(KERNELRELEASE)/extra/
```console
e.g $ sudo ./build.sh -a arm64 \
      -t <path>/aarch64-linux-gnu- \
	  -k /home/fpga/src/linux-5.15.120-generic -c install
```

## Auto Loading modules at Boot time
You can install manually or use the build.sh script to have the module load automatically at boot time.

### Manually
***install***
- installed to : /lib/modules/$(KERNELRELEASE)/extra/
```console
$ make DEVICE=m1 PCIE=deepx install
```
***modules.dep***
- update : /lib/modules/$(KERNELRELEASE)/modules.dep
```console
$ sudo depmod -A
```
***/etc/modprobe.d/[modules].conf***
- copy modules/dx_dma.conf to /etc/modprobe.d
```console
$ sudo cp modules/dx_dma.conf /etc/modprobe.d/
```
***test with modprobe***
- You can verify that it was installed correctly with the modprobe command.
```console
$ sudo modprobe dx_dma
$ lsmod
  dxrt_driver            40960  0
  dx_dma                176128  1 dxrt_driver
```

### build.sh script
***install***
- Handles all commands that need to be installed manually.
```console
$ ./build.sh -d m1 -m deepx -c install
- DEVICE        : m1
- PCIE          : deepx
- MODULE CONF   : /home/jhk/deepx/dxrt/module/rt_npu_linux_driver/modules/dx_dma.conf
- ARCH (HOST)   : x86_64
- KERNEL        : /lib/modules/5.15.0-102-generic/build
- INSTALL       : /lib/modules/5.15.0-102-generic/extra/

 *** Build : install ***
 $ make DEVICE=m1 PCIE=deepx install

make -C /lib/modules/5.15.0-102-generic/build M=/home/jhk/deepx/dxrt/module/rt_npu_linux_driver/modules  modules_install
 ....
 - SUCCESS

 *** Update : /lib/modules/5.15.0-102-generic/modules.dep ***
 $ depmod -A
 $ cp /home/jhk/deepx/dxrt/module/rt_npu_linux_driver/modules/dx_dma.conf /etc/modprobe.d/
```
***uninstall***
- Remove all installed modules files.
```console
$ ./build.sh -d m1 -m deepx -c uninstall
- DEVICE        : m1
- PCIE          : deepx
- MODULE CONF   : /home/jhk/deepx/dxrt/module/rt_npu_linux_driver/modules/dx_dma.conf
- ARCH (HOST)   : x86_64
- KERNEL        : /lib/modules/5.15.0-102-generic/build
- INSTALL       : /lib/modules/5.15.0-102-generic/extra/

 *** Remove : /lib/modules/5.15.0-102-generic/extra ***
 $ rm -rf /lib/modules/5.15.0-102-generic/extra/pci_deepx
 $ rm -rf /lib/modules/5.15.0-102-generic/extra/rt

 *** Remove : /etc/modprobe.d ***
 $ rm /etc/modprobe.d/dx_dma.conf

 *** Update : /lib/modules/5.15.0-102-generic/modules.dep ***
 $ depmod
```
## Debian Package Installation

### Build Debian Package
Build optimized Debian package (.deb) for DKMS-based installation:
```console
$ ./build.sh -c debian-package
```
- Package output: `release/<VERSION>/dxrt-driver-dkms_<VERSION>_all.deb`
- Package size: ~107KB (optimized, excludes utils/ and build artifacts)
- Includes only source files needed for DKMS compilation

### Install Debian Package
Install the latest or specific version of the Debian package:

***Install latest version***
```console
$ sudo ./build.sh -c install-package
```

***Install specific version***
```console
$ sudo ./build.sh -c install-package -p 2.1.0
```

**Post-installation:**
- DKMS automatically builds modules for current kernel
- Checks virt-dma dependency (CONFIG_DMA_VIRTUAL_CHANNELS)
- Loads modules: virt_dma → dx_dma → dxrt_driver
- Modules reload automatically on kernel updates

**Important Notes:**
- DKMS package installation conflicts with manual installation (`build.sh -c install`)
- If DKMS package is installed, remove it before manual installation:
  ```console
  $ sudo dpkg -r dxrt-driver-dkms
  ```

### Uninstall Debian Package
Remove the DKMS package and all installed modules:
```console
$ sudo ./build.sh -c uninstall-package
```

**Optional: Purge configuration files**
```console
$ sudo dpkg --purge dxrt-driver-dkms
```

## Driver Sanity Check

### Quick Start

Run the sanity check from the project root:

```console
$ sudo ./SanityCheck.sh
```

This wrapper script automatically calls `sanity/Sanity.sh` and provides comprehensive driver verification.

### What it Checks

1. **PCIe Link-up** - Verifies DEEPX device (vendor ID 1ff4) is detected
2. **Device Files** - Checks `/dev/dxrt*` existence, type, and permissions
3. **Kernel Modules** - Verifies `dxrt_driver` and `dx_dma` are loaded
4. **Driver Installation** - Checks legacy and DKMS installation status
5. **PCIe Communication** - Tests ioctl interface (IDENTIFY/WRITE_MEM/READ_MEM)

### Test Tools

The sanity check uses test programs in `sanity/tools/`:

**pcie_test** - PCIe driver ioctl verification:
- IDENTIFY_DEVICE: Get device info (memory, DMA channels, etc.)
- WRITE_MEM: Write 4KB test pattern to device memory
- READ_MEM: Read back and verify data integrity

To build test tools:
```console
$ cd sanity/tools
$ make
```

To run manually:
```console
$ sudo ./pcie_test           # Test /dev/dxrt0
$ sudo ./pcie_test /dev/dxrt1  # Test specific device
```

### Output

Results are saved to `sanity/result/`:
- `sanity_check_result_*.log` - Test results and status
- `dmesg_*.log` - Kernel messages (on failure)
- `pcie_*.log` - PCIe device information (on failure)

**Success:**
```
** Sanity check PASSED!
** Driver is properly installed and running.
** PCIe communication test: All ioctl commands verified successfully.
```

**Failure:**
```
** Sanity check FAILED! Check logs at: sanity/result/sanity_check_result_*.log
** Please report this result to DEEPX with logs
```