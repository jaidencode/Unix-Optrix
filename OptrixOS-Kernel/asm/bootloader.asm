BITS 16
ORG 0x7C00

%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 1
%endif

%ifndef INITRD_SECTORS
%define INITRD_SECTORS 0
%endif

%ifndef INITRD_ADDR
%define INITRD_ADDR 0x90000
%endif
%assign INITRD_SEG INITRD_ADDR >> 4

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Set 80x25 text mode
    mov ax, 0x0003
    int 0x10

    ; display boot message
    mov si, bootmsg
.printloop:
    lodsb
    or al, al
    jz .doneprint
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x1E
    int 0x10
    jmp .printloop
.doneprint:

    ; load kernel using BIOS extended read
    mov word [dap_kernel_count], KERNEL_SECTORS
    mov dword [dap_kernel_lba], 1
    mov word [dap_kernel_off], 0x1000
    mov word [dap_kernel_seg], 0x0000
    mov si, dap_kernel
    mov dl, [BOOT_DRIVE]
    mov ah, 0x42
    int 0x13

    ; load initrd right after kernel
    mov word [dap_initrd_count], INITRD_SECTORS
    mov dword [dap_initrd_lba], 1 + KERNEL_SECTORS
    mov word [dap_initrd_off], 0x0000
    mov word [dap_initrd_seg], INITRD_SEG
    mov si, dap_initrd
    mov dl, [BOOT_DRIVE]
    mov ah, 0x42
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

    mov ebx, INITRD_ADDR
    mov ecx, INITRD_SECTORS
    shl ecx, 9

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

bootmsg: db 'Loading OptrixOS...',0

; Disk address packets
align 4
dap_kernel:
    db 0x10,0          ; size, reserved
dap_kernel_count: dw 0
dap_kernel_off:   dw 0
dap_kernel_seg:   dw 0
dap_kernel_lba:   dq 0

dap_initrd:
    db 0x10,0
dap_initrd_count: dw 0
dap_initrd_off:   dw 0
dap_initrd_seg:   dw 0
dap_initrd_lba:   dq 0

    ; boot signature
    times 510-($-$$) db 0
    dw 0xAA55
