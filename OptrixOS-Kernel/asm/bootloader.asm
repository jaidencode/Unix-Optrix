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

    ; setup basic GDT for protected mode
    lgdt [gdt_desc]

    ; enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

; 32-bit protected mode code
[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    call dword 0x1000
.halt:
    hlt
    jmp .halt

; GDT setup
[BITS 16]
gdt_start:
    dq 0x0000000000000000
    dq 0x00cf9a000000ffff ; code segment
    dq 0x00cf92000000ffff ; data segment
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

BOOT_DRIVE: db 0

    ; boot signature
    times 510-($-$$) db 0
    dw 0xAA55
