[org 0x7c00]
[bits 16]
KERNEL_LOAD_ADDR equ 0x1000      ; 4KB
KERNEL_SECTORS   equ 64          ; adjust for your kernel size
SECTORS_PER_TRACK equ 18
HEADS_PER_CYLINDER equ 2

boot:
    mov [BOOT_DRIVE], dl
    mov si, msg_boot
    call print_string
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; Set graphics mode 320x200x256
    mov ax, 0x0013
    int 0x10



    ; Load kernel
    mov si, msg_load
    call print_string
    mov bx, KERNEL_LOAD_ADDR    ; destination address
    mov di, KERNEL_SECTORS      ; sectors remaining
    mov ch, 0                   ; cylinder
    mov dh, 0                   ; head
    mov cl, 2                   ; sector (start after boot)
load_loop:
    mov dl, [BOOT_DRIVE]
    mov ax, 0x0201              ; read one sector
    int 0x13
    jc disk_error
    add bx, 512                 ; advance buffer
    inc cl
    cmp cl, SECTORS_PER_TRACK+1
    jl .no_wrap
    mov cl, 1
    inc dh
    cmp dh, HEADS_PER_CYLINDER
    jl .no_wrap
    mov dh, 0
    inc ch
.no_wrap:
    dec di
    jnz load_loop
    mov si, msg_done
    call print_string

    ; Set up GDT in 16-bit mode
    lgdt [gdt_desc]

    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp 0x08:protected_mode

disk_error:
    hlt
    jmp disk_error

BOOT_DRIVE: db 0
msg_boot db 'OptrixOS Bootloader: Starting...',0
msg_load db 'Loading kernel...',0
msg_done db 'Kernel loaded',0

align 4
gdt_start:
    dq 0
    dq 0x00cf9a000000ffff       ; Code segment
    dq 0x00cf92000000ffff       ; Data segment
gdt_end:
gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

print_string:
    cld
.next_char:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .next_char
.done:
    ret

[bits 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    ; Jump to kernel entry point at KERNEL_LOAD_ADDR
    jmp KERNEL_LOAD_ADDR

times 510-($-$$) db 0
dw 0xaa55
