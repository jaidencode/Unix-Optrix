#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "string.h"

static void prompt(void) {
    terminal_write("optrix> ");
}

static void execute(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        terminal_write("Commands: help, echo, clear\n");
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        terminal_write(cmd + 5);
        terminal_write("\n");
    } else if (strcmp(cmd, "clear") == 0) {
        terminal_clear();
    } else {
        terminal_write("Unknown command\n");
    }
}

void shell_run(void) {
    char buf[80];
    int idx = 0;
    prompt();
    while (1) {
        char c = keyboard_getchar();
        if (c == '\n') {
            buf[idx] = 0;
            terminal_write("\n");
            execute(buf);
            idx = 0;
            prompt();
        } else if (c == 8) {
            if (idx > 0) {
                --idx;
                terminal_write("\b \b");
            }
        } else if (c && idx < 79) {
            buf[idx++] = c;
            char out[2] = {c, 0};
            terminal_write(out);
        }
    }
}
