#ifndef CAPY_URI_H
#define CAPY_URI_H

#include <capy/arena.h>
#include <capy/std.h>
#include <capy/string.h>

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
} capy_uri;

int capy_uri_valid(capy_uri uri);
capy_string capy_uri_string(capy_arena *arena, capy_uri uri);
capy_uri capy_uri_parse(capy_string input);
void capy_uri_parse_authority(capy_uri *uri);
capy_uri capy_uri_resolve_reference(capy_arena *arena, capy_uri base, capy_uri relative);
capy_string capy_uri_normalize(capy_arena *arena, capy_string input, int lowercase);
capy_string capy_uri_path_remove_dot_segments(capy_arena *arena, capy_string path);

#endif
