#ifndef CAPY_URI_H
#define CAPY_URI_H

#include <capy/arena.h>
#include <capy/std.h>
#include <capy/string.h>

#define CAPY_URI_SCHEME 0x1
#define CAPY_URI_AUTHORITY 0x2
#define CAPY_URI_QUERY 0x4
#define CAPY_URI_FRAGMENT 0x8

typedef struct capy_uri
{
    capy_string scheme;
    capy_string authority;
    capy_string userinfo;
    capy_string host;
    capy_string port;
    capy_string path;
    capy_string query;
    capy_string fragment;
    int flags;
} capy_uri;

int capy_uri_valid(capy_uri uri);
capy_string capy_uri_string(capy_arena *arena, capy_uri uri);
capy_uri capy_uri_parse(capy_string input);
capy_uri capy_uri_parse_authority(capy_uri uri);
capy_uri capy_uri_resolve_reference(capy_arena *arena, capy_uri base, capy_uri relative);
int capy_uri_normalize(capy_arena *arena, capy_string *output, capy_string input, int lowercase);
capy_string capy_uri_path_removedots(capy_arena *arena, capy_string path);

#endif
