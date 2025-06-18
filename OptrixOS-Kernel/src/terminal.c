#include "terminal.h"
#include "keyboard.h"
#include "screen.h"
#include "fs.h"
#include "mem.h"
#include <stdint.h>
#include <stddef.h>

#define DEFAULT_COLOR 0x1E

static int row = 0;
static int col = 0;
static uint8_t text_color = DEFAULT_COLOR;

static void scroll(void) {
    volatile uint16_t *vga = (uint16_t*)0xB8000;
    for(int y=1; y<SCREEN_ROWS; y++)
        for(int x=0; x<SCREEN_COLS; x++)
            vga[(y-1)*SCREEN_COLS + x] = vga[y*SCREEN_COLS + x];
    for(int x=0; x<SCREEN_COLS; x++)
        vga[(SCREEN_ROWS-1)*SCREEN_COLS + x] = (BACKGROUND_COLOR<<8) | ' ';
}

static void put_char(char c) {
    if(c=='\n') {
        col = 0;
        row++;
    } else if(c=='\b') {
        if(col>0) { col--; screen_put_char(col,row,' ',text_color); }
    } else {
        screen_put_char(col,row,c,text_color);
        col++;
        if(col>=SCREEN_COLS) { col=0; row++; }
    }
    if(row>=SCREEN_ROWS) { scroll(); row=SCREEN_ROWS-1; }
}

static void print(const char *s) { while(*s) put_char(*s++); }

static void print_int(int n) {
    char buf[16]; int i=0, neg=0;
    if(n==0) { buf[i++]='0'; }
    else {
        if(n<0){neg=1;n=-n;}
        while(n>0&&i<15){ buf[i++]='0'+(n%10); n/=10; }
        if(neg&&i<15) buf[i++]='-';
    }
    for(int j=i-1;j>=0;j--) put_char(buf[j]);
}

static void read_line(char *buf, size_t max) {
    size_t idx=0;
    while(1) {
        char c = keyboard_getchar();
        if(!c) continue;
        if(c=='\n') { put_char('\n'); break; }
        if(c=='\b') {
            if(idx>0) { idx--; put_char('\b'); }
            continue;
        }
        if(idx<max-1) {
            buf[idx++]=c; put_char(c);
        }
    }
    buf[idx]='\0';
}

static void cmd_help(void);
static void cmd_clear(void);
static void cmd_echo(const char*);
static void cmd_about(void);
static void cmd_add(const char*);
static void cmd_mul(const char*);
static void cmd_dir(void);
static void cmd_cd(const char*);
static void cmd_pwd(void);
static void cmd_cat(const char*);
static void cmd_touch(const char*);
static void cmd_rm(const char*);
static void cmd_mkdir(const char*);
static void cmd_rmdir(const char*);
static void cmd_mv(const char*);
static void cmd_cp(const char*);
static void cmd_rand(void);
static void cmd_date(void);
static void cmd_uptime(void);
static void cmd_shutdown(void);
static void cmd_ver(void);
static void cmd_whoami(void);
static void cmd_banner(void);

static unsigned int uptime = 0;
static fs_entry* current_dir;
static char current_path[32] = "/";
static void update_current_path(void){fs_get_path(current_dir,current_path,sizeof(current_path));}

static int streq(const char*a,const char*b){while(*a&&*b){if(*a!=*b) return 0;a++;b++;}return *a==*b;}
static int strprefix(const char*s,const char*p){while(*p){if(*s!=*p) return 0;s++;p++;}return 1;}

static void cmd_help(void){
    print("help clear cls echo about add mul dir cd pwd cat touch rm mv mkdir rmdir cp rand date uptime ver whoami banner shutdown\n");
}

static void cmd_clear(void){screen_clear(); row=col=0;}
static void cmd_echo(const char*args){print(args);put_char('\n');}
static void cmd_about(void){print("OptrixOS text mode\n");}

