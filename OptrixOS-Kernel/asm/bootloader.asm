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

    ; load kernel (assumes kernel starts at LBA 1)
    mov word [dap_packet+2], KERNEL_SECTORS
    mov word [dap_packet+4], 0x1000     ; offset
    mov word [dap_packet+6], 0x0000     ; segment
    mov dword [dap_packet+8], 1         ; starting LBA
    mov dword [dap_packet+12], 0
    mov si, dap_packet
    mov dl, [BOOT_DRIVE]
    mov ah, 0x42      ; BIOS EXTENDED READ
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

dap_packet:
    db 16,0       ; size, reserved
    dw 0          ; sector count (filled at runtime)
    dw 0          ; offset (filled)
    dw 0          ; segment (filled)
    dd 0          ; start LBA low
    dd 0          ; start LBA high

BOOT_DRIVE: db 0

bootmsg: db 'Loading OptrixOS...',0

    ; pad up to partition table area (offset 0x1BE)
    times 446-($-$$) db 0

    ; simple two-partition MBR table
    ; partition 1: boot (start LBA 1, 256 sectors)
    db 0x80,0x00,0x02,0x00,0x0B,0x00,0x00,0x00
    dd 0x00000001
    dd 0x00000100
    ; partition 2: data (start LBA 257, remaining sectors)
    db 0x00,0x00,0x00,0x00,0x83,0x00,0x00,0x00
    dd 0x00000101
    dd 0x00000F00
    ; remaining empty entries
    times (16*4 - 16*2) db 0

    ; boot signature
    dw 0xAA55
