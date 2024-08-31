[bits 16]                           ; 16-bit real mode
[org 0x7C00]                        ; BIOS loads bootloader at address 0x7C00

start:
    xor ax, ax                      ; Put 0 in the ax register. This is to set up segment registers
    mov ds, ax                      ; Set the data segment (ds) register to 0
    mov es, ax                      ; Set the extra segment (es) register to 0

    mov si, mem_map_msg             ; Moves msg into the si register
    call print_string

    call mem_detection

    call print_memory_map

    jmp $

mem_detection:
   clc
   int 0x12                         ; ax = size of low memory in KB starting at address 0
   ret

print_memory_map:
    push ax                         ; save ax, bx, and cx for later restoration, since we will be using these registers
    push bx
    push cx
    xor cx, cx                      ; clear the digit counter

    .convert_loop:                  ; algorithm to convert binary to decimal
        xor dx, dx
        mov bx, 10
        div bx                      ; ax = size of memory, div bx => ax / bx => dx = remainder
        add dl, '0'
        push dx
        inc cx
        cmp ax, 0
        jne .convert_loop
    
    .print_loop:
        pop dx
        mov al, dl
        call print_char
        loop .print_loop
        pop cx
        pop bx
        pop ax
        ret

print_string:
    .next_char:
        lodsb                       ; Loads next byte (character) from the memory location pointed to by the si register into the al register
        cmp al, 0                   ; Compares the value in the al register with 0, checking if we've reached the null terminator
        je .done                    ; If the comparison above is true, jump to .done to end the subroutine
        call print_char             
        jmp .next_char
    
    .done:
        ret

print_char:
    mov ah, 0x0E                    ; Set ah register to 0x0E. This tells BIOS interrupt 0x10 to use the teletype output function to print a character (stored in al) to the screen
    int 0x10                        ; Calls BIOS interrupt 0x10
    ret

section .data
    mem_map_msg db 'Memory Map: ', 0

times 510 - ($ - $$) db 0           ; Pads the rest of the bootloader with zeroes to 512 bytes
dw 0xAA55                           ; Boot signature