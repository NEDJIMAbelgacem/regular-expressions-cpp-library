#pragma once

#define is_char(x) (((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z') || (x) == ' ')
#define is_num(x) ((x) >= '0' && (x) <= '9')
#define EPSILON '\0'
#define MATCH_OTHERS '.'

int generate_uid() {
    static int id = 0;
    int res = id;
    id++;
    return res;
}