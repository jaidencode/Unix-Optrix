BITS 32

extern kernel_main

global start
start:
    call kernel_main
.halt:
    hlt
    jmp .halt
