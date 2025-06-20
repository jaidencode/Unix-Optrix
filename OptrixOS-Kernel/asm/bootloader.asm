BITS 16
ORG 0x7C00
%define PART_TABLE_ADDR 0x9000
%define MBR_BUFFER     0x7E00

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

    ; read MBR to get partition table
    mov word [dap_count], 1
    mov ax, MBR_BUFFER
    mov [dap_offset], ax
    mov word [dap_segment], 0x0000
    mov dword [dap_lba], 0
    mov dword [dap_lba+4], 0
    mov si, kernel_load_dap
    mov dl, [BOOT_DRIVE]
    mov ah, 0x42
    int 0x13

    ; copy first two partition entries for the kernel
    mov si, MBR_BUFFER + 0x1BE
    mov di, PART_TABLE_ADDR
    mov cx, 32
    rep movsb

    ; partition start LBA for kernel in eax
    mov si, PART_TABLE_ADDR
    mov eax, [si+8]
    mov [PART_TABLE_ADDR+64], eax ; store for loader

    ; load kernel using BIOS LBA read from partition
    mov ax, KERNEL_SECTORS
    mov [dap_count], ax
    mov ax, 0x1000
    mov [dap_offset], ax
    mov word [dap_segment], 0x0000
    mov eax, [PART_TABLE_ADDR+64]
    mov [dap_lba], eax
    mov dword [dap_lba+4], 0
    mov si, kernel_load_dap
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

kernel_load_dap:
    db 0x10, 0
dap_count:    dw 0
dap_offset:   dw 0
dap_segment:  dw 0
dap_lba:      dq 0

bootmsg: db 'Loading OptrixOS...',0

    ; partition table (2 entries used)
    times 446-($-$$) db 0
    ; partition 1: kernel partition
    db 0x80,0,0,0,0x0B,0,0,0
    dd 1               ; start LBA
    dd KERNEL_SECTORS  ; sector count
    ; partition 2: storage
    db 0,0,0,0,0x0B,0,0,0
    dd 1+KERNEL_SECTORS
    dd 204800          ; ~100MB
    times 32 db 0      ; remaining entries
    ; pad and signature
    times 510-($-$$) db 0
    dw 0xAA55
