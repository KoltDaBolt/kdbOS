org 0x7C00

mov ax, 0x0000
mov es, ax
mov ax, 0x7E00
mov bx, ax

; isoinfo -sl -i <PATH_TO_ISO>
; Directory listing of /
; d---------   0    0    0               1 Mar  1 2025 [     24 02]  .
; d---------   0    0    0               1 Mar  1 2025 [     24 02]  ..
; ----------   0    0    0               1 Mar  1 2025 [     26 00]  BOOT.BIN;1
; ----------   0    0    0               1 Mar  1 2025 [     25 00]  BOOT.CAT;1
; ----------   0    0    0               2 Mar  1 2025 [     27 00]  BOOT2.BIN;1
; ----------   0    0    0               3 Mar  1 2025 [     29 00]  KERNEL.BIN;1

mov ah, 0x02     ; BIOS function to read disk sectors
mov al, 0x05     ; QEMU VALUE (Why does reading 5 sectors work???)
;mov al, 0x01     ; REAL HARDWARE VALUE (Why does reading 1 sector only work when the sector size of boot2.bin is 2 sectors???)
mov ch, 0x00     ; Cylinder (0 for simplicity, assuming fits in one track)
mov cl, 0x02     ; Starting sector (See comment below for why this value is 2)
mov dh, 0x00     ; Head (0 for simplicity, assuming single-sided disk)
mov dl, 0x80     ; Drive number (0x80 for the first hard drive)

int 0x13
jc disk_read_error
jmp 0x0000:0x7E00

disk_read_error:
    mov si, disk_read_err_string
    call print_string
    jmp $

include "./src/asmlib/print.asm"

disk_read_err_string: db 'Disk Read Error', 0x0D, 0x0A, 0

times 510 - ($ - $$) db 0
dw 0xAA55
