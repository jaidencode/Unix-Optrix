#include "fs.h"
#include "disk.h"
#include "build_params.h"
#include <stddef.h>
#include <stdint.h>

static int streq(const char*a,const char*b){while(*a&&*b){if(*a!=*b)return 0;a++;b++;}return *a==*b;}

#define MAX_ROOT_ENTRIES 20
#define MAX_EXTRA_DIRS 20
#define MAX_EXTRA_DIR_ENTRIES 20
static fs_entry root_entries[MAX_ROOT_ENTRIES];
static fs_entry extra_dir_entries[MAX_EXTRA_DIRS][MAX_EXTRA_DIR_ENTRIES];
static int extra_dir_used=0;
static fs_entry root_dir={"/",1,NULL,root_entries,0,""};

fs_entry* fs_get_root(void){return &root_dir;}

fs_entry* fs_find_subdir(fs_entry*dir,const char*name){for(int i=0;i<dir->child_count;i++)if(dir->children[i].is_dir&&streq(dir->children[i].name,name))return &dir->children[i];return 0;}
fs_entry* fs_find_entry(fs_entry*dir,const char*name){for(int i=0;i<dir->child_count;i++)if(streq(dir->children[i].name,name))return &dir->children[i];return 0;}

fs_entry* fs_create_dir(fs_entry*dir,const char*name){int limit=(dir==&root_dir)?MAX_ROOT_ENTRIES:MAX_EXTRA_DIR_ENTRIES;if(dir->child_count>=limit||extra_dir_used>=MAX_EXTRA_DIRS)return 0;fs_entry*e=&dir->children[dir->child_count++];int i=0;for(;i<31&&name[i];i++)e->name[i]=name[i];e->name[i]=0;e->is_dir=1;e->parent=dir;e->children=extra_dir_entries[extra_dir_used++];e->child_count=0;return e;}
fs_entry* fs_create_file(fs_entry*dir,const char*name){int limit=(dir==&root_dir)?MAX_ROOT_ENTRIES:MAX_EXTRA_DIR_ENTRIES;if(dir->child_count>=limit)return 0;fs_entry*e=&dir->children[dir->child_count++];int i=0;for(;i<31&&name[i];i++)e->name[i]=name[i];e->name[i]=0;e->is_dir=0;e->parent=dir;e->children=NULL;e->child_count=0;e->content[0]=0;return e;}

int fs_delete_entry(fs_entry*dir,const char*name){for(int i=0;i<dir->child_count;i++)if(streq(dir->children[i].name,name)){for(int j=i;j<dir->child_count-1;j++)dir->children[j]=dir->children[j+1];dir->child_count--;return 1;}return 0;}

void fs_write_file(fs_entry*f,const char*text){if(!f||f->is_dir)return;int k=0;while(text[k]&&k<255){f->content[k]=text[k];k++;}f->content[k]=0;}
const char* fs_read_file(fs_entry*f){if(!f||f->is_dir)return "";return f->content;}

static fs_entry* ensure_dir(const char*path){fs_entry*dir=&root_dir;char name[32];int idx=0;for(int i=0;path[i];i++){if(path[i]=='/') {if(idx){name[idx]=0;fs_entry*sub=fs_find_subdir(dir,name);if(!sub)sub=fs_create_dir(dir,name);dir=sub;idx=0;}}else if(idx<31){name[idx++]=path[i];}}if(idx){name[idx]=0;fs_entry*sub=fs_find_subdir(dir,name);if(!sub)sub=fs_create_dir(dir,name);dir=sub;}return dir;}

static void create_file_with_path(const char*path,const char*data){const char*slash=NULL;for(int i=0;path[i];i++)if(path[i]=='/')slash=path+i;fs_entry*dir=&root_dir;const char*name=path;char parent[64];if(slash){int len=slash-path;for(int i=0;i<len&&i<63;i++)parent[i]=path[i];parent[len]=0;dir=ensure_dir(parent);name=slash+1;}fs_entry*f=fs_create_file(dir,name);if(f)fs_write_file(f,data);} 

void fs_init(void){root_dir.child_count=0;extra_dir_used=0;uint32_t total=FS_SECTORS;uint8_t*buf=(uint8_t*)0x80000;for(uint32_t i=0;i<total;i++)ata_read_sector(FS_START_SECTOR+i,buf+i*512);uint32_t magic=*(uint32_t*)buf;if(magic!=0x46534631)return;uint32_t count=*(uint32_t*)(buf+4);size_t off=8;for(uint32_t e=0;e<count;e++){uint8_t type=buf[off++];uint8_t plen=buf[off++];char path[64];for(int i=0;i<plen&&i<63;i++)path[i]=buf[off++];path[plen]=0;if(type){ensure_dir(path);}else{uint32_t size=*(uint32_t*)(buf+off);off+=4;char*text=(char*)(buf+off);off+=size;if(size>255){size=255;}char tmp[256];for(uint32_t i=0;i<size;i++)tmp[i]=text[i];tmp[size]=0;create_file_with_path(path,tmp);}}}
