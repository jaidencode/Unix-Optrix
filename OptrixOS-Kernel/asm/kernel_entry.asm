[bits 32]
section .text
global kernel_entry
extern kernel_main
kernel_entry:
    mov esp, 0x90000
    call kernel_main
.halt:
    hlt
    jmp .halt
