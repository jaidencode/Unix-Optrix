[org 0x7c00]
[bits 16]

KERNEL_LOAD_ADDR equ 0x100000
KERNEL_SECTORS   equ 32
MODE             equ 0x175
MODE_INFO        equ 0x0600

boot:
    mov [BOOT_DRIVE], dl
    xor ax, ax
    mov ds, ax
    mov es, ax
    cli
    mov ss, ax
    mov sp, 0x7c00

    ; get VBE mode info
    mov ax, 0x4F01
    mov cx, MODE
    mov di, MODE_INFO
    int 0x10

    ; set VBE mode with linear framebuffer
    mov ax, 0x4F02
    mov bx, MODE | 0x4000
    int 0x10

    ; store framebuffer address from mode info (offset 40)
    mov ax, [MODE_INFO + 40]
    mov word [FRAMEBUFFER_PTR], ax
    mov ax, [MODE_INFO + 42]
    mov word [FRAMEBUFFER_PTR+2], ax

    ; load kernel
    mov bx, KERNEL_LOAD_ADDR
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    mov cx, 2
.next:
    mov ah, 0x02
    mov al, 1
    int 0x13
    jc .disk_error
    add bx, 512
    inc cx
    cmp cx, 2 + KERNEL_SECTORS
    jl .next

    lgdt [gdt_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:protected_mode

.disk_error:
    hlt
    jmp .disk_error

BOOT_DRIVE db 0
FRAMEBUFFER_PTR dd 0

align 4

gdt_start:
    dq 0
    dq 0x00cf9a000000ffff
    dq 0x00cf92000000ffff

gdt_end:
gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    mov eax, [FRAMEBUFFER_PTR]
    push eax
    call KERNEL_LOAD_ADDR

    hlt
    jmp $

; pad and signature

times 510-($-$$) db 0
dw 0xAA55
