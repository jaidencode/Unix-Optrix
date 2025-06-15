[org 0x7c00]
[bits 16]
KERNEL_LOAD_ADDR equ 0x1000      ; 4KB
KERNEL_SECTORS   equ 64          ; adjust for your kernel size

boot:
    mov [BOOT_DRIVE], dl
    mov si, msg_boot
.print:
    lodsb
    test al, al
    jz .print_done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .print
.print_done:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; =========================
    ; Set VESA graphics mode: 1024x768x32bpp linear framebuffer
    ; AX=0x4F02 (VBE Set SuperVGA Video Mode)
    ; BX=0x4118: bit 14=linear framebuffer, mode 0x118 (see OSDev VBE)
    ; =========================
    mov ax, 0x4F02
    mov bx, 0x4118       ; 1024x768x32bpp linear framebuffer
    int 0x10

    ; Query mode information to obtain framebuffer physical address
    mov ax, 0x4F01
    mov cx, 0x118
    mov di, 0x9000       ; store VBE mode info structure at 0x9000
    int 0x10

    ; Load kernel
    mov bx, KERNEL_LOAD_ADDR
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    mov ah, 0x02                ; BIOS read sectors
    mov al, KERNEL_SECTORS
    mov ch, 0
    mov cl, 2                   ; Start at sector 2
    int 0x13
    jc disk_error

    ; Set up GDT in 16-bit mode
    lgdt [gdt_desc]

    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp 0x08:protected_mode

disk_error:
    hlt
    jmp disk_error

BOOT_DRIVE: db 0
msg_boot db 'OptrixOS Bootloader: Loading kernel...',0

align 4
gdt_start:
    dq 0
    dq 0x00cf9a000000ffff       ; Code segment
    dq 0x00cf92000000ffff       ; Data segment
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
    ; Jump to kernel entry point at KERNEL_LOAD_ADDR
    jmp KERNEL_LOAD_ADDR

times 510-($-$$) db 0
dw 0xaa55
