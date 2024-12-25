org 0x7C00

mov ax, 0x0000
mov es, ax
mov ax, 0x7E00
mov bx, ax

mov ah, 0x02
mov al, 0x05
mov dh, 0x00
mov dl, 0x80
mov ch, 0x00
mov cl, 0x02
int 0x13
jc disk_read_error
jmp 0x0000:0x7E00

disk_read_error:
    mov si, disk_read_err_string
    call print_string
    jmp $

%include "./src/asmlib/print.asm"

disk_read_err_string: db 'Disk Read Error', 0x0D, 0x0A, 0

times 510 - ($ - $$) db 0
dw 0xAA55