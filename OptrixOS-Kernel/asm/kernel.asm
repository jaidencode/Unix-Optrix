BITS 32

extern kernel_main

extern initrd_addr
extern initrd_size

global start
start:
    mov [initrd_addr], ebx
    mov [initrd_size], ecx
    call kernel_main
.halt:
    hlt
    jmp .halt

section .bss
    align 4
initrd_addr: resd 1
initrd_size: resd 1
