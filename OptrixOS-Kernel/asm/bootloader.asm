BITS 16
ORG 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Set video mode 13h (320x200x256)
    mov ax, 0x0013
    int 0x10

    ; load kernel (assumes kernel starts at second sector)
    mov bx, 0x1000    ; load address
    mov dh, 1         ; number of sectors
    mov dl, [BOOT_DRIVE]
    mov ax, 0x0201    ; BIOS read disk
    mov cx, 0x0002    ; cylinder/head/sector = sector 2
    int 0x13

    ; jump to kernel
    jmp 0x0000:0x1000

BOOT_DRIVE: db 0

    ; boot signature
    times 510-($-$$) db 0
    dw 0xAA55
