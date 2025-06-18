BITS 32

extern kernel_main

global start
start:
    push edi ; fs_size
    push esi ; fs_addr
    call kernel_main
    add esp, 8
.halt:
    hlt
    jmp .halt
