bits 32

extern master_exception_handler
global idt_stub_table

%macro ISR_NOERRCODE 1
exception_stub_%1:
    push 0
    push %1
    jmp common_stub_handler
%endmacro

%macro ISR_ERRCODE 1
exception_stub_%1:
    push %1
    jmp common_stub_handler
%endmacro

%assign i 0
%rep 256
    %if i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 21 || i == 29 || i == 30
        ISR_ERRCODE i
    %else
        ISR_NOERRCODE i
    %endif
%assign i i+1
%endrep

common_stub_handler:
    pusha

    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call master_exception_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iret

section .data
idt_stub_table:
%assign i 0
%rep 256
    dd exception_stub_%+i
%assign i i+1
%endrep