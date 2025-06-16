BITS 32

extern screen_init
extern boot_logo
extern terminal_init
extern terminal_run

global start
start:
    call screen_init
    call boot_logo
    call terminal_init
    call terminal_run
.halt:
    hlt
    jmp .halt
