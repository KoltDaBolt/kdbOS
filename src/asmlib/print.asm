print_char:
    mov ah, 0x0E
    int 0x10
    ret

print_string:
    .loop:
        lodsb
        cmp al, 0
        je .done_print_string
        call print_char
        jmp .loop
    
    .done_print_string:
        ret

print_hex:   ; Assumes the number you want to print is in eax
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
	