static void parse_two_ints(const char*args,int* a,int* b){int idx=0,neg=0;*a=*b=0;while(args[idx]==' ')idx++;if(args[idx]=='-'){neg=1;idx++;}while(args[idx]>='0'&&args[idx]<='9'){*a=*a*10+(args[idx]-'0');idx++;}if(neg)*a=-*a;while(args[idx]==' ')idx++;neg=0;if(args[idx]=='-'){neg=1;idx++;}while(args[idx]>='0'&&args[idx]<='9'){*b=*b*10+(args[idx]-'0');idx++;}if(neg)*b=-*b;}
static void cmd_add(const char*args){int a,b;parse_two_ints(args,&a,&b);print_int(a+b);put_char('\n');}
static void cmd_mul(const char*args){int a,b;parse_two_ints(args,&a,&b);print_int(a*b);put_char('\n');}

static unsigned int rand_state=1234567;static void cmd_rand(void){rand_state=rand_state*1103515245+12345;print_int(rand_state&0x7fffffff);put_char('\n');}

static void cmd_dir(void){for(int i=0;i<current_dir->child_count;i++){print(current_dir->children[i].name);if(current_dir->children[i].is_dir)print("/");print("  ");}put_char('\n');}
static void cmd_cd(const char*args){
    while(*args==' ') args++;
    if(args[0]==0) { current_dir=fs_get_root(); update_current_path(); return; }
    fs_entry* target = fs_resolve_path(current_dir, args);
    if(target && target->is_dir){
        current_dir = target;
        update_current_path();
    } else {
        print("No such directory\n");
    }
}
static void cmd_pwd(void){print(current_path);put_char('\n');}
static void cmd_cat(const char*name){
    while(*name==' ') name++;
    fs_entry*f=fs_resolve_path(current_dir,name);
    if(f&&!f->is_dir){print(fs_read_file(f));put_char('\n');}
    else print("File not found\n");
}
static fs_entry* path_parent(const char*path,char*name_out){
    fs_entry* dir=(path[0]=='/')?fs_get_root():current_dir;
    const char*last=path;const char*p=path;
    while(*p){if(*p=='/')last=p+1;p++;}
    int len=0;while(last[len]&&len<31){name_out[len]=last[len];len++;}
    name_out[len]='\0';
    if(last==path){return (path[0]=='/')?fs_get_root():current_dir;}
    int plen=last-path;char tmp[64];if(plen>=64)plen=63;for(int i=0;i<plen;i++)tmp[i]=path[i];tmp[plen]='\0';
    if(plen==0)return fs_get_root();
    return fs_resolve_path(dir,tmp);
}
static void cmd_touch(const char*name){
    while(*name==' ')name++;
    char fname[32];
    fs_entry*dir=path_parent(name,fname);
    if(!dir){print("Invalid path\n");return;}
    if(fs_find_entry(dir,fname)){print("Exists\n");return;}
    if(fs_create_file(dir,fname))print("Created\n");else print("Fail\n");
}
static void cmd_rm(const char*name){
    while(*name==' ')name++;
    fs_entry*f=fs_resolve_path(current_dir,name);
    if(f&&f->parent&&fs_delete_entry(f->parent,f->name))print("Removed\n");else print("Not found\n");
}
static void cmd_mkdir(const char*name){
    while(*name==' ')name++;
    char dname[32];
    fs_entry*dir=path_parent(name,dname);
    if(!dir){print("Invalid path\n");return;}
    if(fs_create_dir(dir,dname))print("Dir created\n");else print("Fail\n");
}
static void cmd_rmdir(const char*name){
    while(*name==' ')name++;
    fs_entry*dir=fs_resolve_path(current_dir,name);
    if(dir&&dir->is_dir&&dir->child_count==0&&dir->parent){
        fs_delete_entry(dir->parent,dir->name);print("Dir removed\n");
    }else print("Not empty\n");
}
static void cmd_mv(const char*args){
    char src[64];char dst[64];int i=0;
    while(args[i]&&args[i]!=' '&&i<63){src[i]=args[i];i++;}src[i]='\0';
    if(args[i]==0){print("Usage\n");return;}i++;
    int j=0;while(args[i]&&j<63){dst[j++]=args[i++];}dst[j]='\0';
    fs_entry*f=fs_resolve_path(current_dir,src);
    if(!f){print("No file\n");return;}
    char newname[32];fs_entry*newdir=path_parent(dst,newname);
    if(!newdir){print("Invalid path\n");return;}
    if(f->parent!=newdir){
        if(newdir->child_count>=20){print("Fail\n");return;}
        fs_entry*copy=fs_create_file(newdir,newname);
        if(!copy){print("Fail\n");return;}
        copy->is_dir=f->is_dir;copy->child_count=f->child_count;copy->children=f->children;copy->parent=newdir;fs_write_file(copy,f->content);
        fs_delete_entry(f->parent,f->name);
    }else{
        int k=0;while(newname[k]&&k<31){f->name[k]=newname[k];k++;}f->name[k]='\0';
    }
    print("Renamed\n");
}
static void cmd_cp(const char*args){
    char src[64];char dst[64];int i=0;
    while(args[i]&&args[i]!=' '&&i<63){src[i]=args[i];i++;}src[i]='\0';
    if(args[i]==0){print("Usage\n");return;}i++;
    int j=0;while(args[i]&&j<63){dst[j++]=args[i++];}dst[j]='\0';
    fs_entry*f=fs_resolve_path(current_dir,src);
    if(!f||f->is_dir){print("No file\n");return;}
    char newname[32];fs_entry*dir=path_parent(dst,newname);
    if(!dir){print("Invalid path\n");return;}
    fs_entry*d=fs_find_entry(dir,newname);
    if(!d)d=fs_create_file(dir,newname);
    if(d&&!d->is_dir){fs_write_file(d,fs_read_file(f));print("Copied\n");}else print("Fail\n");
}
static void cmd_date(void){print("Build: " __DATE__ " " __TIME__ "\n");}
static void cmd_uptime(void){print("Uptime: ");print_int(uptime);print("\n");}
static void cmd_shutdown(void){print("Shutdown\n");while(1){__asm__("hlt");}}
static void cmd_ver(void){print("OptrixOS 0.1 text\n");}
static void cmd_whoami(void){print("root\n");}
static void cmd_banner(void){fs_entry*f=fs_find_entry(fs_get_root(),"logo.txt");if(f){print(fs_read_file(f));put_char('\n');}}

