# SPDX-License-Identifier: GPL-2.0
# Makefile for DeepX AI accelerators driver

ifneq ($(KERNELRELEASE),)
include Kbuild
else

current_dir := $(abspath $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))
include $(current_dir)/device.mk

KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
KERNEL_ARCH ?=
MODULE_DIR ?= $(shell pwd)

ifneq ($(ARCH),)
KERNEL_ARCH += ARCH=$(ARCH)
endif
ifneq ($(CROSS_COMPILE),)
KERNEL_ARCH += CROSS_COMPILE=$(CROSS_COMPILE)
endif

# rt version
VERSION_FILE = $(shell pwd)/../release.ver
VERSION_STRING := $(shell cat $(VERSION_FILE) | cut -d'v' -f2)
RT_VERSION_MAJOR := $(shell echo $(VERSION_STRING) | cut -d'.' -f1)
RT_VERSION_MINOR := $(shell echo $(VERSION_STRING) | cut -d'.' -f2)
RT_VERSION_PATCH := $(shell echo $(VERSION_STRING) | cut -d'.' -f3)

# pcie version
VERSION_FILE = $(shell pwd)/pci_deepx/release.ver
VERSION_STRING := $(shell cat $(VERSION_FILE) | cut -d'v' -f2)
PCIE_VERSION_MAJOR := $(shell echo $(VERSION_STRING) | cut -d'.' -f1)
PCIE_VERSION_MINOR := $(shell echo $(VERSION_STRING) | cut -d'.' -f2)
PCIE_VERSION_PATCH := $(shell echo $(VERSION_STRING) | cut -d'.' -f3)

all: check_kbuild_config
	@echo "** RT Driver Version : $(RT_VERSION_MAJOR).$(RT_VERSION_MINOR).$(RT_VERSION_PATCH) **"
	@echo "** PCie Driver Version : $(PCIE_VERSION_MAJOR).$(PCIE_VERSION_MINOR).$(PCIE_VERSION_PATCH) **"
	$(MAKE) -C $(KERNEL_DIR) M=$(MODULE_DIR) $(KERNEL_ARCH) modules \
		CCFLAGS="-DRT_VERSION_MAJOR=$(RT_VERSION_MAJOR) -DRT_VERSION_MINOR=$(RT_VERSION_MINOR) -DRT_VERSION_PATCH=$(RT_VERSION_PATCH) \
		-DPCIE_VERSION_MAJOR=$(PCIE_VERSION_MAJOR) -DPCIE_VERSION_MINOR=$(PCIE_VERSION_MINOR) -DPCIE_VERSION_PATCH=$(PCIE_VERSION_PATCH)"

install:
	$(MAKE) -C $(KERNEL_DIR) M=$(MODULE_DIR) $(KERNEL_ARCH) modules_install

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(MODULE_DIR) $(KERNEL_ARCH) clean

sparse:
	$(MAKE) -C $(KERNEL_DIR) M=$(MODULE_DIR) $(KERNEL_ARCH) C=1 W=1

check_kbuild_config:
	 @if [ ! -f "$(KERNEL_DIR)/include/generated/autoconf.h" -o ! -f "$(KERNEL_DIR)/include/config/auto.conf" ]; then \
		echo >&2 " ERROR:  Kernel configuration is invalid."; \
		echo >&2 "         missing  $(KERNEL_DIR)/include/generated/autoconf.h,"; \
		echo >&2 "                  $(KERNEL_DIR)/include/config/auto.conf"; \
		$(Q)test $(KERNEL_DIR)  = /lib/modules/$(shell uname -r)/build && (echo >&2 " ** Prease Install: sudo apt install --reinstall linux-headers-$$(uname -r) "); \
		$(Q)test $(KERNEL_DIR) != /lib/modules/$(shell uname -r)/build && (echo >&2 " ** RUN: 'make oldconfig && make prepare' on kernel src to fix it."); \
		echo >&2 ; \
        false; \
    fi
endif
