; Bootloader for OptrixOS
[org 0x7c00]
[bits 16]

global boot
boot:
    cli
    mov [BOOT_DRIVE], dl
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    ; load kernel (one sector) to 0x1000
    mov bx, 0x1000
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    int 0x13
    jc disk_error

    ; setup GDT for protected mode
    cli
    lgdt [GDT_DESC]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:init_pm

; 32-bit code
[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9fc00
    jmp CODE_SEG:0x1000

[bits 16]
disk_error:
    hlt
    jmp disk_error

BOOT_DRIVE db 0

; GDT entries
align 4
GDT:
    dq 0
CODE_SEG equ 0x08
DATA_SEG equ 0x10
    dq 0x00cf9a000000ffff ; code segment
    dq 0x00cf92000000ffff ; data segment
GDT_END:

; GDT descriptor
GDT_DESC:
    dw GDT_END - GDT - 1
    dd GDT

times 510 - ($-$$) db 0
DW 0xAA55
