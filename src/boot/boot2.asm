org 0x7E00

code_segment equ gdt_kernel_code - gdt
data_segment equ gdt_kernel_data - gdt

mov ax, 0x0000
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7C00

memmap_entry equ 0x0500

call getMemoryMap
call printMemoryMap
call enter_protected

getMemoryMap:
    pusha

    mov di, 0x0504

    mov eax, 0xE820
    xor ebx, ebx
    mov ecx, 24
    mov edx, 0x534D4150
    xor bp, bp
    mov dword [es:di + 20], 1
    int 0x15
    jc short .failed
    mov edx, 0x534D4150
    cmp eax, edx
    jne short .failed
    test ebx, ebx
    je short .failed
    jmp short .jumpin

    .memory_loop:
        mov eax, 0xE820
        mov dword [es:di + 20], 1
        mov ecx, 24
        int 0x15
        jc short .done_mem_map
        mov edx, 0x534D4150

    .jumpin:
        jcxz .skip_entry
        cmp cl, 20
        jbe short .notext
        test byte [es:di + 20], 1
        je short .skip_entry

    .notext:
        mov eax, [es:di + 8]
        or eax, [es:di + 12]
        jz .skip_entry
        inc bp
        add di, 24

    .skip_entry:
        test ebx, ebx
        jne short .memory_loop

    .done_mem_map:
        mov [memmap_entry], bp
        clc
        popa
        ret

    .failed:
        stc
        ret

printMemoryMap:
    mov di, 0x0504
    mov cx, [memmap_entry]

    .print_entries:
        cmp cx, 0
        je .done_print_mem_map
    
        mov eax, [es:di + 4]
        call print_hex
        mov eax, [es:di]
        call print_hex
        mov al, ' '
        call print_char

        mov eax, [es:di + 12]
        call print_hex
        mov eax, [es:di + 8]
        call print_hex
        mov al, ' '
        call print_char

        mov eax, [es:di + 16]
        call print_hex

        mov al, 0x0D
        call print_char
        mov al, 0x0A
        call print_char

        add di, 24
        dec cx
        jmp .print_entries

    .done_print_mem_map:
        ret

enter_protected:
    cli
    mov ax, 0x00                 
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
.load_protected:
    cli
    lgdt [gdt_descriptor]

    in al, 0x92
    or al, 2
    out 0x92, al

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp code_segment:load32
hang:
    jmp hang   

include './src/asmlib/print.asm'

gdt:
gdt_null:
    dd 0x0
    dd 0x0

gdt_kernel_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00

gdt_kernel_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt - 1
    dd gdt

use32
load32:
    mov eax, 6    ; qemu value
    mov ecx, 100  ; qemu value
    mov edi, 0x100000
    call ata_lba_read

    jmp code_segment:0x100000

ata_lba_read:
    pusha

    mov ebx, eax

    mov edx, 0x1F6
    shr eax, 24
    or eax, 0xE0
    out dx, al

    mov edx, 0x1F2
    mov al, cl
    out dx, al

    mov edx, 0x1F3
    mov eax, ebx
    out dx, al

    mov edx, 0x1F4
    mov eax, ebx
    shr eax, 8
    out dx, al

    mov edx, 0x1F5
    mov eax, ebx
    shr eax, 16
    out dx, al

    mov edx, 0x1F7
    mov al, 0x20
    out dx, al

    .next_sector:
        push ecx

    .try_again:
        mov edx, 0x1F7
        in al, dx
        test al, 8
        jz .try_again
        mov ecx, 256
        mov edx, 0x1F0
        rep insw
        pop ecx
        loop .next_sector

        popa
        ret

times 2560 - ($-$$) db 0