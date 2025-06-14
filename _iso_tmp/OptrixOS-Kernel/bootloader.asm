[org 0x7c00]
[bits 16]

global boot

BOOT_DRIVE: db 0
; Number of 512 byte sectors to load for the kernel. The previous value of
; 16 was insufficient once the kernel grew in size which caused the boot
; process to hang before the graphical mode was initialised. 32 sectors is
; plenty of room for the current build and keeps things simple.
KERNEL_SECTORS equ 32
KERNEL_LOAD_ADDR equ 0x1000

msg_boot db 'OptrixOS Kernel boot',0

boot:
    mov [BOOT_DRIVE], dl       ; save boot drive

    ; print boot message
    mov si, msg_boot
.print_loop:
    lodsb
    test al, al
    jz .after_print
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .print_loop
.after_print:

    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; load kernel image
    mov bx, KERNEL_LOAD_ADDR
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0
    mov cl, 2
    int 0x13
    jc disk_error

    ; set graphics mode 13h
    mov ax, 0x0013
    int 0x10

    ; set up GDT
    lgdt [gdt_desc]

    ; enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    jmp CODE_SEG:KERNEL_LOAD_ADDR

[bits 16]
disk_error:
    hlt
    jmp disk_error

; GDT
CODE_SEG equ 0x08
DATA_SEG equ 0x10
align 4
gdt_start:
    dq 0
    dq 0x00cf9a000000ffff ; code
    dq 0x00cf92000000ffff ; data
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Padding
 times 510-($-$$) db 0
 dw 0xaa55
