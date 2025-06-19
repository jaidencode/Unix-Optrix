#include "terminal.h"
#include "keyboard.h"
#include "screen.h"
#include "graphics.h"
#include <stdint.h>
#include <stddef.h>

#define DEFAULT_COLOR 0x1E

static int row = 0;
static int col = 0;
static uint8_t text_color = DEFAULT_COLOR;

static void scroll(void){
    volatile uint16_t *vga = (uint16_t*)0xB8000;
    for(int y=1;y<SCREEN_ROWS;y++)
        for(int x=0;x<SCREEN_COLS;x++)
            vga[(y-1)*SCREEN_COLS+x] = vga[y*SCREEN_COLS+x];
    for(int x=0;x<SCREEN_COLS;x++)
        vga[(SCREEN_ROWS-1)*SCREEN_COLS+x] = (BACKGROUND_COLOR<<8) | ' ';
}

static void put_char(char c){
    if(c=='\n'){col=0;row++;}
    else if(c=='\b'){if(col>0){col--;screen_put_char(col,row,' ',text_color);}}
    else{screen_put_char(col,row,c,text_color);col++;if(col>=SCREEN_COLS){col=0;row++;}}
    if(row>=SCREEN_ROWS){scroll();row=SCREEN_ROWS-1;}
    screen_set_cursor(col,row);
}

static void print(const char*s){while(*s)put_char(*s++);} 

static void print_int(int n){
    char b[16];int i=0,neg=0;
    if(n==0){b[i++]='0';}
    else {
        if(n<0){neg=1;n=-n;}
        while(n>0&&i<15){b[i++]='0'+(n%10);n/=10;}
        if(neg&&i<15) b[i++]='-';
    }
    for(int j=i-1;j>=0;j--) put_char(b[j]);
}

static void read_line(char*buf,size_t max){size_t i=0;while(1){char c=keyboard_getchar();if(!c)continue;if(c=='\n'){put_char('\n');break;}if(c=='\b'){if(i>0){i--;put_char('\b');}continue;}if(i<max-1){buf[i++]=c;put_char(c);}}buf[i]='\0';}

static void cmd_help(const char*);static void cmd_clear(const char*);static void cmd_echo(const char*);static void cmd_about(const char*);static void cmd_add(const char*);static void cmd_mul(const char*);static void cmd_rand(const char*);static void cmd_date(const char*);static void cmd_uptime(const char*);static void cmd_ver(const char*);static void cmd_whoami(const char*);static void cmd_banner(const char*);static void cmd_shutdown(const char*);static void cmd_dummy(const char*);

static unsigned int uptime = 0;

typedef struct{const char*name;void(*func)(const char*);} command_t;

#define DUMMY_CMDS 37

static command_t commands[50];

static void init_commands(void){
    int idx=0;
    commands[idx++] = (command_t){"help",cmd_help};
    commands[idx++] = (command_t){"clear",cmd_clear};
    commands[idx++] = (command_t){"cls",cmd_clear};
    commands[idx++] = (command_t){"echo",cmd_echo};
    commands[idx++] = (command_t){"about",cmd_about};
    commands[idx++] = (command_t){"add",cmd_add};
    commands[idx++] = (command_t){"mul",cmd_mul};
    commands[idx++] = (command_t){"rand",cmd_rand};
    commands[idx++] = (command_t){"date",cmd_date};
    commands[idx++] = (command_t){"uptime",cmd_uptime};
    commands[idx++] = (command_t){"ver",cmd_ver};
    commands[idx++] = (command_t){"whoami",cmd_whoami};
    commands[idx++] = (command_t){"banner",cmd_banner};
    commands[idx++] = (command_t){"shutdown",cmd_shutdown};
    for(int i=0;i<DUMMY_CMDS;i++){
        static char names[DUMMY_CMDS][8];
        names[i][0]='c';names[i][1]='m';names[i][2]='d';names[i][3]='0'+(i/10);names[i][4]='0'+(i%10);names[i][5]=0;
        commands[idx++] = (command_t){names[i],cmd_dummy};
    }
}

static void cmd_help(const char*){print("Available commands:\n");for(int i=0;i<50;i++){print(commands[i].name);print(" ");}put_char('\n');}
static void cmd_clear(const char*){screen_clear();row=col=0;}
static void cmd_echo(const char*args){print(args);put_char('\n');}
static void cmd_about(const char*){print("OptrixOS minimal terminal\n");}
static void parse_two_ints(const char*args,int*a,int*b){int idx=0,neg=0;*a=*b=0;while(args[idx]==' ')idx++;if(args[idx]=='-'){neg=1;idx++;}while(args[idx]>='0'&&args[idx]<='9'){*a=*a*10+(args[idx]-'0');idx++;}if(neg)*a=-*a;while(args[idx]==' ')idx++;neg=0;if(args[idx]=='-'){neg=1;idx++;}while(args[idx]>='0'&&args[idx]<='9'){*b=*b*10+(args[idx]-'0');idx++;}if(neg)*b=-*b;}
static void cmd_add(const char*args){int a,b;parse_two_ints(args,&a,&b);print_int(a+b);put_char('\n');}
static void cmd_mul(const char*args){int a,b;parse_two_ints(args,&a,&b);print_int(a*b);put_char('\n');}
static unsigned int rand_state=1234567;static void cmd_rand(const char*){rand_state=rand_state*1103515245+12345;print_int(rand_state&0x7fffffff);put_char('\n');}
static void cmd_date(const char*){print("Build: " __DATE__ " " __TIME__ "\n");} 
static void cmd_uptime(const char*){print_int(uptime);put_char('\n');}
static void cmd_ver(const char*){print("0.2\n");} 
static void cmd_whoami(const char*){print("root\n");} 
static void cmd_banner(const char*){print("Welcome to OptrixOS\n");} 
static void cmd_shutdown(const char*){print("Shutdown\n");while(1){__asm__("hlt");}} 
static void cmd_dummy(const char*){print("Executed\n");} 

static void execute(const char*line){
    for(int i=0;i<50;i++){
        size_t j=0;while(line[j]&&commands[i].name[j]&&line[j]==commands[i].name[j])j++;if(line[j]==0&&commands[i].name[j]==0){commands[i].func("");return;}if(line[j]==' '&&commands[i].name[j]==0){commands[i].func(line+j+1);return;}}
    if(line[0]) print("Unknown\n");
}

void terminal_init(void){
    screen_clear(); row=col=0; init_commands(); }

void terminal_run(void){
    char buf[128]; while(1){print("$ "); read_line(buf,sizeof(buf)); if(buf[0]) execute(buf); uptime++;}}
