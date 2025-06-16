#include "terminal.h"
#include "keyboard.h"
#include <stdint.h>
#include <stddef.h>

static int streq(const char* a, const char* b) {
    while(*a && *b) {
        if(*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int strprefix(const char* str, const char* pre) {
    while(*pre) {
        if(*str != *pre) return 0;
        str++; pre++;
    }
    return 1;
}

#define WIDTH 80
#define HEIGHT 25
#define BORDER_COLOR 0x0F
#define TEXT_COLOR 0x0E

static volatile uint16_t* const VIDEO = (uint16_t*)0xB8000;
static int row = 1;
static int col = 1;

static void put_entry_at(char c, uint8_t color, int x, int y) {
    VIDEO[y * WIDTH + x] = ((uint16_t)color << 8) | c;
}

static void scroll(void) {
    for(int y=2; y<HEIGHT-1; y++) {
        for(int x=1; x<WIDTH-1; x++) {
            VIDEO[(y-1)*WIDTH + x] = VIDEO[y*WIDTH + x];
        }
    }
    for(int x=1; x<WIDTH-1; x++)
        VIDEO[(HEIGHT-2)*WIDTH + x] = ((uint16_t)0x01 << 8) | ' ';
}

static void putchar(char c) {
    if(c=='\n') {
        row++;
        col=1;
    } else {
        put_entry_at(c, TEXT_COLOR, col, row);
        col++;
        if(col >= WIDTH-1) {
            col=1;
            row++;
        }
    }
    if(row >= HEIGHT-1) {
        scroll();
        row = HEIGHT-2;
    }
}

void terminal_init(void) {
    row = 1;
    col = 1;
}

static void print(const char* str) {
    for(size_t i=0; str[i]; i++)
        putchar(str[i]);
}

static void read_line(char* buf, size_t max) {
    size_t idx=0;
    while(idx < max-1) {
        char c = keyboard_getchar();
        if(!c) continue;
        if(c=='\n') {
            putchar('\n');
            break;
        }
        if(c=='\b') {
            if(idx>0) { idx--; col--; putchar(' '); col--; }
            continue;
        }
        putchar(c);
        buf[idx++] = c;
    }
    buf[idx]='\0';
}

static void cmd_help(void) {
    print("Available commands:\n");
    print("help - show this help\n");
    print("clear - clear the screen\n");
    print("echo  - echo text\n");
}

static void cmd_clear(void) {
    for(int y=1; y<HEIGHT-1; y++)
        for(int x=1; x<WIDTH-1; x++)
            VIDEO[y*WIDTH + x] = ((uint16_t)0x01 << 8) | ' ';
    row=1; col=1;
}

static void cmd_echo(const char* args) {
    print(args);
    putchar('\n');
}

static void execute(const char* line) {
    if(streq(line, "help")) {
        cmd_help();
    } else if(streq(line, "clear")) {
        cmd_clear();
    } else if(strprefix(line, "echo ")) {
        cmd_echo(line+5);
    } else if(line[0]) {
        print("Unknown command\n");
    }
}

void terminal_run(void) {
    char buf[128];
    while(1) {
        print("> ");
        read_line(buf, sizeof(buf));
        execute(buf);
    }
}
