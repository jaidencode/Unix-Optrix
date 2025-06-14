[org 0x7c00]
[bits 16]
KERNEL_LOAD_ADDR equ 0x1000
KERNEL_SECTORS equ 32
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
    mov bx, KERNEL_LOAD_ADDR
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0
    mov cl, 2
    int 0x13
    jc disk_error
    mov ax, 0x13
    int 0x10
    lgdt [gdt_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:protected_mode
disk_error:
    hlt
    jmp disk_error
[bits 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    call gdt_install
    call idt_install
    jmp 0x08:kernel_entry
BOOT_DRIVE: db 0
msg_boot db 'OptrixOS Minimum Kernel boot',0
align 4
gdt_start:
    dq 0
    dq 0x00cf9a000000ffff
    dq 0x00cf92000000ffff
gdt_end:
gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start
times 510-($-$$) db 0
dw 0xaa55