static void execute(const char*line){
    if(streq(line,"help")) cmd_help();
    else if(streq(line,"clear")||streq(line,"cls")) cmd_clear();
    else if(strprefix(line,"echo ")) cmd_echo(line+5);
    else if(streq(line,"about")) cmd_about();
    else if(strprefix(line,"add ")) cmd_add(line+4);
    else if(strprefix(line,"mul ")) cmd_mul(line+4);
    else if(streq(line,"dir")||streq(line,"ls")) cmd_dir();
    else if(strprefix(line,"cd ")) cmd_cd(line+3);
    else if(streq(line,"pwd")) cmd_pwd();
    else if(strprefix(line,"cat ")) cmd_cat(line+4);
    else if(strprefix(line,"touch ")) cmd_touch(line+6);
    else if(strprefix(line,"rm ")) cmd_rm(line+3);
    else if(strprefix(line,"mv ")) cmd_mv(line+3);
    else if(strprefix(line,"mkdir ")) cmd_mkdir(line+6);
    else if(strprefix(line,"rmdir ")) cmd_rmdir(line+6);
    else if(strprefix(line,"cp ")) cmd_cp(line+3);
    else if(streq(line,"rand")) cmd_rand();
    else if(streq(line,"date")||streq(line,"time")) cmd_date();
    else if(streq(line,"uptime")) cmd_uptime();
    else if(streq(line,"ver")) cmd_ver();
    else if(streq(line,"whoami")) cmd_whoami();
    else if(streq(line,"banner")) cmd_banner();
    else if(streq(line,"shutdown")||streq(line,"exit")) cmd_shutdown();
    else if(line[0]) print("Unknown\n");
}

void terminal_init(void){
    screen_clear();
    fs_init();
    current_dir=fs_get_root();
    update_current_path();
    fs_entry* logo = fs_find_entry(current_dir, "logo.txt");
    if(logo) {
        print(fs_read_file(logo));
        put_char('\n');
    }
}

void terminal_run(void){
    char buf[128];
    while(1){
        print("$ ");
        read_line(buf,sizeof(buf));
        if(buf[0]) execute(buf);
        uptime++;
    }
}
