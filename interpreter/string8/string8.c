#pragma once

#include "string8.h"
//#define STRLEN(s)
static String_t* new_sized_string8(const int64_t base_size) {
    String_t* str = malloc(sizeof(String_t));
    str->length = base_size;
    str->str = malloc(sizeof(char)*str->length);
    return str;
}

void del_string8(String_t* str8) {
    free(str8->str);
    free(str8);
}

int64_t string8_search(const String_t* haystack, const String_t* needle) {
    // TODO: implement non-naive algorithm for string search.
    if (haystack->length < needle->length) return -1;
    int64_t i = 0;
    while (i < haystack->length) {
        if (!memcmp(haystack->str+i, needle->str, needle->length)) return i;
        i++;
    }
    return -1;
}
