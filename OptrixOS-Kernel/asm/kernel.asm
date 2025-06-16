BITS 32

extern graphics_set_framebuffer
extern screen_init
extern boot_logo
extern login_prompt
extern terminal_init
extern terminal_run

global start
start:
    push ebx
    call graphics_set_framebuffer
    add esp, 4
    call screen_init
    call boot_logo
    call login_prompt
    call terminal_init
    call terminal_run
.halt:
    hlt
    jmp .halt
