[bits 32]
section .text
global kernel_entry
extern kernel_main
kernel_entry:
    ; Simple 32-bit entry
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    call kernel_main
.halt:
    hlt
    jmp .halt
