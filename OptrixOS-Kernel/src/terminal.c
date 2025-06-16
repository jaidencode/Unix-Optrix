#include "terminal.h"
#include "keyboard.h"
#include "screen.h"
#include <stdint.h>
#include <stddef.h>

static void fs_init(void);

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
#define HEIGHT 50
#define BORDER_COLOR 0x0F
#define DEFAULT_TEXT_COLOR 0x0B
#define CURSOR_COLOR 0x0C
#define CURSOR_CHAR '_'

static volatile uint16_t* const VIDEO = (uint16_t*)0xB8000;
static int row = 1;
static int col = 1;
static int cursor_on = 0;
static uint8_t text_color = DEFAULT_TEXT_COLOR;

static void draw_cursor(int visible) {
    char ch = visible ? CURSOR_CHAR : ' ';
    uint8_t color = visible ? CURSOR_COLOR : 0x00;
    VIDEO[row * WIDTH + col] = ((uint16_t)color << 8) | ch;
}

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
        VIDEO[(HEIGHT-2)*WIDTH + x] = ((uint16_t)0x00 << 8) | ' ';
}

static void putchar(char c) {
    draw_cursor(0);
    if(c=='\n') {
        row++;
        col=1;
    } else {
        put_entry_at(c, text_color, col, row);
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
    draw_cursor(1);
}

void terminal_init(void) {
    row = 1;
    col = 1;
    draw_cursor(1);
    fs_init();
}

static void print(const char* str) {
    for(size_t i=0; str[i]; i++)
        putchar(str[i]);
}

static void print_int(int n) {
    char buf[16];
    int i = 0;
    int neg = 0;
    if(n == 0) {
        buf[i++] = '0';
    } else {
        if(n < 0) { neg = 1; n = -n; }
        while(n > 0 && i < (int)sizeof(buf)-1) {
            buf[i++] = '0' + (n % 10);
            n /= 10;
        }
        if(neg && i < (int)sizeof(buf)-1)
            buf[i++] = '-';
        for(int j=i-1, k=0; j>k; j--, k++) {
            char t = buf[j]; buf[j] = buf[k]; buf[k] = t;
        }
    }
    buf[i] = '\0';
    print(buf);
}

static void pause(void) {
    for(volatile int i=0; i<10000; i++);
}

static void read_line(char* buf, size_t max) {
    size_t idx = 0;
    int blink = 0;
    cursor_on = 1;
    draw_cursor(1);
    while(idx < max-1) {
        char c = keyboard_getchar();
        if(!c) {
            blink++;
            if(blink > 5000) {
                blink = 0;
                cursor_on = !cursor_on;
                draw_cursor(cursor_on);
            }
            pause();
            continue;
        }
        if(cursor_on) { draw_cursor(0); cursor_on = 0; }
        if(c == '\n') {
            putchar('\n');
            draw_cursor(1);
            break;
        }
        if(c == '\b') {
            if(idx > 0) {
                draw_cursor(0);         /* remove cursor from current position */
                col--;                  /* move back one column */
                idx--;                  /* update buffer index */
                put_entry_at(' ', 0x00, col, row); /* clear character */
                draw_cursor(1);         /* redraw cursor at new position */
            }
            continue;
        }
        if(c == '\t') {
            int spaces = 4 - ((col - 1) % 4);
            for(int s=0; s<spaces && idx < max-1; s++) {
                putchar(' ');
                buf[idx++] = ' ';
            }
            continue;
        }
        putchar(c);
        buf[idx++] = c;
    }
    draw_cursor(0);
    buf[idx] = '\0';
}

static void cmd_help(void) {
    print("Available commands:\n");
    print("help - show this help\n");
    print("clear - clear the screen\n");
    print("echo  - echo text\n");
    print("about - about this OS\n");
    print("ping  - check connectivity\n");
    print("reverse <text> - reverse a string\n");
    print("add <a> <b>   - add two numbers\n");
    print("color <hex>  - set text color\n");
    print("border      - redraw border\n");
    print("dir         - list directory contents\n");
    print("cd <dir>    - change directory\n");
    print("pwd        - show current path\n");
}

static void cmd_clear(void) {
    for(int y=1; y<HEIGHT-1; y++)
        for(int x=1; x<WIDTH-1; x++)
            VIDEO[y*WIDTH + x] = ((uint16_t)0x00 << 8) | ' ';
    row=1; col=1;
}

static void cmd_echo(const char* args) {
    print(args);
    putchar('\n');
}

static void cmd_about(void) {
    print("OptrixOS experimental kernel\n");
}

static void cmd_ping(void) {
    print("pong!\n");
}

static void cmd_reverse(const char* args) {
    int len = 0;
    while(args[len]) len++;
    for(int i=len-1; i>=0; i--)
        putchar(args[i]);
    putchar('\n');
}

static void cmd_add(const char* args) {
    int a = 0, b = 0;
    int idx = 0;
    int neg = 0;
    while(args[idx] == ' ') idx++;
    if(args[idx] == '-') { neg = 1; idx++; }
    while(args[idx] >= '0' && args[idx] <= '9') {
        a = a*10 + (args[idx]-'0');
        idx++;
    }
    if(neg) a = -a;
    while(args[idx] == ' ') idx++;
    neg = 0;
    if(args[idx] == '-') { neg = 1; idx++; }
    while(args[idx] >= '0' && args[idx] <= '9') {
        b = b*10 + (args[idx]-'0');
        idx++;
    }
    if(neg) b = -b;
    print_int(a + b);
    putchar('\n');
}

static void cmd_color(const char* args) {
    uint8_t val = 0;
    int idx = 0;
    while(args[idx] == ' ') idx++;
    while(args[idx]) {
        char c = args[idx++];
        val <<= 4;
        if(c >= '0' && c <= '9') val |= (c - '0');
        else if(c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
        else if(c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
        else { val = DEFAULT_TEXT_COLOR; break; }
    }
    text_color = val;
}

static void cmd_border(void) {
    screen_init();
    terminal_init();
}

/* simple in-memory filesystem */
typedef struct entry {
    const char* name;
    int is_dir;
    struct entry* parent;
    const struct entry* children;
    int child_count;
} entry;

static entry bin_entries[] = {
    {"echo", 0, NULL, NULL, 0},
    {"ping", 0, NULL, NULL, 0},
};

static entry docs_entries[] = {
    {"guide.txt", 0, NULL, NULL, 0},
    {"info.txt", 0, NULL, NULL, 0},
};

static entry root_entries[] = {
    {"bin", 1, NULL, bin_entries, 2},
    {"docs", 1, NULL, docs_entries, 2},
    {"readme.txt", 0, NULL, NULL, 0},
};

static entry root_dir = {"/", 1, NULL, root_entries, 3};
static entry* current_dir = &root_dir;
static char current_path[32] = "/";

static void fs_init(void) {
    for(int i=0;i<root_dir.child_count;i++)
        root_entries[i].parent = &root_dir;
    for(int i=0;i<2;i++)
        bin_entries[i].parent = &root_entries[0];
    for(int i=0;i<2;i++)
        docs_entries[i].parent = &root_entries[1];
}

static entry* find_subdir(entry* dir, const char* name) {
    for(int i=0;i<dir->child_count;i++)
        if(dir->children[i].is_dir && streq(dir->children[i].name, name))
            return (entry*)&dir->children[i];
    return NULL;
}

static void cmd_dir(void) {
    for(int i=0;i<current_dir->child_count;i++) {
        print(current_dir->children[i].name);
        if(current_dir->children[i].is_dir) print("/");
        print("  ");
    }
    putchar('\n');
}

static void cmd_cd(const char* args) {
    if(args[0]=='\0' || streq(args, "/")) {
        current_dir = &root_dir;
        current_path[0] = '/'; current_path[1] = '\0';
        return;
    }
    if(streq(args, "..")) {
        if(current_dir->parent) {
            current_dir = current_dir->parent;
            if(current_dir == &root_dir) {
                current_path[0] = '/'; current_path[1] = '\0';
            } else {
                /* simple two level path */
                current_path[0] = '/';
                const char* name = current_dir->name;
                int i=0; while(name[i]) { current_path[i+1]=name[i]; i++; }
                current_path[i+1]='\0';
            }
        }
        return;
    }
    entry* d = find_subdir(current_dir, args);
    if(d) {
        current_dir = d;
        if(current_dir == &root_dir) {
            current_path[0] = '/'; current_path[1] = '\0';
        } else {
            current_path[0]='/';
            int i=0; while(args[i]) { current_path[i+1] = args[i]; i++; }
            current_path[i+1]='\0';
        }
    } else {
        print("No such directory\n");
    }
}

static void cmd_pwd(void) {
    print(current_path);
    putchar('\n');
}

static void execute(const char* line) {
    if(streq(line, "help")) {
        cmd_help();
    } else if(streq(line, "clear")) {
        cmd_clear();
    } else if(strprefix(line, "echo ")) {
        cmd_echo(line+5);
    } else if(streq(line, "about")) {
        cmd_about();
    } else if(streq(line, "ping")) {
        cmd_ping();
    } else if(strprefix(line, "reverse ")) {
        cmd_reverse(line+8);
    } else if(strprefix(line, "add ")) {
        cmd_add(line+4);
    } else if(strprefix(line, "color ")) {
        cmd_color(line+6);
    } else if(streq(line, "border")) {
        cmd_border();
    } else if(streq(line, "dir")) {
        cmd_dir();
    } else if(strprefix(line, "cd ")) {
        cmd_cd(line+3);
    } else if(streq(line, "pwd")) {
        cmd_pwd();
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
