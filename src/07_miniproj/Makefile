# Part executed when called from kernel build system
ifneq ($(KERNELRELEASE),)
obj-m += miniproj.o   ## name of the generated module
mymodule-objs := miniproj.o  ## list of objets needed by that module
CFLAGS_skeleton := -DDEBUG   ## to enable printing of debugging messages

# Part executed when called from standard make in the module source directory
else
CPU := arm64
KDIR := /buildroot/output/build/linux-5.15.148/
TOOLS := /buildroot/output/host/usr/bin/aarch64-buildroot-linux-gnu-
PWD := $(shell pwd)
#MODPATH := /rootfs
MODPATH := /buildroot/output/target

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)

install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install

endif
