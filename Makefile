# This is a Makefile equivalent to the build.sh script if you prefer Makefiles

ASM = nasm

SRC_DIR = src
BUILD_DIR = build
ISO_IMAGE = kdbOS.iso

all: $(ISO_IMAGE)

# Rule to create the bootable ISO
$(ISO_IMAGE): $(BUILD_DIR)/boot.bin
	mkisofs -o $(ISO_IMAGE) -b $(BUILD_DIR)/boot.bin -no-emul-boot -boot-load-size 4 .

# Rule to compile boot.asm into a binary bootloader
$(BUILD_DIR)/boot.bin: $(SRC_DIR)/boot.asm
	mkdir -p $(BUILD_DIR)
	$(ASM) -f bin $(SRC_DIR)/boot.asm -o $(BUILD_DIR)/boot.bin
	# Calculate padding size to meet ISO standards (2048 bytes)
	current_size=$$(stat --format=%s $(BUILD_DIR)/boot.bin); \
	padding_size=$$((2048 - current_size)); \
	if [ $$padding_size -gt 0 ]; then \
		dd if=/dev/zero bs=1 count=$$padding_size >> $(BUILD_DIR)/boot.bin; \
	fi

clean:
	rm -f $(ISO_IMAGE) $(BUILD_DIR)/boot.bin

.PHONY: all clean