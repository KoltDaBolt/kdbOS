rm kdbOS.iso build/boot.bin

nasm -f bin src/boot.asm -o build/boot.bin

current_size=$(stat --format=%s build/boot.bin)
padding_size=$((2048 - current_size))
if [ $padding_size -gt 0 ]; then
	dd if=/dev/zero bs=1 count=$padding_size >> build/boot.bin
fi

mkisofs -o kdbOS.iso -b build/boot.bin -no-emul-boot -boot-load-size 4 .
