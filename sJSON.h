/*
author: shway
date: 2026/3/10
description: A header file implements a json lib
*/

#ifndef __SJSON_H__
#define __SJSON_H__

#include <stddef.h>
#include <stdbool.h>

typedef struct _sJSON_ {
    char* data;
    char* cur;
    char* end;

    int depth;
    char* error;
}sJSON;

typedef struct _sJSON_Value_ {
    int type;
    char* start;
    char* end;
    int depth;
}sJSON_Value;

typedef enum _sJSON_Type_e_ {
    SJSON_ERROR,
    SJSON_END,
    SJSON_ARRAY,
    SJSON_OBJECT,
    SJSON_NUMBER,
    SJSON_STRING,
    SJSON_BOOL,
    SJSON_NULL
}sJSON_Type_e;



#endif

#ifdef SJSON_IMPLEMENT
sJSON sJSON_Reader(char* data, size_t len) {
    return (sJSON) {
        .data = data,
            .cur = data,
            .end = data + len,
            .depth = 0,
            .error = NULL
    };
}

static bool sJSON_IsNumberCont(char c) {
    return (c >= '0' && c <= '9') ||
        c == 'e' || c == 'E' || c == '.' || c == '-' || c == '+';
}

static bool sJSON_IsString(char* cur, char* end, char* expect) {
    while (*expect) {
        if (cur == end || *cur != *expect)
            return false;
        expect++, cur++;
    }
    return true;
}

sJSON_Value sJSON_Parse(sJSON* r) {
    sJSON_Value ret;
top:
    if (r->error) {
        return (sJSON_Value) {
                .type = SJSON_ERROR,
                .start = r->cur,
                .end = r->cur
        };
    }

    if (r->cur == r->end) {
        r->error = "unexpectedeof";
        goto top;
    }
    ret.start = r->cur;
    switch (*r->cur) {
    case ' ': case '\n': case '\r': case '\t': case ':': case ',':
        r->cur++;
        goto top;
    case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        ret.type = SJSON_NUMBER;
        r->cur++;
        while (r->cur != r->end && sJSON_IsNumberCont(*r->cur))
            r->cur++;
        return ret;
    case '"':
        ret.type = SJSON_STRING;
        ret.start = ++r->cur;
        for (;;) {
            if (r->cur == r->end) {
                r->error = "unclosed string";
                goto top;
            }
            if (*r->cur == '"') break;
            if (*r->cur == '\\') r->cur++; 
            if (r->cur != r->end) r->cur++;
        }
        ret.end = r->cur++;
        return ret;
    case '{':case '[':
        ret.type = (*r->cur == '{') ? SJSON_OBJECT : SJSON_OBJECT;
        ret.depth = ++r->depth;
        r->cur++;
        break;
    case '}': case ']':
        ret.type = SJSON_END;
        if (--r->depth < 0) {
            r->error = (*r->cur = '}') ? "stray '}'" : "stray ']'";
            goto top;
        }
        r->cur++;
        break;
    case 'n': case 't': case 'f':
        ret.type = (*r->cur == 'n') ? SJSON_NULL : SJSON_BOOL;
        if (sJSON_IsString(r->cur, r->end, "null")) {
            r->cur += 4;
            break;
        }
        if (sJSON_IsString(r->cur, r->end, "true")) {
            r->cur += 4;
            break;
        }
        if (sJSON_IsString(r->cur, r->end, "false")) {
            r->cur += 5;
            break;
        }

    default:
        r->error = "unkown token";
        goto top;
    }

    ret.end = r->cur;
    return ret;
}

static void sJSON_DiscardUntil(sJSON* r, int depth) {
    sJSON_Value val;
    val.type = SJSON_NULL;
    while (r->depth != depth && val.type != SJSON_ERROR)
        val = sJSON_Parse(r);
}

bool sJSON_IterArray(sJSON* r, sJSON_Value arr, sJSON_Value* val) {
    sJSON_DiscardUntil(r, arr.depth);
    *val = sJSON_Parse(r);
    if (val->type == SJSON_ERROR || val->type == SJSON_END)
        return false;
    return true;
}

bool sJSON_IterObject(sJSON* r, sJSON_Value obj, sJSON_Value* key, sJSON_Value* val) {
    sJSON_DiscardUntil(r, obj.depth);
    *key = sJSON_Parse(r);
    if (key->type == SJSON_ERROR || key->type == SJSON_END) return false;
    *val = sJSON_Parse(r);
    if (val->type == SJSON_ERROR) {
        r->error = "unexpected object end";
        return false;
    }

    if (val->type == SJSON_ERROR) return false;
    return true;
}

void sJSON_location(sJSON* r, int* line, int* col) {
    int ln = 1, cl = 1;
    for (char* p = r->data; p != r->cur; p++) {
        if (*p == '\n') {
            ln++;
            cl = 0;
        }
        cl++;
    }
    *line = ln;
    *col = cl;
}

#endif