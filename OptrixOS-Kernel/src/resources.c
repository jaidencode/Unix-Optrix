#include "resources.h"
#include "fs.h"

const resource_file resource_files[] = {
    {"welcome.txt", "Welcome to OptrixOS!\nEnjoy your stay.\n"},
    {"logo.txt", "  ____        _   _      ____  _____ \n / __ \\      | | | |    / __ \\|  __ \\
| |  | |_ __ | |_| |_  | |  | | |__) |\n| |  | | '_ \\| __| __| | |  | |  ___/\n| |__| | | | | |_| |_  | |__| | |    \n \\____/|_| |_|\\__|\\__|  \\____/|_|    \n"},
    {"hello.txt", "Hello World\n"}
};
const int resource_files_count = 3;

void resources_embed(void){
    fs_entry* root = fs_get_root();
    fs_entry* res_dir = fs_find_subdir(root, "resources");
    if(!res_dir)
        res_dir = fs_create_dir(root, "resources");

    for(int i=0;i<resource_files_count;i++){
        const char* name = resource_files[i].name;
        fs_entry* f = fs_find_entry(res_dir, name);
        if(!f)
            f = fs_create_file(res_dir, name);
        if(f){
            fs_write_file(f, resource_files[i].data);
            f->embedded = 1;
        }
    }
}
