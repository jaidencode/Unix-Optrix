#include "terminal.h"
#include "keyboard.h"
#include "screen.h"
#include "mouse.h"
#include "fs.h"
#include "mem.h"
#include <stdint.h>
#include <stddef.h>

static int origin_x = OFFSET_X;
static int origin_y = OFFSET_Y + 14; /* space for title bar */

static int streq(const char* a, const char* b) { while(*a && *b) { if(*a!=*b) return 0; a++; b++; } return *a==*b; }
static int strprefix(const char* str, const char* pre) { while(*pre) { if(*str!=*pre) return 0; str++; pre++; } return 1; }

#define MAX_COLS 80
#define MAX_ROWS 60
#define DEFAULT_TEXT_COLOR 0x0F
#define CURSOR_COLOR 0x0E
#define BACKGROUND_COLOR 0x00
#define CURSOR_CHAR '_'

static char text_buffer[MAX_ROWS][MAX_COLS];
static uint8_t color_buffer[MAX_ROWS][MAX_COLS];
static int term_cols = 0;
static int term_rows = 0;
static window_t *term_window = NULL;

static int row = 0;
static int col = 0;
static int cursor_on = 0;
static uint8_t text_color = DEFAULT_TEXT_COLOR;
static fs_entry* current_dir;
static char current_path[32] = "/";

/* Called whenever the terminal window changes position or size. */
static void recursive_update_tick(void) {
    for(int y=0; y<term_rows; y++)
        for(int x=0; x<term_cols; x++)
            screen_put_char_offset(x, y, text_buffer[y][x],
                                  color_buffer[y][x], origin_x, origin_y);
}

void terminal_recursive_update(void) {
    recursive_update_tick();
}

void terminal_set_window(window_t *win) {
    term_window = win;
    int old_x = origin_x;
    int old_y = origin_y;
    origin_x = win->x + 2;
    origin_y = win->y + 14;
    int new_cols = (win->w - 4) / CHAR_WIDTH;
    if(new_cols > MAX_COLS) new_cols = MAX_COLS;
    int new_rows = (win->h - 16) / CHAR_HEIGHT;
    if(new_rows > MAX_ROWS) new_rows = MAX_ROWS;
    if(new_cols != term_cols || new_rows != term_rows) {
        term_cols = new_cols;
        term_rows = new_rows;
        row = col = 0;
        for(int y=0;y<MAX_ROWS;y++)
            for(int x=0;x<MAX_COLS;x++) {
                text_buffer[y][x] = ' ';
                color_buffer[y][x] = BACKGROUND_COLOR;
            }
        recursive_update_tick();
    } else if(old_x != origin_x || old_y != origin_y) {
        recursive_update_tick();
    }
}

static void draw_cursor(int visible) {
    if(visible)
        screen_put_char_offset(col, row, CURSOR_CHAR, CURSOR_COLOR,
                               origin_x, origin_y);
    else
        screen_put_char_offset(col, row, text_buffer[row][col],
                               color_buffer[row][col], origin_x, origin_y);
}

static void put_entry_at(char c, uint8_t color, int x, int y) {
    if(x < 0 || x >= term_cols || y < 0 || y >= term_rows)
        return;
    text_buffer[y][x] = c;
    color_buffer[y][x] = color;
    screen_put_char_offset(x, y, c, color, origin_x, origin_y);
}

static void scroll(void) {
    for(int y=1; y<term_rows; y++) {
        for(int x=0; x<term_cols; x++) {
            text_buffer[y-1][x] = text_buffer[y][x];
            color_buffer[y-1][x] = color_buffer[y][x];
            screen_put_char_offset(x, y-1,
                                  text_buffer[y][x], color_buffer[y][x],
                                  origin_x, origin_y);
        }
    }
    for(int x=0; x<term_cols; x++) {
        text_buffer[term_rows-1][x] = ' ';
        color_buffer[term_rows-1][x] = BACKGROUND_COLOR;
        screen_put_char_offset(x, term_rows-1, ' ', BACKGROUND_COLOR,
                              origin_x, origin_y);
    }
}

static void putchar(char c) {
    draw_cursor(0);
    if(c == '\n') {
        col = 0;
        row++;
    } else {
        put_entry_at(c, text_color, col, row);
        col++;
        if(col >= term_cols) {
            col = 0;
            row++;
        }
    }
    if(row >= term_rows) {
        scroll();
        row = term_rows-1;
    }
    draw_cursor(1);
}

