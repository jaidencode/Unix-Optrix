#include "graphics.h"
#include "ports.h"

#define VGA_MEMORY ((unsigned char*)0xA0000)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

void graphics_init(void){
    /* switch to VGA mode 0x13 */
    __asm__("int $0x10" : : "a"(0x0013));
}

void graphics_fill_rect(int x, int y, int w, int h, uint8_t color){
    for(int j=0;j<h;j++){
        if(y+j<0||y+j>=SCREEN_HEIGHT) continue;
        for(int i=0;i<w;i++){
            if(x+i<0||x+i>=SCREEN_WIDTH) continue;
            VGA_MEMORY[(y+j)*SCREEN_WIDTH + (x+i)] = color;
        }
    }
}
