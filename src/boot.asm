[bits 16]                           ; 16-bit real mode
[org 0x7C00]                        ; BIOS loads bootloader at address 0x7C00

start:
    xor ax, ax                      ; Put 0 in the ax register. This is to set up segment registers
    mov ds, ax                      ; Set the data segment (ds) register to 0
    mov es, ax                      ; Set the extra segment (es) register to 0

    mov si, msg                     ; Moves msg into the si register
    call print_string

main_loop:
    call get_key
    call print_char
    jmp main_loop

print_string:
    .next_char:
        lodsb                       ; Loads next byte (character) from the memory location pointed to by the si register into the al register
        cmp al, 0                   ; Compares the value in the al register with 0, checking if we've reached the null terminator
        je .done                    ; If the comparison above is true, jump to .done to end the subroutine
        call print_char             ; 
        jmp .next_char
    
    .done:
        ret

print_char:
    mov ah, 0x0E                    ; Set ah register to 0x0E. This tells BIOS interrupt 0x10 to use the teletype output function to print a character to the screen
    int 0x10                        ; Calls BIOS interrupt 0x16
    ret

get_key:
    xor ah, ah                      ; Clears ah register. Setting this to 0 tells BIOS interrupt 0x16 to wait for a key press
    int 0x16                        ; Calls BIOS interrupt 0x16
    ret

section .data
    msg db 'Hello World!', 0

times 510 - ($ - $$) db 0           ; Pads the rest of the bootloader with zeroes to 512 bytes
dw 0xAA55                           ; Boot signature