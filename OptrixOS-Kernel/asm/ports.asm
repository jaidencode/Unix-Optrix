[BITS 32]

GLOBAL inb
GLOBAL outb

inb:
    mov edx, [esp+4]
    in al, dx
    movzx eax, al
    ret

outb:
    mov edx, [esp+4]
    mov al, [esp+8]
    out dx, al
    ret
