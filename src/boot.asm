org 0x7C00
bits 16

entry:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x5000

    cli
    call enableA20
    call getMemoryMap
    call printMemoryEntry
    call loadGDT
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp dword 0x08:.pmode

.pmode:
    [bits 32]

    jmp .halt

    mov ax, 0x10
    mov ds, ax
    mov ss, ax

    mov esi, Hello
    mov edi, ScreenBuffer
    cld

.loop:
    lodsb
    or al, al
    jz .done

    mov [edi], al
    inc edi

    mov [edi], byte 0x0F
    inc edi
    jmp .loop

.done:
    jmp .halt

.halt:
    jmp .halt

enableA20:
    [bits 16]

    call A20WaitInput
    mov al, KbdControllerDisableKeyboard
    out KbdControllerCommandPort, al

    call A20WaitInput
    mov al, KbdControllerReadCtrlOutputPort
    out KbdControllerCommandPort, al

    call A20WaitOutput
    in al, KbdControllerDataPort
    push eax

    call A20WaitInput
    mov al, KbdControllerWriteCtrlOutputPort
    out KbdControllerCommandPort, al

    call A20WaitInput
    pop eax
    or al, 2
    out KbdControllerDataPort, al

    call A20WaitInput
    mov al, KbdControllerEnableKeyboard
    out KbdControllerCommandPort, al

    call A20WaitInput
    ret

A20WaitInput:
    [bits 16]

    in al, KbdControllerCommandPort
    test al, 2
    jnz A20WaitInput
    ret

A20WaitOutput:
    [bits 16]

    in al, KbdControllerCommandPort
    test al, 1
    jz A20WaitOutput
    ret

getMemoryMap:
    [bits 16]

    memmap_entry equ 0x5000
    mov di, 0x5004

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
    jmp short .jmpin

    .memmap_loop:
        mov eax, 0xE820
        mov dword [es:di + 20], 1
        mov ecx, 24
        int 0x15
        jc short .done_mem_map
        mov edx, 0x534D4150

    .jmpin:
        jcxz .skip_entry
        cmp cl, 20
        jbe short .no_text
        test byte [es:di + 20], 1
        je short .skip_entry
    
    .no_text:
        mov eax, [es:di + 8]
        or eax, [es:di + 12]
        jz .skip_entry
        inc bp
        add di, 24
    
    .skip_entry:
        test ebx, ebx
        jne short .memmap_loop
    
    .done_mem_map:
        mov [memmap_entry], bp
        clc
        ret

    .failed:
        stc
        ret

printMemoryEntry: ; Keep in mind, x86_64 is little endian
    [bits 16]

    mov di, 0x5004
    mov cx, [memmap_entry]

    .print_mem_loop:
        cmp cx, 0
        je .done_print_mem

        mov eax, [es:di + 4]
        call printHex
        mov al, ' '
        call printChar

        mov eax, [es:di]
        call printHex
        mov al, ' '
        call printChar

        mov eax, [es:di + 12]
        call printHex
        mov al, ' '
        call printChar

        mov eax, [es:di + 8]
        call printHex
        mov al, ' '
        call printChar

        mov eax, [es:di + 16]
        call printHex
        mov al, ' '
        call printChar
        
        mov al, 13
        call printChar
        mov al, 10
        call printChar

        add di, 24
        dec cx
        jmp .print_mem_loop
    
    .done_print_mem:
        ret

printChar:  ; Assumes character to print is in al
    [bits 16]

    mov ah, 0x0E
    int 0x10

    ret

printHex:   ; Assumes the number you want to print is in eax
    [bits 16]
    
    pusha
    mov ecx, 8
    mov ebx, eax

    .next_digit:
        rol ebx, 4
        mov al, bl
        and al, 0x0F
        cmp al, 10
        jl .print_digit
        add al, 'A' - 10
        jmp .print_hex_digit

    .print_digit:
        add al, '0'

    .print_hex_digit:
        mov ah, 0x0E
        int 0x10
        loop .next_digit

    popa
    ret

loadGDT:
    [bits 16]

    lgdt [GDTDescriptor]
    ret

KbdControllerDataPort               equ 0x60
KbdControllerCommandPort            equ 0x64
KbdControllerDisableKeyboard        equ 0xAD
KbdControllerEnableKeyboard         equ 0xAE
KbdControllerReadCtrlOutputPort     equ 0xD0
KbdControllerWriteCtrlOutputPort    equ 0xD1

ScreenBuffer                        equ 0xB8000

GDT:
    dq 0

    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 00001111b
    db 0

    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 00001111b
    db 0

GDTDescriptor:
    dw GDTDescriptor - GDT - 1
    dd GDT

Hello:    db "Hello from protected mode!", 0

times 510 - ($ - $$) db 0
dw 0xAA55