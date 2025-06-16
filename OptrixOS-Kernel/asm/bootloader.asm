BITS 16
ORG 0x7C00

%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 1
%endif

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Get VESA mode information for 0x103
    mov ax, 0x4F01
    mov cx, 0x103
    mov di, mode_info
    int 0x10
    ; Save linear framebuffer address
    mov si, mode_info
    mov eax, [si + 0x28]
    mov [fb_addr], eax

    ; Set VESA graphics mode 0x4103 (800x600 256 colors, linear FB)
    mov ax, 0x4F02
    mov bx, 0x4103
    int 0x10

    ; load kernel (assumes kernel starts at second sector)
    mov bx, 0x1000    ; ES:BX points to load address
    mov dl, [BOOT_DRIVE]
    mov dh, 0         ; head
    mov ah, 0x02      ; BIOS read disk
    mov al, KERNEL_SECTORS
    mov cx, 0x0002    ; CH=0, CL=2 (sector 2)
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

    mov ebx, [fb_addr]

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

mode_info: times 256 db 0
fb_addr:   dd 0

BOOT_DRIVE: db 0

    ; boot signature
    times 510-($-$$) db 0
    dw 0xAA55
