; Bootloader for OptrixOS (stage1)
[org 0x7c00]
[bits 16]

global boot
boot:
    ; copy ourselves to 0x0600 so we can reuse 0x7c00 area
    cld
    mov si, 0x7c00
    mov di, 0x0600
    mov cx, 256
    rep movsw
    jmp 0x0000:boot_stage2

boot_stage2:
    cli
    mov [BOOT_DRIVE], dl
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    ; clear a small portion of memory (~64KB) like fsboot.s
    mov ax, 0
    mov es, ax
    mov di, 0
    mov cx, 0x2000
    rep stosw

    ; prompt for kernel path (input ignored but mimics fsboot.s)
    mov si, prompt_msg
    call print_string
    mov di, PATH_BUF
read_char:
    call get_key
    cmp al, 0x0d
    je done_read
    mov [di], al
    inc di
    call print_char
    jmp read_char
done_read:
    call newline

    ; inform the user we are loading the kernel
    mov si, load_msg
    call print_string
    call newline

    ; load kernel from disk (KERNEL_SECTORS sectors) to 0x1000
    mov bx, 0x1000
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0
    mov cl, 2
    int 0x13
    jc disk_error

    mov si, loaded_msg
    call print_string
    call newline

    ; enter protected mode
    cli
    lgdt [GDT_DESC]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9fc00
    jmp CODE_SEG:0x1000

[bits 16]
disk_error:
    mov si, disk_msg
    call print_string
    hlt
    jmp disk_error

; BIOS helper routines
print_char:
    mov ah, 0x0e
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    ret

print_string:
    lodsb
    or al, al
    jz .done_ps
    push si
    call print_char
    pop si
    jmp print_string
.done_ps:
    ret

get_key:
    mov ah, 0x00
    int 0x16
    mov ah, 0
    ret

newline:
    mov al, 13
    call print_char
    mov al, 10
    call print_char
    ret

prompt_msg db 'Kernel path: ',0
load_msg   db 'Loading kernel...',0
loaded_msg db 'Kernel Loaded',0
disk_msg   db 'Disk read error',0
PATH_BUF times 64 db 0

BOOT_DRIVE db 0
KERNEL_SECTORS equ 4

align 4
GDT:
    dq 0
CODE_SEG equ 0x08
DATA_SEG equ 0x10
    dq 0x00cf9a000000ffff
    dq 0x00cf92000000ffff
GDT_END:

GDT_DESC:
    dw GDT_END - GDT - 1
    dd GDT

times 510-($-$$) db 0
dw 0xaa55
