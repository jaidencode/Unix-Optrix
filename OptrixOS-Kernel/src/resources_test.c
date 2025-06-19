#include "resources_test.h"
#include "fs.h"
#include "screen.h"
#include <stdint.h>

static int r_col = 0;
static int r_row = 0;

static void r_put(char c){
    if(c=='\n'){
        r_row++; r_col=0;
    } else {
        screen_put_char(r_col,r_row,c,0x1E);
        r_col++;
        if(r_col>=SCREEN_COLS){ r_col=0; r_row++; }
    }
}

static void r_print(const char*s){ while(*s) r_put(*s++); }

void resources_test(void){
    fs_entry* dir = fs_find_path("resources");
    if(!dir || !dir->is_dir){
        r_print("resources dir not found\n");
        return;
    }
    r_print("resource files:\n");
    for(fs_entry* f=dir->child; f; f=f->sibling){
        r_print(" - ");
        r_print(f->name);
        r_print(" (");
        char size_buf[16];
        int idx=0;
        unsigned int sz=f->size;
        if(sz==0){size_buf[idx++]='0';}
        else{
            while(sz>0&&idx<15){size_buf[idx++]='0'+(sz%10);sz/=10;}
        }
        for(int i=idx-1;i>=0;i--) r_put(size_buf[i]);
        r_print(" bytes)");
        if(f->embedded)
            r_print(" [embedded]\n");
        else
            r_print(" [disk]\n");

        const char* content = fs_read_file(f);
        r_print("   \"");
        for(int i=0; i<32 && content[i]; i++)
            r_put(content[i]);
        r_print("\"\n");
    }
    r_print("\n");
}
