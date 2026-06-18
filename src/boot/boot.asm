BITS 32

global loader
extern kernel_main

MAGIC_NUMBER        equ 0x1BADB002
FLAGS               equ 0x0
CHECKSUM            equ -(MAGIC_NUMBER + FLAGS)

KERNEL_STACK_SIZE   equ 16384 ; (16 KB)

section .multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

section .text
loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE
    push ebx
    push eax

    call kernel_main
    
    hang:
        jmp hang

section .bss
align 16
kernel_stack:
    resb KERNEL_STACK_SIZE

section .note.GNU-stack noalloc
