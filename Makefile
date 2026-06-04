CODEDIRS=. src/kernel src/kernel/arch/i386 src/kernel/klib
INCDIRS=. src/include src/include/arch/i386 src/include/klib

ASM = nasm
ASMFLAGS = -f elf32

CC = gcc
DEPFLAGS = -MP -MD
NOFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS = -m32 -Wall -Wextra -Werror -Wno-error=unused-variable -g $(foreach D, $(INCDIRS), -I$(D)) $(DEPFLAGS) $(NOFLAGS)

LDFLAGS = -T linker.ld -melf_i386

CFILES = $(foreach D, $(CODEDIRS), $(wildcard $(D)/*.c))
OBJECTS = $(patsubst %.c, %.o, $(CFILES)) src/boot/boot.o
DEPFILES = $(patsubst %.c, %.d, $(CFILES))

-include $(DEPFILES)

.PHONY: all clean runqemu

all: kernel.elf
	mkdir -p iso/boot/grub
	cp grub.cfg iso/boot/grub/grub.cfg
	cp kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o kdbOS.iso iso -d /usr/lib/grub/i386-pc

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

runqemu: all
	qemu-system-i386 -cdrom kdbOS.iso

clean:
	rm -rf src/boot/*.o
	rm -rf src/kernel/*.o src/kernel/*.d
	rm -rf src/kernel/arch/i386/*.o src/kernel/arch/i386/*.d
	rm -rf src/kernel/klib/*.o src/kernel/klib/*.d
	rm -rf kernel.elf kdbOS.iso iso/
