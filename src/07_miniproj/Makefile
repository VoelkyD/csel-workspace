# Nom du compilateur croisé pour user-space
CROSS_COMPILE := /buildroot/output/host/usr/bin/aarch64-buildroot-linux-gnu-
CC := $(CROSS_COMPILE)gcc

# Variables pour le module noyau
CPU := arm64
KDIR := /buildroot/output/build/linux-5.15.148/
MODPATH := /buildroot/output/target
PWD := $(shell pwd)

# Nom du module
obj-m := miniproj.o
miniproj-objs := miniproj.o

# Fichiers user-space
USER_SRCS := fanctl.c daemon_fan.c
USER_BINS := $(USER_SRCS:.c=.o)

# Partie noyau
ifneq ($(KERNELRELEASE),)

# build module à partir des .o déclarés
else

all: modules $(USER_BINS)

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(CROSS_COMPILE) modules

# compilation fichiers user-space
%.o: %.c
	$(CC) -Wall $< -o $@

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(USER_BINS)

install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install

endif
