[bits 32]

global start
extern kmain

start:
    call kmain
.hang:
    hlt
    jmp .hang
