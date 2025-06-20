#include "string.h"

size_t ustrlen(const char* s){ size_t l=0; while(s[l]) l++; return l; }
void ustrcpy(char* d, const char* s){ while((*d++ = *s++)); }
