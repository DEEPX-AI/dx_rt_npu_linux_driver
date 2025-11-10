# RELEASE_NOTES
## v1.8.0 / 2025-09-12

### 1. Changed
- Support version suffix
- Allow to access model profiling region (npu_dma_addr - 0x100000)

### 2. Fixed

### 3. Added
- Support single MSI probe
- Support safe link-up on RaspberryPI5

## v1.7.1 / 2025-07-16
### 1. Changed
### 2. Fixed
- Fix device identification error
### 3. Added
- add uninstall script and utility functions for project cleanup

## v1.7.0 / 2025-07-07
### 1. Changed
### 2. Fixed
### 3. Added
- Implement command for getting PCIe Status from device.


## v1.6.0 / 2025-05-22
### 1. Changed
- dxrt_drv.c dxrt_drv_cdev.c dxrt_drv_message.c dxrt_drv_npu.c
- dxrt_drv.h dxrt_drv_common.h dxrt_drv_npu.h
### 2. Fixed
- Kbuild build.sh device.mk
### 3. Added
- dxrt_drv_npu_v3.c npu_reg_debug_DX_V3.h npu_reg_dma_DX_V3.h npu_reg_sys_DX_V3.h

## v1.5.0 / 2025-05-14
### 1. Changed
- None
### 2. Fixed
- None
### 3. Added
- Implements termiation api
  If the user wants to terminate normally, the corresponding API is called and the driver is notified of termination.

## v1.4.3 / 2025-05-12
### 1. Changed
- None
### 2. Fixed
- Fix dma contiguous memory allocation failure on ARM
### 3. Added
- None

## v1.4.2 / 2024-05-07
### 1. Changed
- Remove unused code
- Change the numger of dma channel(3->4)
### 2. Fixed
- Fix dma transaction fail with iommu
### 3. Added
- None

## v1.4.1 / 2025-03-20
### 1. Changed
- None
### 2. Fixed
- fix bounding error when multi-processes are running with multi-bounding options
### 3. Added
- None

## v1.4.0 / 2025-03-11
### 1. Changed
- None
### 2. Fixed
- None
### 3. Added
- Add Otp read / write api

## v1.3.3 / 2025-02-19
### 1. Changed
- None
### 2. Fixed
- Fixed DMA Error Issue on NXP i.MX8 (Kernel: v6.6.23)
  - Fixed an issue where incorrect offset values in the vector table caused problems in handling DMA done interrupts.
  - Modified the logic to update the configuration space's MSI data when it does not match the cached MSI data.
### 3. Added
- None

## v1.3.2 / 2024-12-19
### 1. Changed
- Implement auto recovery
- Add ddr bandwidth info into response data
### 2. Fixed
- None
### 3. Added
- Update resume function for power management

## v1.3.1 / 2024-12-19
### 1. Changed
- None
### 2. Fixed
- fix unknown return type of request_acc func
### 3. Added
- None

## v1.3.0 / 2024-12-10
### 1. Changed
- Update device max num(64)
### 2. Fixed
- None
### 3. Added
- implement recovery concept

## v1.1.0 / 2024-10-28
### 1. Changed
- Add driver version mechanism with DO
- Update firmware update command(default:device reset)
- Update lock mechanism for queue
### 2. Fixed
- Fix bounding bug for multi-process
### 3. Added
- Add npu reqeust / response command

## v1.0.7 / 2024-07-23
### 1. Changed
- Update hybrid interrupt mode.
- Update scheduler option(bound / priority / bandwidth)
### 2. Fixed
- Fix spinlock error
- Fixed host system hang issue when accessing PCIe
### 3. Added
- Add interrupt handshake logic for inference

## v1.0.7 / 2024-07-23
### 1. Changed
- Update hybrid interrupt mode.
- Update scheduler option(bound / priority / bandwidth)
### 2. Fixed
- Fix spinlock error
- Fixed host system hang issue when accessing PCIe
### 3. Added
- Add interrupt handshake logic for inference

## v1.0.7 / 2024-07-23
### 1. Changed
- Update hybrid interrupt mode.
- Update scheduler option(bound / priority / bandwidth)
### 2. Fixed
- Fix spinlock error
- Fixed host system hang issue when accessing PCIe
### 3. Added
- Add interrupt handshake logic for inference

## v1.0.0 / 2024-04-29
### 1. Changed
- DXNN Version2 architecture device driver
- Update pci_deepx kernel module(2.0.0).
  This module is only supported on Accleator devices.
### 2. Fixed
- None
### 3. Added
- Update RT kernel module(1.0.0).
  This is an integrated module that supports Accleator and Standalone Device.
