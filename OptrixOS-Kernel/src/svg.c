#include "svg.h"
#include "graphics.h"
#include <stdint.h>

static int hex_digit(char c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static uint8_t parse_color(const char* s) {
    if(!s || s[0] != '#') return 0;
    int r = hex_digit(s[1]) * 16 + hex_digit(s[2]);
    int g = hex_digit(s[3]) * 16 + hex_digit(s[4]);
    int b = hex_digit(s[5]) * 16 + hex_digit(s[6]);
    return (uint8_t)((r + g + b) / 3); /* simple grayscale mapping */
}

static int parse_int(const char* s) {
    int v = 0;
    int i = 0;
    while(s[i] >= '0' && s[i] <= '9') {
        v = v * 10 + (s[i] - '0');
        i++;
    }
    return v;
}

static const char* find_attr(const char* p, const char* name) {
    int i = 0;
    while(p[i] && p[i] != '>') {
        int j = 0;
        while(name[j] && p[i+j] == name[j]) j++;
        if(name[j] == '\0' && p[i+j] == '=') {
            const char* v = p + i + j + 1;
            if(*v == '"' || *v == '\'') v++;
            return v;
        }
        i++;
    }
    return 0;
}

static void parse_rect(const char* p) {
    const char* val;
    int x=0, y=0, w=0, h=0;
    uint8_t color=0;
    val = find_attr(p, "x"); if(val) x = parse_int(val);
    val = find_attr(p, "y"); if(val) y = parse_int(val);
    val = find_attr(p, "width"); if(val) w = parse_int(val);
    val = find_attr(p, "height"); if(val) h = parse_int(val);
    val = find_attr(p, "fill"); if(val) color = parse_color(val);
    draw_rect(x, y, w, h, color);
}

void svg_render(const char* svg_text) {
    const char* p = svg_text;
    while(*p) {
        if(*p == '<') {
            p++;
            if(p[0]=='r' && p[1]=='e' && p[2]=='c' && p[3]=='t') {
                const char* attrs = p + 4;
                while(*p && *p != '>') p++; /* move to end of tag */
                parse_rect(attrs);
            }
        }
        if(*p) p++;
    }
}
