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

    ; load kernel starting at sector 2 using a loop so large images work
    mov si, 0          ; segment portion of load address
    mov di, 0x1000     ; offset portion of load address
    mov dx, [BOOT_DRIVE]
    xor ch, ch         ; cylinder 0
    xor dh, dh         ; head 0
    mov cl, 2          ; sector 2
    mov cx, [kernel_sectors]
load_loop:
    mov es, si
    mov bx, di
    mov ah, 0x02
    mov al, 1
    int 0x13
    add di, 512
    cmp di, 0x10000
    jb .addr_ok
    sub di, 0x10000
    add si, 0x1000
.addr_ok:
    inc cl
    cmp cl, 19
    jl .sect_ok
    mov cl, 1
    inc dh
    cmp dh, 2
    jl .sect_ok
    mov dh, 0
    inc ch
.sect_ok:
    dec cx
    jnz load_loop

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

kernel_sectors: dw KERNEL_SECTORS

    ; boot signature
    times 510-($-$$) db 0
    dw 0xAA55
