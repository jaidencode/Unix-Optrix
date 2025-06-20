BITS 16
ORG 0x7C00

%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 1
%endif
%ifndef KERNEL_LBA
%define KERNEL_LBA 1
%endif

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [BOOT_DRIVE], dl

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

    ; prepare Disk Address Packet for LBA load
    mov word [dap], 0x0010
    mov word [dap+2], KERNEL_SECTORS
    mov word [dap+4], 0x1000
    mov word [dap+6], 0x0000
    mov dword [dap+8], KERNEL_LBA
    mov dword [dap+12], 0x00000000

    mov dl, [BOOT_DRIVE]
    mov si, dap
    mov ah, 0x42
    int 0x13
    jc disk_error

    ; setup basic GDT for protected mode
    lgdt [gdt_desc]

    ; enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

disk_error:
    mov si, diskerr
.errloop:
    lodsb
    or al, al
    jz halt_loop
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x4F
    int 0x10
    jmp .errloop

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
halt_loop:
    hlt
    jmp halt_loop

; GDT setup
[BITS 16]
gdt_start:
    dq 0x0000000000000000
    dq 0x00cf9a000000ffff
    dq 0x00cf92000000ffff
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

dap: times 16 db 0
BOOT_DRIVE: db 0

bootmsg: db 'Loading OptrixOS...',0
diskerr: db 'Disk read error',0

; Reserve space for partition table
    times 446-($-$$) db 0
part_table:
    times 64 db 0

    dw 0xAA55
