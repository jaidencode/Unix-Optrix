BITS 16
ORG 0x7C00

%define PVD_BUF   0x0500
%define DIR_BUF   0x0600
%define DAP_BUF   0x0400

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [BOOT_DRIVE], dl

    ; Set text mode
    mov ax, 0x0003
    int 0x10

    ; print boot message
    mov si, bootmsg
.print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x1E
    int 0x10
    jmp .print
.done:

    ; read Primary Volume Descriptor (LBA 16)
    mov ax, 0
    mov es, ax
    mov bx, PVD_BUF
    mov eax, 16
    mov cx, 1
    call read_lba

    ; root directory extent
    mov si, PVD_BUF + 0x9C + 2
    mov eax, [si]
    mov [ROOT_LBA], eax
    mov si, PVD_BUF + 0x9C + 10
    mov eax, [si]
    mov [ROOT_SIZE], eax

    ; load first sector of root directory
    mov ax, 0
    mov es, ax
    mov bx, DIR_BUF
    mov eax, [ROOT_LBA]
    mov cx, 1
    call read_lba

    ; find kernel file in directory
    mov si, DIR_BUF
.search:
    mov bl, [si]
    cmp bl, 0
    je kernel_not_found
    mov di, si
    mov al, [di+32]
    mov bp, di
    add bp, 33
    push si
    mov si, kernel_name
    mov cx, kernel_name_len
    mov di, bp
    repe cmpsb
    pop si
    je found_kernel
    movzx bx, bl
    add si, bx
    jmp .search

kernel_not_found:
    jmp hang

found_kernel:
    mov eax, [si+2]
    mov [KERNEL_LBA], eax
    mov eax, [si+10]
    mov [KERNEL_SIZE], eax

    ; load kernel using LBA read
    mov ax, 0
    mov es, ax
    mov bx, 0x1000
    mov eax, [KERNEL_LBA]
    mov ecx, [KERNEL_SIZE]
    add ecx, 2047
    shr ecx, 11
    call read_lba

    ; setup GDT and enter protected mode
    lgdt [gdt_desc]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

; BIOS read using INT 13h extensions
read_lba:
    pusha
    push ds
    mov ax, 0
    mov ds, ax
    mov byte [DAP_BUF], 0x10
    mov byte [DAP_BUF+1], 0
    mov word [DAP_BUF+2], cx
    mov word [DAP_BUF+4], bx
    mov word [DAP_BUF+6], es
    mov dword [DAP_BUF+8], eax
    mov dword [DAP_BUF+12], 0
    mov si, DAP_BUF
    mov dl, [BOOT_DRIVE]
    mov ah, 0x42
    int 0x13
    pop ds
    jc hang
    popa
    ret

hang:
    cli
    hlt
    jmp hang

; 32-bit protected mode stub
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
.hlt:
    hlt
    jmp .hlt

[BITS 16]
; GDT setup
gdt_start:
    dq 0x0000000000000000
    dq 0x00cf9a000000ffff
    dq 0x00cf92000000ffff
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

BOOT_DRIVE: db 0
ROOT_LBA:   dd 0
ROOT_SIZE:  dd 0
KERNEL_LBA: dd 0
KERNEL_SIZE: dd 0

kernel_name: db 'KERNEL.BIN;1'
kernel_name_len equ $ - kernel_name

bootmsg: db 'Loading OptrixOS...',0

times 510-($-$$) db 0
dw 0xAA55
