BITS 16
ORG 0x7C00



start:
    cli
    mov [BOOT_DRIVE], dl
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

    ; read first filesystem sector to obtain kernel size
    mov bx, 0x0600    ; buffer for root sector
    mov dl, [BOOT_DRIVE]
    mov dh, 0
    mov ah, 0x02
    mov al, 1         ; read 1 sector
    mov cx, 0x0002    ; CH=0, CL=2 (sector 2)
    int 0x13
    mov si, bx
    mov ax, [si+4]    ; root sector count
    mov [root_sectors], ax
    mov ax, [si+8]    ; kernel sector count
    mov [kernel_sectors], ax
    mov ax, [root_sectors]
    add ax, 2         ; boot sector + root sectors -> first kernel sector
    mov [kernel_start], ax

    ; load kernel using values read from the filesystem
    mov bx, 0x1000    ; ES:BX points to load address
    mov dl, [BOOT_DRIVE]
    mov dh, 0         ; head
    mov ah, 0x02      ; BIOS read disk
    mov al, [kernel_sectors]
    mov cx, [kernel_start]
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

root_sectors:   dw 0
kernel_sectors: dw 0
kernel_start:   dw 0

BOOT_DRIVE: db 0

bootmsg: db 'Loading OptrixOS...',0

    ; boot signature
    times 510-($-$$) db 0
    dw 0xAA55
