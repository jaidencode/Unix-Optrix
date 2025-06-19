[BITS 32]

GLOBAL inb
GLOBAL outb
GLOBAL inw
GLOBAL outw

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

inw:
    mov edx, [esp+4]
    in ax, dx
    movzx eax, ax
    ret

outw:
    mov edx, [esp+4]
    mov ax, [esp+8]
    out dx, ax
    ret
