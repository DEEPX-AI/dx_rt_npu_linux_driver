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

all: check_kbuild_config
	$(MAKE) -C $(KERNEL_DIR) M=$(MODULE_DIR) $(KERNEL_ARCH) modules

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
