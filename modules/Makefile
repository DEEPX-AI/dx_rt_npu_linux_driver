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

ifneq ($(findstring rpi,$(KERNEL_DIR)),)
export CONFIG_RPI_BUILD=1
$(info ** RaspberryPI Build **)
else
$(info ** Standard Build **)
endif


ifneq ($(ARCH),)
KERNEL_ARCH += ARCH=$(ARCH)
endif
ifneq ($(CROSS_COMPILE),)
KERNEL_ARCH += CROSS_COMPILE=$(CROSS_COMPILE)
endif

# rt version
RT_VERSION_FILE = $(shell pwd)/../release.ver
RT_VERSION_STRING := $(shell cat $(RT_VERSION_FILE) | cut -d'v' -f2)
RT_VERSION_MAJOR := $(shell echo $(RT_VERSION_STRING) | cut -d'.' -f1)
RT_VERSION_MINOR := $(shell echo $(RT_VERSION_STRING) | cut -d'.' -f2)
RT_VERSION_PATCH := $(shell echo $(RT_VERSION_STRING) | cut -d'.' -f3 | cut -d'-' -f1)
# Extract optional suffix (empty if not present)
RT_VERSION_SUFFIX := $(shell echo $(RT_VERSION_STRING) | awk -F- 'NF>1{print $$2}')

# Only pass the macro to the compiler when suffix exists
RT_VERSION_SUFFIX_FLAG := $(strip $(if $(RT_VERSION_SUFFIX),-DRT_VERSION_SUFFIX=$(RT_VERSION_SUFFIX),))

# For pretty printing (include '-' only when suffix exists)
RT_VERSION_SUFFIX_PRINT := $(if $(RT_VERSION_SUFFIX),-$(RT_VERSION_SUFFIX),)

# pcie version
PCIE_VERSION_FILE = $(shell pwd)/pci_deepx/release.ver
PCIE_VERSION_STRING := $(shell cat $(PCIE_VERSION_FILE) | cut -d'v' -f2)
PCIE_VERSION_MAJOR := $(shell echo $(PCIE_VERSION_STRING) | cut -d'.' -f1)
PCIE_VERSION_MINOR := $(shell echo $(PCIE_VERSION_STRING) | cut -d'.' -f2)
PCIE_VERSION_PATCH := $(shell echo $(PCIE_VERSION_STRING) | cut -d'.' -f3)

# version defines
VERSION_FLAGS := \
	-DRT_VERSION_MAJOR=$(RT_VERSION_MAJOR) \
	-DRT_VERSION_MINOR=$(RT_VERSION_MINOR) \
	-DRT_VERSION_PATCH=$(RT_VERSION_PATCH) \
	$(RT_VERSION_SUFFIX_FLAG) \
	-DPCIE_VERSION_MAJOR=$(PCIE_VERSION_MAJOR) \
	-DPCIE_VERSION_MINOR=$(PCIE_VERSION_MINOR) \
	-DPCIE_VERSION_PATCH=$(PCIE_VERSION_PATCH)

define do_build
	$(MAKE) -C $(KERNEL_DIR) M=$(MODULE_DIR) $(KERNEL_ARCH) $(1) CCFLAGS="$(VERSION_FLAGS)"
endef

all: check_kbuild_config
	@echo "** RT Driver Version : $(RT_VERSION_MAJOR).$(RT_VERSION_MINOR).$(RT_VERSION_PATCH)$(RT_VERSION_SUFFIX_PRINT) **"
	@echo "** PCie Driver Version : $(PCIE_VERSION_MAJOR).$(PCIE_VERSION_MINOR).$(PCIE_VERSION_PATCH) **"
	$(call do_build,modules)

sparse:
	$(call do_build,modules C=1 W=1)

install:
	$(MAKE) -C $(KERNEL_DIR) M=$(MODULE_DIR) $(KERNEL_ARCH) modules_install

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(MODULE_DIR) $(KERNEL_ARCH) clean

check_kbuild_config:
	@if [ ! -f "$(KERNEL_DIR)/include/generated/autoconf.h" -o ! -f "$(KERNEL_DIR)/include/config/auto.conf" ]; then \
		echo >&2 " ERROR:  Kernel configuration is invalid."; \
		echo >&2 "         missing  $(KERNEL_DIR)/include/generated/autoconf.h,"; \
		echo >&2 "                  $(KERNEL_DIR)/include/config/auto.conf"; \
		$(Q)test $(KERNEL_DIR) = /lib/modules/$(shell uname -r)/build && (echo >&2 " ** Please Install: sudo apt install --reinstall linux-headers-$$(uname -r) "); \
		$(Q)test $(KERNEL_DIR) != /lib/modules/$(shell uname -r)/build && (echo >&2 " ** RUN: 'make oldconfig && make prepare' on kernel src to fix it."); \
		echo >&2 ; \
		false; \
	fi
endif
