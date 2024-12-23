org 0x7C00
bits 16

entry:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    cli
    call enableA20
    call getMemoryMap
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

    mov ax, 0x5000
    mov es, ax
    mov di, 0

    mov eax, 0xE820
    mov ebx, 0
    mov edx, 0x534D4150
    mov ecx, 24
    mov bp, 0
    mov [es:di + 20], dword 1

    .memory_loop:
        int 0x15
        jc .done_mem_map
        cmp eax, edx
        jne .done_mem_map
        test ebx, ebx
        je .done_mem_map

        push di
        call printMemoryEntry
        pop di

        mov eax, 0xE820
        mov ecx, 24
        mov edx, 0x534D4150
        inc bp
        add di, 24
        test ebx, ebx
        jnz .memory_loop
    
    .done_mem_map:
        ret

printMemoryEntry:
    [bits 16]

    push ax
    push bx
    push cx
    push dx
    push si
    push di

    mov eax, [es:di]
    call printHex
    mov al, ' '
    call printChar

    mov eax, [es:di + 8]
    call printHex
    mov al, ' '
    call printChar

    mov eax, [es:di + 16]
    call printHex
    
    mov al, 13
    call printChar
    mov al, 10
    call printChar

    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    
    ret

printChar:
    [bits 16]

    mov ah, 0x0E
    int 0x10

    ret

printString:
    [bits 16]

    mov ah, 0x0E

    .print_next:
        lodsb
        or al, al
        jz .done_print_string
        int 0x10
        jmp .print_next

    .done_print_string:
        ret

printHex:
    [bits 16]
    
    push ax
    push bx
    push cx
    push dx
    mov cx, 8

    .next_digit:
        rol eax, 4
        mov bl, al
        and bl, 0x0F
        cmp bl, 10
        jl .print_digit
        add bl, 'A' - 10
        jmp .write_digit

    .print_digit:
        add bl, '0'

    .write_digit:
        mov ah, 0x0E
        mov al, bl
        int 0x10
        loop .next_digit

    pop dx
    pop cx
    pop bx
    pop ax
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