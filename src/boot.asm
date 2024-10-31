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
    call loadGDT
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp dword 0x08:.pmode

.pmode:
    [bits 32]

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

    mov [edi], byte 0xF1
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