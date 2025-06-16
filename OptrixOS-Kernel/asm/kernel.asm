BITS 32

extern graphics_set_framebuffer
extern kernel_main

global start
start:
    push ebx
    call graphics_set_framebuffer
    add esp, 4
    call kernel_main
.halt:
    hlt
    jmp .halt
