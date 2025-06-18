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

    ; load kernel starting at sector 2 using INT 13h extensions
    xor ax, ax
    mov es, ax
    mov bx, 0x1000    ; ES:BX points to load address
    mov dl, [BOOT_DRIVE]

    mov dword [remaining], KERNEL_SECTORS
    mov dword [lba], 2

.load_loop:
    mov eax, [remaining]
    test eax, eax
    jz .done_load
    cmp eax, 127
    jbe .less_than_max
    mov cx, 127
    jmp .set_count
.less_than_max:
    mov cx, ax
.set_count:
    mov word [dap+2], cx        ; sectors to read
    mov word [dap+4], bx        ; offset
    mov word [dap+6], es        ; segment
    mov eax, [lba]
    mov dword [dap+8], eax      ; LBA low
    mov dword [dap+12], 0       ; LBA high
    mov ah, 0x42
    mov si, dap
    int 0x13
    jc .disk_error
    ; advance pointers
    mov ax, cx
    shl ax, 9                   ; bytes = sectors * 512
    add bx, ax
    jc .inc_seg
    jmp .no_inc
.inc_seg:
    push ax
    mov ax, es
    inc ax
    mov es, ax
    pop ax
.no_inc:
    movzx eax, cx
    add dword [lba], eax
    sub dword [remaining], eax
    jmp .load_loop

.disk_error:
    mov si, errormsg
.err_print:
    lodsb
    or al, al
    jz .error_halt
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x4F
    int 0x10
    jmp .err_print

.error_halt:
    cli
    hlt
    jmp .error_halt

.done_load:

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

remaining: dd 0
lba: dd 0
dap:
    db 0x10     ; size
    db 0
    dw 0        ; sector count (filled in at runtime)
    dw 0        ; offset
    dw 0        ; segment
    dd 0        ; LBA low
    dd 0        ; LBA high

errormsg: db 'Disk read error',0

bootmsg: db 'Loading OptrixOS...',0

    ; boot signature
    times 510-($-$$) db 0
    dw 0xAA55
