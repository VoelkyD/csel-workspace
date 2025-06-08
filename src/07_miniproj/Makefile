# === Kernel module configuration ===
ifneq ($(KERNELRELEASE),)

obj-m := miniproj.o

# === User-space build ===
else

# --- Toolchain & Paths ---
CPU := arm64
KDIR := /buildroot/output/build/linux-5.15.148
TOOLS := /buildroot/output/host/usr/bin/aarch64-buildroot-linux-gnu-
CROSS := $(TOOLS)gcc
PWD := $(shell pwd)
MODPATH := /buildroot/output/target

# --- User-space sources ---
OLED_SRCS := oled/ssd1306.c
OLED_OBJS := $(OLED_SRCS:.c=.o)

# --- Default target ---
all: miniproj.ko fanctl.o daemon_fan.o

# --- Kernel module build ---
miniproj.ko:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules

# --- fanctl build ---
fanctl.o: fanctl.c
	$(CROSS) -Wall fanctl.c -o fanctl.o

# --- OLED .o compilation ---
%.o: %.c
	$(CROSS) -Wall -c $< -o $@

# --- daemon_fan build ---
daemon_fan.o: daemon_fan.c $(OLED_OBJS)
	$(CROSS) -Wall daemon_fan.c $(OLED_OBJS) -o daemon_fan.o

# --- Clean ---
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) *.o *.mod.c *.ko *.symvers *.order fanctl.o daemon_fan.o $(OLED_OBJS)

# --- Install kernel module ---
install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install

endif