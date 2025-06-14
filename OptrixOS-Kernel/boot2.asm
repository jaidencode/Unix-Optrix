[bits 32]
section .text

global stage2_entry

; Simple second stage that clears the VGA framebuffer to green. This
; demonstrates the stage2 entry point without relying on further disk
; loading logic.
stage2_entry:
    mov edi, 0xA0000            ; VGA mode 13h framebuffer
    mov ecx, 320*200            ; number of pixels
    mov al, 0x02                ; colour (green)
    rep stosb                   ; fill screen
    ret
