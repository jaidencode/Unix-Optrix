#include "screen.h"
#include "keyboard.h"

/* Simple login prompt shown before terminal starts */
void login_prompt(void) {
    keyboard_flush();
    const char *user_msg = "Username: admin";
    const char *pass_msg = "Password: ";
    char input[32];

    while(1) {
        screen_clear();
        screen_init();
        for(int i=0; user_msg[i]; i++)
            screen_put_char((SCREEN_COLS - 14)/2 + i, SCREEN_ROWS/2 - 1, user_msg[i], 0x0F);
        for(int i=0; pass_msg[i]; i++)
            screen_put_char((SCREEN_COLS - 10)/2 + i, SCREEN_ROWS/2 + 1, pass_msg[i], 0x0F);

        int idx = 0;
        while(idx < 31) {
            char c = keyboard_getchar();
            if(!c) continue;
            if(c == '\n') break;
            if(c == '\b') {
                if(idx > 0) {
                    idx--; 
                    screen_put_char((SCREEN_COLS - 10)/2 + 10 + idx, SCREEN_ROWS/2 + 1, ' ', 0x0F);
                }
                continue;
            }
            screen_put_char((SCREEN_COLS - 10)/2 + 10 + idx, SCREEN_ROWS/2 + 1, '*', 0x0F);
            input[idx++] = c;
        }
        input[idx] = '\0';
        if(idx==0) continue;
        if(input[0]=='a' && input[1]=='d' && input[2]=='m' && input[3]=='i' && input[4]=='n' && input[5]=='1' && input[6]=='\0')
            break;
    }
}
