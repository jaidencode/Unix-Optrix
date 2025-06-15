[bits 32]
section .text
global kernel_entry
extern kernel_main
extern framebuffer
kernel_entry:
    ; Retrieve VBE framebuffer physical address stored by bootloader
    mov eax, [0x9000 + 40]
    mov [framebuffer], eax
    mov esp, 0x90000
    call kernel_main
.halt:
    hlt
    jmp .halt
