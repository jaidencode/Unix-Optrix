BITS 32

extern kernel_main
extern initrd_start
extern initrd_size

global start
start:
    mov [initrd_start], eax
    mov [initrd_size], ebx
    call kernel_main
.halt:
    hlt
    jmp .halt
