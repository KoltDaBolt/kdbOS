bits 32

global context_switch
global bootstrap_first_process

section .text

context_switch:
    pusha
    pushfd

    mov eax, [esp + 40]
    mov [eax], esp

    mov edx, [esp + 44]
    mov ecx, [esp + 48]

    mov esp, edx
    mov cr3, ecx

    popf
    popa

    ret

bootstrap_first_process:
    mov edx, [esp + 4]
    mov ecx, [esp + 8]

    mov esp, edx
    mov cr3, ecx

    popf
    popa

    ret
