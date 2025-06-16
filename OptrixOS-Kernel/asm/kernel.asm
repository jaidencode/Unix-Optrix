BITS 32

extern screen_init

global start
start:
    call screen_init
.halt:
    hlt
    jmp .halt
