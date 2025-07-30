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
    - build.sh
    - [rt]
        - Kbuild
    - [pci_deepx] : submodule
        - Kbuild

- [utils] : submodule
```
***device.mk***
- set up configuration for supported devices, you can select device with 'DEVICE=[device]' macro
```console
$ make DEVICE=[device]
```
- For a device like m1, you need to select a submodule(PCIe) that has a dependency on m1.
```console
$ make DEVICE=m1 PCIE=[deepx|xilinx]
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
```console
 Usage:
	build.sh <options>

 options:
	-d [device]	     select target device: m1
	-m [module]	     select PCIe module: deepx xilinx
	-k [kernel dir]	 'KERNEL_DIR=[kernel dir]', The directory where the kernel source is located
			         default: /lib/modules/5.15.0-100-generic/build)
	-a [arch]	     set 'ARCH=[arch]' Target CPU architecture for cross-compilation, default: x86_64
	-t [cross tool]	 'CROSS_COMPILE=[cross tool]' cross compiler binary, e.g aarch64-linux-gnu-
	-i [install dir] 'INSTALL_MOD_PATH=[install dir]', module install directory
			         install to: [install dir]/lib/modules/[KERNELRELEASE]/extra/
	-c [command]	 clean | install
	-j [jobs]	     set build jobs
	-v		         verbose (V=1)
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
- Build m1 with submodule xilinx : make DEVICE=m1 PCIE=xilinx
```console
e.g $ ./build.sh -d m1 -m xilinx
```
***clean***
```console
e.g $ ./build.sh -c clean
```

***install***
- installed to : /lib/modules/$(KERNELRELEASE)/extra/
```console
e.g $ sudo ./build.sh -c install
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
