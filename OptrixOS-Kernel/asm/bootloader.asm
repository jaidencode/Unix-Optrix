BITS 16
ORG 0x7C00

%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 1
%endif
%ifndef PART1_START
%define PART1_START 2048
%endif
%ifndef PART1_SIZE
%define PART1_SIZE 10240
%endif
%ifndef PART2_START
%define PART2_START (PART1_START + PART1_SIZE)
%endif
%ifndef PART2_SIZE
%define PART2_SIZE 204800
%endif

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

    ; locate first partition LBA and load kernel using LBA read
    mov bx, 0x7C00 + 0x1BE + 8
    mov eax, [bx]         ; starting LBA of partition 1
    mov [dap_lba], eax
    mov dword [dap_lba+4], 0

    mov word [dap_sects], KERNEL_SECTORS
    mov dl, [BOOT_DRIVE]
    mov si, dap
    mov ah, 0x42          ; extended read
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

; Disk Address Packet for BIOS extended read
dap:
    dw 0x10
    dw 0
dap_sects: dw 0
    dw 0x1000
    dw 0
dap_lba: dq 0

bootmsg: db 'Loading OptrixOS...',0

    ; pad up to partition table start
    times 0x1BE-($-$$) db 0

    ; partition table
    db 0x80,0,0,0,0x83,0,0,0
    dd PART1_START
    dd PART1_SIZE
    db 0x00,0,0,0,0x83,0,0,0
    dd PART2_START
    dd PART2_SIZE
    times 16*2 db 0

    dw 0xAA55
