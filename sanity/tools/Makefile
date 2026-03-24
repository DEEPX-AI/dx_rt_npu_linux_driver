# SPDX-License-Identifier: GPL-2.0
# Makefile for DXRT driver sanity check tools

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../../modules/include
LDFLAGS = 

TARGETS = pcie_test

.PHONY: all clean install

all: $(TARGETS)

pcie_test: pcie_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS) *.o

install: all
	@echo "Sanity check tools built successfully"
	@echo "Run: sudo ./pcie_test [device_path]"

help:
	@echo "DXRT Driver Sanity Check Tools - Build Instructions"
	@echo ""
	@echo "Targets:"
	@echo "  make         - Build all sanity check tools"
	@echo "  make clean   - Remove built programs"
	@echo "  make install - Build and show usage"
	@echo ""
	@echo "Programs:"
	@echo "  pcie_test - PCIe driver ioctl test (IDENTIFY/WRITE_MEM/READ_MEM)"
	@echo "  make help    - Show this help message"
	@echo ""
	@echo "Programs:"
	@echo "  pcie_test    - Test PCIe driver basic functionality"
	@echo "                 (IDENTIFY, WRITE_MEM, READ_MEM)"
	@echo ""
	@echo "Usage:"
	@echo "  sudo ./pcie_test              # Test /dev/dxrt0"
	@echo "  sudo ./pcie_test /dev/dxrt1   # Test /dev/dxrt1"
