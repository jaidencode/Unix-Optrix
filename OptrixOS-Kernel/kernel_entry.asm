[bits 32]

extern kernel_start

section .text

global start

start:
    ; set up stack
    mov esp, stack_top
    push 0
    call kernel_start
.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 4096
stack_top:
