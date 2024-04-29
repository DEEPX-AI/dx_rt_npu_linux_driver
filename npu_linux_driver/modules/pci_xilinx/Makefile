# SPDX-License-Identifier: GPL-2.0
# DeepX Synopsys PCIe driver

ifneq ($(KERNELRELEASE),)
include Kbuild
else

export CONFIG_DX_AI_ACCEL_PCIE_XILINX=m

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
