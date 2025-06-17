#include "desktop.h"
#include "screen.h"
#include "graphics.h"
#include "mouse.h"
#include "terminal.h"
#include "window.h"
#include "exec.h"
#include "fs.h"
#include "taskbar.h"

#define DESKTOP_BG_COLOR 0x17

static void draw_wallpaper(void) {
    for(int y=0; y<SCREEN_HEIGHT; y++) {
        for(int x=0; x<SCREEN_WIDTH; x++) {
            uint8_t c = ((x/8) ^ (y/8)) & 0x0F;
            put_pixel(x, y, c | 0x10);
        }
    }
}
#define MAX_ICONS 20

typedef struct { int x; int y; fs_entry* entry; } icon_t;

static icon_t icons[MAX_ICONS];
static int icon_count = 0;
static int click_timer = 0;
static int last_clicked = -1;
static int fs_ready = 0;

static void terminal_exec(window_t *win) {
    terminal_set_window(win);
    terminal_init();
    terminal_run(win);
}

static void draw_icons(void) {
    const int spacing = 40;
    int x = 50;
    int y = 50;
    for(int i=0; i<icon_count; i++) {
        icons[i].x = x;
        icons[i].y = y;
        const char *name = icons[i].entry->name;
        uint8_t col = 0x07;
        int len = 0;
        while(name[len]) len++;
        if(len > 4 && name[len-4]=='.' && name[len-3]=='o' &&
           name[len-2]=='p' && name[len-1]=='t')
            col = 0x03; /* OPT executable */
        draw_rect(x, y, 32, 32, col);
        char c = name[0];
        screen_put_char((x+12-OFFSET_X)/CHAR_WIDTH,
                       (y+12-OFFSET_Y)/CHAR_HEIGHT, c, 0x0F);
        x += spacing;
        if(x + 32 > SCREEN_WIDTH - 50) { x = 50; y += spacing; }
    }
}

static int in_icon(int idx, int x, int y) {
    return (x >= icons[idx].x && x < icons[idx].x+32 &&
            y >= icons[idx].y && y < icons[idx].y+32);
}

static void refresh_icons(void) {
    fs_entry* desk = fs_find_subdir(fs_get_root(), "desktop");
    if(!desk) { icon_count = 0; return; }
    if(desk->child_count != icon_count) {
        icon_count = 0;
        for(int i=0; i<desk->child_count && i<MAX_ICONS; i++)
            icons[icon_count++].entry = &desk->children[i];
        draw_wallpaper();
        draw_icons();
    }
}

void desktop_init(void) {
    if(!fs_ready) { fs_init(); fs_ready = 1; }
    draw_wallpaper();
    fs_entry* root = fs_get_root();
    fs_entry* desk = fs_find_subdir(root, "desktop");
    icon_count = 0;
    if(desk) {
        for(int i=0; i<desk->child_count && i<MAX_ICONS; i++) {
            icons[icon_count++].entry = &desk->children[i];
        }
    }
    draw_icons();
    exec_init();
    exec_register("terminal.opt", terminal_exec);
    taskbar_init();
}

void desktop_run(void) {
    mouse_init();
    mouse_set_visible(1);
    if(!mouse_is_present())
        ; /* fallback will use keyboard input */
    mouse_draw(DESKTOP_BG_COLOR);
    while(1) {
        mouse_update();
        refresh_icons();
        taskbar_draw();
        int mx = mouse_get_x();
        int my = mouse_get_y();

        if(mouse_clicked()) {
            for(int i=0;i<icon_count;i++) {
                if(in_icon(i,mx,my)) {
                    if(click_timer>0 && last_clicked==i && click_timer<20) {
                        exec_run(icons[i].entry->name);
                        desktop_init();
                    } else {
                        last_clicked=i;
                        click_timer=1;
                    }
                    break;
                }
            }
            taskbar_handle_click(mx, my);
        } else {
            if(click_timer>0) click_timer++;
            if(click_timer>30){ click_timer=0; last_clicked=-1; }
        }

        mouse_draw(DESKTOP_BG_COLOR);
    }
}
