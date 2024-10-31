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
