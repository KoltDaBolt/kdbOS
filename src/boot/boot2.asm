org 0x7E00

entry:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    cli
    call getMemoryMap
    call printMemoryEntry
    call enableA20
    call loadGDT
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp $
    ;jmp dword 0x08:.pmode

;.pmode:
    ;jmp .halt

    ;mov ax, 0x10
    ;mov ds, ax
    ;mov ss, ax

    ;mov esi, HELLO
    ;mov edi, ScreenBuffer
    ;cld

;.loop:
    ;lodsb
    ;or al, al
    ;jz .done

    ;mov [edi], al
    ;inc edi

    ;mov [edi], byte 0x0F
    ;inc edi
    ;jmp .loop

getMemoryMap:
    memmap_entry equ 0x0500
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
    mov di, 0x0504
    mov cx, [memmap_entry]

    .print_mem_loop:
        cmp cx, 0
        je .done_print_mem

        mov eax, [es:di + 4]
        call print_hex
        mov al, ' '
        call print_char

        mov eax, [es:di]
        call print_hex
        mov al, ' '
        call print_char

        mov eax, [es:di + 12]
        call print_hex
        mov al, ' '
        call print_char

        mov eax, [es:di + 8]
        call print_hex
        mov al, ' '
        call print_char

        mov eax, [es:di + 16]
        call print_hex
        mov al, ' '
        call print_char
        
        mov al, 0x0D
        call print_char
        mov al, 0x0A
        call print_char

        add di, 24
        dec cx
        jmp .print_mem_loop
    
    .done_print_mem:
        ret

enableA20:
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
    in al, KbdControllerCommandPort
    test al, 2
    jnz A20WaitInput
    ret

A20WaitOutput:
    in al, KbdControllerCommandPort
    test al, 1
    jz A20WaitOutput
    ret

loadGDT:
    lgdt [GDTDescriptor]
    ret

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

%include "./src/asmlib/print.asm"

KbdControllerDataPort               equ 0x60
KbdControllerCommandPort            equ 0x64
KbdControllerDisableKeyboard        equ 0xAD
KbdControllerEnableKeyboard         equ 0xAE
KbdControllerReadCtrlOutputPort     equ 0xD0
KbdControllerWriteCtrlOutputPort    equ 0xD1

ScreenBuffer                        equ 0xB8000

HELLO: db "Hello from protected mode!", 0

times 2048 - ($ - $$) db 0