# DXRT Driver Sanity Check Tools

This directory contains tools to verify the DXRT driver functionality.

## Programs

### pcie_test

Tests basic PCIe driver communication through ioctl commands:

1. **IDENTIFY_DEVICE** - Get device information (type, memory, DMA channels, etc.)
2. **WRITE_MEM** - Write test pattern to device memory
3. **READ_MEM** - Read back data and verify integrity

## Build

```bash
make
```

## Usage

### Manual Testing

```bash
# Test default device (/dev/dxrt0)
sudo ./pcie_test

# Test specific device
sudo ./pcie_test /dev/dxrt1
```

### Automated Testing (via Sanity.sh)

The pcie_test program is automatically called by the main sanity check script:

```bash
cd ..
sudo ./Sanity.sh
```

The sanity script will:
- Build pcie_test if not already built
- Run all driver verification checks
- Execute PCIe communication test
- Generate detailed logs in `result/` directory

## Requirements

- DXRT driver must be loaded (`lsmod | grep dxrt`)
- Device file must exist (`ls -l /dev/dxrt*`)
- PCIe device must be detected (`lspci | grep 1ff4`)
- Root privileges required for memory access

## Test Sequence

1. **Open Device** - Opens `/dev/dxrt*` device file
2. **Identify Device** - Retrieves device information via DXRT_CMD_IDENTIFY_DEVICE
3. **Write Memory** - Writes 4KB test pattern via DXRT_CMD_WRITE_MEM
4. **Read Memory** - Reads back and verifies data via DXRT_CMD_READ_MEM

## Exit Codes

The program returns specific exit codes to indicate which test failed:

- **0** - All tests passed successfully
- **1** - Failed to open device file
- **2** - IDENTIFY_DEVICE ioctl failed
- **3** - WRITE_MEM ioctl failed
- **4** - READ_MEM ioctl failed
- **5** - Data verification failed (read/write mismatch)

This allows automated scripts to determine exactly which operation failed.

## Output

Success:
```
================================================================================
  Test Summary
================================================================================
✓ All tests PASSED

The DXRT driver is working correctly.
PCIe communication is functioning properly.
```

Failure:
```
================================================================================
  Test Summary
================================================================================
✗ N test(s) FAILED

Please check:
  1. Driver is loaded (lsmod | grep dxrt)
  2. Device file exists (/dev/dxrt0)
  3. Permissions are correct (666)
  4. PCIe device is detected (lspci | grep 1ff4)
```

## Integration with Sanity Check

This program complements the driver sanity check script:

```bash
# Run full sanity check (driver + PCIe test)
cd ..
sudo ./sanity_check.sh

# Run PCIe communication test only
cd sanity
sudo ./pcie_test
```

## Troubleshooting

If tests fail:

1. Check kernel logs: `dmesg | tail -50`
2. Verify driver: `lsmod | grep -E 'dxrt|dx_dma'`
3. Check device: `ls -l /dev/dxrt*`
4. Check PCIe: `lspci -vvv -s $(lspci | grep 1ff4 | awk '{print $1}')`
5. Review test output for specific error messages

## Clean

```bash
make clean
```
