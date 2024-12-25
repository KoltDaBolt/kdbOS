ASM = nasm

SRC_DIR = src
SRC_BOOT_DIR = boot
BIN_DIR = bin
ISO_IMAGE = kdbOS.iso

all: $(ISO_IMAGE)

$(ISO_IMAGE): $(BIN_DIR)/boot.bin $(BIN_DIR)/boot2.bin
	mkisofs -o $(ISO_IMAGE) -b $(BIN_DIR)/boot.bin -b $(BIN_DIR)/boot2.bin -no-emul-boot -boot-load-size 4 .

$(BIN_DIR)/boot2.bin: $(SRC_DIR)/$(SRC_BOOT_DIR)/boot2.asm
	$(ASM) -f bin $(SRC_DIR)/$(SRC_BOOT_DIR)/boot2.asm -o $(BIN_DIR)/boot2.bin

$(BIN_DIR)/boot.bin: $(SRC_DIR)/$(SRC_BOOT_DIR)/boot.asm
	$(ASM) -f bin $(SRC_DIR)/$(SRC_BOOT_DIR)/boot.asm -o $(BIN_DIR)/boot.bin

clean:
	rm -f $(ISO_IMAGE) $(BIN_DIR)/*.bin

.PHONY: all clean