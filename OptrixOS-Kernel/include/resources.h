#ifndef RESOURCES_H
#define RESOURCES_H
typedef struct { const char* name; const char* data; } resource_file;
extern const int resource_files_count;
extern const resource_file resource_files[];
#endif
