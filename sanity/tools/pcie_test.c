// SPDX-License-Identifier: GPL-2.0
/*
 * DEEPX Driver Sanity Check Program
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 * This program tests the basic functionality of the DXRT driver
 * by performing the following operations:
 *  1. IDENTIFY_DEVICE - Get device information
 *  2. WRITE_MEM       - Write test data to device memory
 *  3. READ_MEM        - Read back data and verify
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

/* Exit codes */
#define EXIT_SUCCESS           0
#define EXIT_DEVICE_OPEN_FAIL  1
#define EXIT_IDENTIFY_FAIL     2
#define EXIT_WRITE_MEM_FAIL    3
#define EXIT_READ_MEM_FAIL     4
#define EXIT_VERIFY_FAIL       5

/* Driver definitions */
#define DXRT_IOCTL_MAGIC     'D'

typedef enum {
    DXRT_CMD_IDENTIFY_DEVICE    = 0,
    DXRT_CMD_WRITE_MEM          = 7,
    DXRT_CMD_READ_MEM           = 8,
} dxrt_cmd_t;

typedef enum {
    DX_IDENTIFY_NONE        = 0,
    DX_IDENTIFY_FWUPLOAD    = 1,
} dxrt_ident_sub_cmd_t;

typedef struct {
    int32_t     cmd;
    int32_t     sub_cmd;
    void*       data;
    uint32_t    size;
} dxrt_message_t;

typedef struct {
    uint32_t type;
    uint32_t variant;
    uint64_t mem_addr;
    uint64_t mem_size;
    uint32_t num_dma_ch;
    uint16_t fw_ver;
    uint16_t bd_rev;
    uint16_t bd_type;
    uint16_t ddr_freq;
    uint16_t ddr_type;
    uint16_t interface;
} dxrt_device_info_t;

typedef struct {
    uint64_t data;
    uint64_t base;
    uint32_t offset;
    uint32_t size;
    uint32_t ch;
} dxrt_req_meminfo_t;

#define DXRT_IOCTL_MESSAGE _IOW(DXRT_IOCTL_MAGIC, 0, dxrt_message_t)

/* Test configuration */
#define TEST_DATA_SIZE      (1024 * 4)  // 4KB test data
#define TEST_PATTERN_1      (0xA5A5A5A5)
#define TEST_PATTERN_2      (0x5A5A5A5A)

/* Color codes for output */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"

/* Global variables */
static int g_fd = -1;
static dxrt_device_info_t g_dev_info;

/* Utility functions */
void print_header(const char *title) {
    printf("\n");
    printf("================================================================================\n");
    printf("  %s\n", title);
    printf("================================================================================\n");
}

void print_result(const char *test_name, int success) {
    if (success) {
        printf("[%s✓%s] %s\n", COLOR_GREEN, COLOR_RESET, test_name);
    } else {
        printf("[%s✗%s] %s\n", COLOR_RED, COLOR_RESET, test_name);
    }
}

