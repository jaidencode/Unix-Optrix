BITS 32

extern screen_init
extern terminal_init
extern terminal_run

global start
start:
    call screen_init
    call terminal_init
    call terminal_run
.halt:
    hlt
    jmp .halt
