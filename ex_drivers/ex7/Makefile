# Part executed when called from kernel build system
ifneq ($(KERNELRELEASE),)
obj-m += mymodule.o
mymodule-objs := skeleton.o
CFLAGS_skeleton := -DDEBUG

else
CPU := arm64
KDIR := /buildroot/output/build/linux-5.15.148/
TOOLS := /buildroot/output/host/usr/bin/aarch64-buildroot-linux-gnu-
PWD := $(shell pwd)
MODPATH := /buildroot/output/target

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules_install

endif