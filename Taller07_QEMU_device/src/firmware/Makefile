# Makefile para compilar firmware RISC-V mínimo

CROSS_COMPILE = riscv64-unknown-elf-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

MARCH = -march=rv32i -mabi=ilp32
CFLAGS = $(MARCH) -nostdlib -fno-builtin -ffunction-sections -fdata-sections \
         -Wall -Wextra -O2 -isystem/usr/lib/gcc/riscv64-unknown-elf/10.2.0/include

# Linker flags passed through GCC driver
LDFLAGS = -Wl,-T,linker.ld -Wl,--gc-sections -Wl,--print-memory-usage -Wl,-Map=firmware.map

SOURCES = start.S main.c app.c devices.c
OBJECTS = start.o main.o app.o devices.o

TARGET = firmware.elf
BINARY = firmware.bin

.PHONY: all clean disasm

all: $(TARGET) $(BINARY)

$(TARGET): $(OBJECTS)
	$(CC) $(MARCH) -nostdlib $(LDFLAGS) -o $@ $^ -lgcc

$(BINARY): $(TARGET)
	$(OBJCOPY) -O binary $< $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET) $(BINARY) firmware.map

disasm: $(TARGET)
	$(CROSS_COMPILE)objdump -d $(TARGET)
