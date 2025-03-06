ASM = fasm
GCC = i686-elf-gcc
LD = i686-elf-ld
CFLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label  -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

SRC_BOOT_DIR = src/boot
SRC_KERNEL_DIR = src/kernel
BIN_DIR = bin
BUILD_DIR = build
ISO_IMAGE = kdbOS.iso

init:
	mkdir -p bin build

all: $(ISO_IMAGE)

qemu: $(BIN_DIR)/os.bin

$(BIN_DIR)/os.bin: $(BIN_DIR)/boot.bin $(BIN_DIR)/boot2.bin $(BIN_DIR)/kernel.bin
	cat $(BIN_DIR)/boot.bin $(BIN_DIR)/boot2.bin $(BIN_DIR)/kernel.bin > $(BIN_DIR)/os.bin
	dd if=/dev/zero bs=512 count=100 >> $(BIN_DIR)/os.bin

$(ISO_IMAGE): $(BIN_DIR)/boot.bin $(BIN_DIR)/boot2.bin $(BIN_DIR)/kernel.bin
	genisoimage -o $(ISO_IMAGE) -b boot.bin -input-charset utf-8 -no-emul-boot $(BIN_DIR)

$(BIN_DIR)/kernel.bin: $(BUILD_DIR)/kernel.asm.o $(BUILD_DIR)/kernel.o
	$(LD) -T ./linker.ld -o $(BIN_DIR)/kernel.bin -nostdlib $(BUILD_DIR)/kernel.asm.o $(BUILD_DIR)/kernel.o

$(BUILD_DIR)/kernel.o: $(SRC_KERNEL_DIR)/kernel.c
	$(GCC) $(CFLAGS) -std=gnu99 -c $(SRC_KERNEL_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o

$(BUILD_DIR)/kernel.asm.o: $(SRC_KERNEL_DIR)/kernel.asm
	fasm $(SRC_KERNEL_DIR)/kernel.asm $(BUILD_DIR)/kernel.asm.o

$(BIN_DIR)/boot2.bin: $(SRC_BOOT_DIR)/boot2.asm
	fasm $(SRC_BOOT_DIR)/boot2.asm $(BIN_DIR)/boot2.bin

$(BIN_DIR)/boot.bin: $(SRC_BOOT_DIR)/boot.asm
	fasm $(SRC_BOOT_DIR)/boot.asm $(BIN_DIR)/boot.bin

clean:
	rm -f $(ISO_IMAGE) $(BIN_DIR)/*.bin $(BUILD_DIR)/*.o

.PHONY: all clean
