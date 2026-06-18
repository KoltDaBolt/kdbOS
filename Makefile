CODEDIRS = . \
			src/kernel \
			src/kernel/arch/i386 \
			src/kernel/arch/i386/drivers \
			src/kernel/arch/i386/interrupts \
			src/kernel/arch/i386/memory \
			src/kernel/arch/i386/tables \
			src/kernel/klibc \
			src/kernel/memory \
			src/kernel/vterm

INCDIRS = . \
			src/include \
			src/include/arch/i386 \
			src/include/arch/i386/drivers \
			src/include/arch/i386/interrupts \
			src/include/arch/i386/memory \
			src/include/arch/i386/tables \
			src/include/klibc \
			src/include/memory \
			src/include/vterm

ASM = nasm
ASMFLAGS = -f elf32

CC = gcc
DEPFLAGS = -MP -MD
NOFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS = -m32 -Wall -Wextra -Werror -Wno-error=unused-variable -g $(foreach D, $(INCDIRS), -I$(D)) $(DEPFLAGS) $(NOFLAGS)

LDFLAGS = -T linker.ld -melf_i386

CFILES = $(foreach D, $(CODEDIRS), $(wildcard $(D)/*.c))
ASMFILES = $(foreach D, $(CODEDIRS), $(wildcard $(D)/*.asm))
OBJECTS = $(patsubst %.c, %.o, $(CFILES)) $(patsubst %.asm, %.o, $(ASMFILES)) src/boot/boot.o
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
	qemu-system-i386 -m 4G -cdrom kdbOS.iso

clean:
	rm -rf src/boot/*.o
	rm -rf src/kernel/*.o src/kernel/*.d
	rm -rf src/kernel/arch/i386/*.o src/kernel/arch/i386/*.d
	rm -rf src/kernel/arch/i386/drivers/*.o src/kernel/arch/i386/drivers/*.d
	rm -rf src/kernel/arch/i386/interrupts/*.o src/kernel/arch/i386/interrupts/*.d
	rm -rf src/kernel/arch/i386/memory/*.o src/kernel/arch/i386/memory/*.d
	rm -rf src/kernel/arch/i386/tables/*.o src/kernel/arch/i386/tables/*.d
	rm -rf src/kernel/klibc/*.o src/kernel/klibc/*.d
	rm -rf src/kernel/memory/*.o src/kernel/memory/*.d
	rm -rf src/kernel/vterm/*.o src/kernel/vterm/*.d
	rm -rf kernel.elf kdbOS.iso iso/
