format ELF

use32
extrn kernel_main
public _start

CODE_SEGMENT equ 0x08
DATA_SEGMENT equ 0x10

section '.text'

_start:
    mov ax, DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ebp, 0x200000
    mov esp, ebp

	call kernel_main
    jmp $

times 512-($ - $$) db 0