void terminal_init(void) {
    keyboard_flush();
    for(int y=0; y<term_rows; y++)
        for(int x=0; x<term_cols; x++)
            put_entry_at(' ', BACKGROUND_COLOR, x, y);
    row = 0;
    col = 0;
    draw_cursor(1);
    fs_init();
    current_dir = fs_get_root();
    current_path[0] = '/';
    current_path[1] = '\0';
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

static void read_line(char* buf, size_t max, window_t *win) {
    size_t idx = 0;
    int blink = 0;
    cursor_on = 1;
    draw_cursor(1);
    mouse_draw(BACKGROUND_COLOR);
    while(idx < max-1) {
        mouse_update();
        window_handle_mouse(win, mouse_get_x(), mouse_get_y(), mouse_clicked());
        if(!win->visible) { buf[0] = '\0'; break; }
        terminal_set_window(win);
        window_draw(win);
        mouse_draw(BACKGROUND_COLOR);
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
                draw_cursor(0);
                if(col == 0) {
                    col = term_cols-1;
                    row--;
                } else {
                    col--;
                }
                idx--;
                put_entry_at(' ', BACKGROUND_COLOR, col, row);
                draw_cursor(1);
            }
            continue;
        }
        if(c == '\t') {
            int spaces = 4 - (col % 4);
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
    mouse_draw(BACKGROUND_COLOR);
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
    print("cat <file>  - view file\n");
    print("touch <file> - create file\n");
    print("write <file> <text> - write text to file\n");
    print("rm <file>   - delete file\n");
    print("mv <a> <b>  - rename file\n");
    print("mkdir <name> - create directory\n");
    print("rmdir <name> - remove directory\n");
    print("date       - show build date\n");
    print("whoami     - display current user\n");
    print("hello      - greet the user\n");
    print("uptime     - show uptime counter\n");
    print("meminfo    - show memory usage\n");
}

static void cmd_clear(void) {
    for(int y=0; y<term_rows; y++)
        for(int x=0; x<term_cols; x++)
            put_entry_at(' ', BACKGROUND_COLOR, x, y);
    row=0; col=0;
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

static void cmd_date(void) {
    print("Build date: " __DATE__ " " __TIME__ "\n");
}

static void cmd_whoami(void) {
    print("root\n");
}

static void cmd_hello(void) {
    print("Hello, user!\n");
}

static unsigned int uptime_counter = 0;
static void cmd_uptime(void) {
    print("Uptime: ");
    print_int(uptime_counter);
    print(" ticks\n");
}

static void cmd_meminfo(void) {
    print("Total: ");
    print_int(mem_total());
    print(" bytes, Used: ");
    print_int(mem_used());
    print(" bytes, Free: ");
    print_int(mem_free());
    print(" bytes\n");
}

#include "fs.h"

static void cmd_dir(void) {
    for(int i=0; i<current_dir->child_count; i++) {
        print(current_dir->children[i].name);
        if(current_dir->children[i].is_dir) print("/");
        print("  ");
    }
    putchar('\n');
}

static void cmd_cd(const char* args) {
    if(args[0]=='\0' || streq(args, "/")) {
        current_dir = fs_get_root();
        current_path[0] = '/'; current_path[1] = '\0';
        return;
    }
    if(streq(args, "..")) {
        if(current_dir->parent) {
            current_dir = current_dir->parent;
            if(current_dir == fs_get_root()) {
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
    fs_entry* d = fs_find_subdir(current_dir, args);
    if(d) {
        current_dir = d;
        if(current_dir == fs_get_root()) {
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

static void cmd_cat(const char* name) {
    fs_entry* f = fs_find_entry(current_dir, name);
    if(f && !f->is_dir) {
        print(fs_read_file(f));
        putchar('\n');
    } else {
        print("File not found\n");
    }
}

static void cmd_touch(const char* name) {
    if(fs_find_entry(current_dir, name)) {
        print("File exists\n");
        return;
    }
    if(fs_create_file(current_dir, name)) {
        print("File created\n");
    } else {
        print("Cannot create file\n");
    }
}

static void cmd_mkdir(const char* name) {
    if(fs_find_entry(current_dir, name)) {
        print("Entry exists\n");
        return;
    }
    if(fs_create_dir(current_dir, name)) {
        print("Directory created\n");
    } else {
        print("Cannot create directory\n");
    }
}

static void cmd_rmdir(const char* name) {
    fs_entry* d = fs_find_entry(current_dir, name);
    if(d && d->is_dir && d->child_count==0) {
        if(fs_delete_entry(current_dir, name))
            print("Directory removed\n");
        else
            print("Error removing directory\n");
    } else {
        print("Directory not empty or not found\n");
    }
}

static void cmd_write(const char* args) {
    int i = 0;
    while(args[i] && args[i] != ' ') i++;
    if(args[i]==0 || args[i+1] != '"') {
        print("Usage: write <file> \"text\"\n");
        return;
    }
    char fname[32];
    int j=0; for(j=0;j<i && j<31;j++) fname[j]=args[j]; fname[j]=0;
    i++; /* skip space */
    if(args[i] != '"') { print("Usage: write <file> \"text\"\n"); return; }
    i++; /* skip opening quote */
    char textbuf[256];
    int k=0;
    while(args[i] && args[i] != '"' && k<255) {
        textbuf[k++] = args[i++];
    }
    textbuf[k] = '\0';
    if(args[i] != '"') { print("Usage: write <file> \"text\"\n"); return; }
    fs_entry* f = fs_find_entry(current_dir, fname);
    if(!f) f = fs_create_file(current_dir, fname);
    if(f) {
        fs_write_file(f, textbuf);
        print("Written\n");
    } else {
        print("Cannot write file\n");
    }
}

static void cmd_rm(const char* name) {
    if(fs_delete_entry(current_dir, name))
        print("Deleted\n");
    else
        print("No such file\n");
}

static void cmd_mv(const char* args) {
    char src[32];
    char dst[32];
    int i=0; while(args[i] && args[i]!=' ') { src[i]=args[i]; i++; if(i>=31) break; }
    src[i]=0;
    if(args[i]==0){ print("Usage: mv <src> <dst>\n"); return; }
    int j=0; i++; while(args[i] && j<31){ dst[j++]=args[i++]; }
    dst[j]=0;
    fs_entry* f = fs_find_entry(current_dir, src);
    if(f) {
        int k=0; while(dst[k] && k<31){ f->name[k]=dst[k]; k++; }
        f->name[k]=0;
        print("Renamed\n");
    } else {
        print("No such file\n");
    }
}

static void cmd_cp(const char* args) {
    char src[32];
    char dst[32];
    int i = 0;
    while(args[i] && args[i] != ' ') { if(i < 31) src[i] = args[i]; i++; }
    src[i < 31 ? i : 31] = '\0';
    if(args[i] == 0) { print("Usage: cp <src> <dst>\n"); return; }
    int j = 0; i++; while(args[i] && j < 31) { dst[j++] = args[i++]; }
    dst[j] = '\0';
    fs_entry* f = fs_find_entry(current_dir, src);
    if(!f || f->is_dir) { print("No such file\n"); return; }
    fs_entry* d = fs_find_entry(current_dir, dst);
    if(!d) d = fs_create_file(current_dir, dst);
    if(d && !d->is_dir) {
        fs_write_file(d, fs_read_file(f));
        print("Copied\n");
    } else {
        print("Cannot copy\n");
    }
}

static void cmd_shutdown(void) {
    print("Shutting down...\n");
    while(1) { __asm__("hlt"); }
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
    } else if(strprefix(line, "cat ")) {
        cmd_cat(line+4);
    } else if(strprefix(line, "touch ")) {
        cmd_touch(line+6);
    } else if(strprefix(line, "write ")) {
        cmd_write(line+6);
    } else if(strprefix(line, "rm ")) {
        cmd_rm(line+3);
    } else if(strprefix(line, "mv ")) {
        cmd_mv(line+3);
    } else if(strprefix(line, "cp ")) {
        cmd_cp(line+3);
    } else if(strprefix(line, "mkdir ")) {
        cmd_mkdir(line+6);
    } else if(strprefix(line, "rmdir ")) {
        cmd_rmdir(line+6);
    } else if(streq(line, "shutdown")) {
        cmd_shutdown();
    } else if(streq(line, "date")) {
        cmd_date();
    } else if(streq(line, "whoami")) {
        cmd_whoami();
    } else if(streq(line, "hello")) {
        cmd_hello();
    } else if(streq(line, "uptime")) {
        cmd_uptime();
    } else if(streq(line, "meminfo")) {
        cmd_meminfo();
    } else if(streq(line, "ls")) {
        cmd_dir();
    } else if(line[0]) {
        print("Unknown command\n");
    }
}

void terminal_run(window_t *win) {
    char buf[128];
    mouse_set_visible(1);
    mouse_draw(BACKGROUND_COLOR);
    while(win->visible) {
        print("> ");
        read_line(buf, sizeof(buf), win);
        if(!win->visible) break;
        if(buf[0])
            execute(buf);
        terminal_recursive_update();
        uptime_counter++;
    }
}
