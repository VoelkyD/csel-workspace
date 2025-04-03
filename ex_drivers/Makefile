# Cross compiler path (change if needed)
CC := /buildroot/output/host/usr/bin/aarch64-buildroot-linux-gnu-gcc

# Nom du binaire final
TARGET := read_chipid

# Sources
SRC := read_chipid.c

# Compilation flags
CFLAGS := -Wall

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
