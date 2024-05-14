# DEEPX Runtime PCIe Synopsys Linux driver
RT AI Accelerator(NPU) PCIe linux driver

## Module
**Kbuild Config**

```console
CONFIG_DX_AI_ACCEL_PCIE_DEEPX=m
```

**Module Name**
- dx_dma.ko

## Makefile
This repository contains a Makefie. Makefile has the following Parameters:

| Parameter Name   | Description                                    |
|------------------|------------------------------------------------|
| KERNEL_DIR       | Kernel Build Directory                         |
| INSTALL_MOD_PATH | [PATH]/lib/modules/$(KERNELRELEASE)/extra/     |
| ARCH             | Architecture Name                              |
| CROSS_COMPILE    | Cross Compiler                                 |

## Compile
If your target system is capable of self-compiling the Linux Kernel module, you can compile it with:

**build**
```console
e.g $ make
```

**clean**
```console
e.g $ make clean
```

**install**
```console
e.g $ make INSTALL_MOD_PATH=<path> install
- installed to : <path>/lib/modules/$(KERNELRELEASE)/extra/
```

## Cross Compile
If you have a cross-compilation environment for target system, you can compile with:

**build**
```console
e.g $ make ARCH=arm64 CROSS_COMPILE=<path>/aarch64-linux-gnu- KERNEL_DIR=/home/fpga/src/linux-5.15.120-generic
```

**clean**
```console
e.g $ make ARCH=arm64 CROSS_COMPILE=<path>/aarch64-linux-gnu- KERNEL_DIR=/home/fpga/src/linux-5.15.120-generic clean
```

**install**
```console
e.g $ make ARCH=arm64 CROSS_COMPILE=<path>/aarch64-linux-gnu- KERNEL_DIR=/home/fpga/src/linux-5.15.120-generic INSTALL_MOD_PATH=<path> install
- installed to : <path>/lib/modules/$(KERNELRELEASE)/extra/
```
