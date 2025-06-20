#ifndef ROOT_FILES_H
#define ROOT_FILES_H
typedef struct { const char* path; const unsigned char* data; unsigned int size; } root_file;
extern const int root_files_count;
extern const root_file root_files[];
#endif
