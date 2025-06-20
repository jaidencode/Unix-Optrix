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

    ; load kernel (assumes kernel starts at second sector)
    mov bx, 0x1000    ; ES:BX points to load address
    mov dl, [BOOT_DRIVE]
    mov dh, 0         ; head
    mov ah, 0x02      ; BIOS read disk
    mov al, KERNEL_SECTORS
    mov cx, 0x0002    ; CH=0, CL=2 (sector 2)
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

bootmsg: db 'Loading OptrixOS...',0

    ; pad so partition table starts at offset 0x1BE
    times 446-($-$$) db 0

part_table:
part1_entry:
    db 0x80        ; bootable flag
    db 0,2,0       ; CHS begin (head 0, sector 2)
    db 0x83        ; type Linux
    db 0,0,0       ; CHS end (ignored)
    dd 1           ; start LBA - patched by build script
    dd KERNEL_SECTORS ; total sectors - patched
part2_entry: times 16 db 0
part3_entry: times 16 db 0
part4_entry: times 16 db 0

    ; boot signature
    dw 0xAA55