void print_info(const char *format, ...) {
    va_list args;
    printf("%s[INFO]%s ", COLOR_BLUE, COLOR_RESET);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void print_error(const char *format, ...) {
    va_list args;
    printf("%s[ERROR]%s ", COLOR_RED, COLOR_RESET);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void print_warning(const char *format, ...) {
    va_list args;
    printf("%s[WARN]%s ", COLOR_YELLOW, COLOR_RESET);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

/* Test functions */
int test_open_device(const char *dev_path) {
    print_header("Test 1: Open Device");
    
    g_fd = open(dev_path, O_RDWR);
    if (g_fd < 0) {
        print_error("Failed to open %s: %s", dev_path, strerror(errno));
        print_result("Open device", 0);
        return -1;
    }
    
    print_info("Device opened successfully: %s (fd=%d)", dev_path, g_fd);
    print_result("Open device", 1);
    return 0;
}

int test_identify_device(void) {
    print_header("Test 2: Identify Device (DXRT_CMD_IDENTIFY_DEVICE)");
    
    dxrt_message_t msg;
    memset(&msg, 0, sizeof(msg));
    memset(&g_dev_info, 0, sizeof(g_dev_info));
    
    msg.cmd = DXRT_CMD_IDENTIFY_DEVICE;
    msg.sub_cmd = DX_IDENTIFY_NONE;
    msg.data = &g_dev_info;
    msg.size = sizeof(g_dev_info);
    
    if (ioctl(g_fd, DXRT_IOCTL_MESSAGE, &msg) < 0) {
        print_error("ioctl IDENTIFY_DEVICE failed: %s", strerror(errno));
        print_result("Identify device", 0);
        return -1;
    }
    
    print_info("Device Information:");
    print_info("  Type       : %s (%u)", 
        (g_dev_info.type == 0) ? "Accelerator" : "Standalone", g_dev_info.type);
    print_info("  Variant    : %u", g_dev_info.variant);
    print_info("  Memory     : 0x%lx - 0x%lx (%lu MB)", 
        g_dev_info.mem_addr, 
        g_dev_info.mem_addr + g_dev_info.mem_size,
        g_dev_info.mem_size / (1024 * 1024));
    print_info("  DMA Ch     : %u", g_dev_info.num_dma_ch);
    print_info("  FW Version : %u", g_dev_info.fw_ver);
    print_info("  Board Rev  : %u", g_dev_info.bd_rev);
    print_info("  Board Type : %u", g_dev_info.bd_type);
    print_info("  DDR Freq   : %u", g_dev_info.ddr_freq);
    print_info("  DDR Type   : %u", g_dev_info.ddr_type);
    print_info("  Interface  : %u", g_dev_info.interface);
    
    print_result("Identify device", 1);
    return 0;
}

int test_write_memory(void) {
    print_header("Test 3: Write Memory (DXRT_CMD_WRITE_MEM)");
    
    if (g_dev_info.mem_size == 0) {
        print_error("Device memory not available");
        print_result("Write memory", 0);
        return -1;
    }
    
    // Allocate and initialize test data
    uint32_t *test_data = (uint32_t *)malloc(TEST_DATA_SIZE);
    if (!test_data) {
        print_error("Failed to allocate test data buffer");
        print_result("Write memory", 0);
        return -1;
    }
    
    // Fill with test pattern
    for (int i = 0; i < TEST_DATA_SIZE / sizeof(uint32_t); i++) {
        test_data[i] = (i % 2) ? TEST_PATTERN_1 : TEST_PATTERN_2;
    }
    
    // Prepare memory info
    dxrt_req_meminfo_t meminfo;
    meminfo.data = (uint64_t)test_data;
    meminfo.base = g_dev_info.mem_addr;
    meminfo.offset = 0;
    meminfo.size = TEST_DATA_SIZE;
    meminfo.ch = 0;  // Use DMA channel 0
    
    // Prepare message
    dxrt_message_t msg;
    msg.cmd = DXRT_CMD_WRITE_MEM;
    msg.sub_cmd = 0;
    msg.data = &meminfo;
    msg.size = sizeof(meminfo);
    
    print_info("Writing %u bytes to device memory...", TEST_DATA_SIZE);
    print_info("  Base address: 0x%lx", meminfo.base);
    print_info("  Offset      : 0x%x", meminfo.offset);
    print_info("  DMA Channel : %u", meminfo.ch);
    
    if (ioctl(g_fd, DXRT_IOCTL_MESSAGE, &msg) < 0) {
        print_error("ioctl WRITE_MEM failed: %s", strerror(errno));
        free(test_data);
        print_result("Write memory", 0);
        return -1;
    }
    
    print_info("Write completed successfully");
    free(test_data);
    print_result("Write memory", 1);
    return 0;
}

int test_read_memory(void) {
    print_header("Test 4: Read Memory (DXRT_CMD_READ_MEM)");
    
    if (g_dev_info.mem_size == 0) {
        print_error("Device memory not available");
        print_result("Read memory", 0);
        return -1;
    }
    
    // Allocate read buffer
    uint32_t *read_data = (uint32_t *)calloc(1, TEST_DATA_SIZE);
    if (!read_data) {
        print_error("Failed to allocate read data buffer");
        print_result("Read memory", 0);
        return -1;
    }
    
    // Prepare memory info
    dxrt_req_meminfo_t meminfo;
    meminfo.data = (uint64_t)read_data;
    meminfo.base = g_dev_info.mem_addr;
    meminfo.offset = 0;
    meminfo.size = TEST_DATA_SIZE;
    meminfo.ch = 0;  // Use DMA channel 0
    
    // Prepare message
    dxrt_message_t msg;
    msg.cmd = DXRT_CMD_READ_MEM;
    msg.sub_cmd = 0;
    msg.data = &meminfo;
    msg.size = sizeof(meminfo);
    
    print_info("Reading %u bytes from device memory...", TEST_DATA_SIZE);
    print_info("  Base address: 0x%lx", meminfo.base);
    print_info("  Offset      : 0x%x", meminfo.offset);
    print_info("  DMA Channel : %u", meminfo.ch);
    
    if (ioctl(g_fd, DXRT_IOCTL_MESSAGE, &msg) < 0) {
        print_error("ioctl READ_MEM failed: %s", strerror(errno));
        free(read_data);
        print_result("Read memory", 0);
        return -1;
    }
    
    print_info("Read completed successfully");
    
    // Verify data
    int errors = 0;
    for (int i = 0; i < TEST_DATA_SIZE / sizeof(uint32_t); i++) {
        uint32_t expected = (i % 2) ? TEST_PATTERN_1 : TEST_PATTERN_2;
        if (read_data[i] != expected) {
            if (errors < 10) {  // Print first 10 errors
                print_error("Data mismatch at index %d: expected 0x%08x, got 0x%08x",
                    i, expected, read_data[i]);
            }
            errors++;
        }
    }
    
    free(read_data);
    
    if (errors > 0) {
        print_error("Data verification failed: %d errors found", errors);
        print_result("Read memory (verify)", 0);
        return -1;
    }
    
    print_info("Data verification successful: all %u bytes match", TEST_DATA_SIZE);
    print_result("Read memory (verify)", 1);
    return 0;
}

void cleanup(void) {
    if (g_fd >= 0) {
        close(g_fd);
        g_fd = -1;
    }
}

void print_usage(const char *prog_name) {
    printf("Usage: %s [device_path]\n", prog_name);
    printf("\n");
    printf("Options:\n");
    printf("  device_path  Path to DXRT device (default: /dev/dxrt0)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s              # Test /dev/dxrt0\n", prog_name);
    printf("  %s /dev/dxrt1   # Test /dev/dxrt1\n", prog_name);
    printf("\n");
}

int main(int argc, char *argv[]) {
    const char *dev_path = "/dev/dxrt0";
    
    // Parse arguments
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        dev_path = argv[1];
    }
    
    printf("\n");
    printf("================================================================================\n");
    printf("  DEEPX Driver Sanity Check\n");
    printf("================================================================================\n");
    printf("  Device: %s\n", dev_path);
    printf("  Date  : %s %s\n", __DATE__, __TIME__);
    printf("================================================================================\n");
    
    // Run tests
    if (test_open_device(dev_path) < 0) {
        cleanup();
        print_header("Test Summary");
        printf("%s✗ Open device FAILED%s\n", COLOR_RED, COLOR_RESET);
        printf("\nPlease check:\n");
        printf("  1. Driver is loaded (lsmod | grep dxrt)\n");
        printf("  2. Device file exists (%s)\n", dev_path);
        printf("  3. Permissions are correct (666)\n");
        printf("  4. PCIe device is detected (lspci | grep 1ff4)\n");
        printf("\nFor more information, check dmesg and system logs.\n");
        return EXIT_DEVICE_OPEN_FAIL;
    }
    
    if (test_identify_device() < 0) {
        cleanup();
        print_header("Test Summary");
        printf("%s✗ IDENTIFY_DEVICE FAILED%s\n", COLOR_RED, COLOR_RESET);
        printf("\nThe device was opened successfully, but identification failed.\n");
        printf("This may indicate a firmware or hardware issue.\n");
        return EXIT_IDENTIFY_FAIL;
    }
    
    if (test_write_memory() < 0) {
        cleanup();
        print_header("Test Summary");
        printf("%s✗ WRITE_MEM FAILED%s\n", COLOR_RED, COLOR_RESET);
        printf("\nMemory write operation failed.\n");
        printf("Check PCIe DMA channel and memory address configuration.\n");
        return EXIT_WRITE_MEM_FAIL;
    }
    
    if (test_read_memory() < 0) {
        cleanup();
        print_header("Test Summary");
        printf("%s✗ READ_MEM or VERIFICATION FAILED%s\n", COLOR_RED, COLOR_RESET);
        printf("\nMemory read operation or data verification failed.\n");
        printf("Check PCIe DMA channel and memory integrity.\n");
        return EXIT_READ_MEM_FAIL;
    }
    
    cleanup();
    
    // Print summary
    print_header("Test Summary");
    printf("%s✓ All tests PASSED%s\n", COLOR_GREEN, COLOR_RESET);
    printf("\nThe DXRT driver is working correctly.\n");
    printf("PCIe communication is functioning properly.\n");
    return EXIT_SUCCESS;
}
