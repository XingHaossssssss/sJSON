#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define SJSON_IMPLEMENT

#include "sJSON.h"


char* json_text = "{ \"x\": 10, \"y\": 20, \"w\": 30, \"h\": 40 }";

typedef struct { int x, y, w, h } Rect;

bool eq(sJSON_Value val, char* s) {
    size_t len = val.end - val.start;
    return strlen(s) == len && !memcmp(s, val.start, len);
}

int main() {
    Rect rect = { 0 };
    sJSON r = sJSON_Reader(json_text, strlen(json_text));
    sJSON_Value obj = sJSON_Parse(&r);

    sJSON_Value key, val;
    while (sJSON_IterObject(&r, obj, &key, &val)) {
        if (eq(key, "x")) { rect.x = atoi(val.start); }
        if (eq(key, "y")) { rect.y = atoi(val.start); }
        if (eq(key, "w")) { rect.w = atoi(val.start); }
        if (eq(key, "h")) { rect.h = atoi(val.start); }
    }
    printf("rect: {%d, %d, %d, %d}\n", rect.x, rect.y, rect.w, rect.h);
    return 0;
}