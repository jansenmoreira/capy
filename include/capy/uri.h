#ifndef CAPY_URI_H
#define CAPY_URI_H

#include <capy/arena.h>
#include <capy/string.h>

typedef struct capy_URI
{
    capy_string schema;
    capy_string authority;
    capy_string userinfo;
    capy_string host;
    capy_string port;
    capy_string path;
    capy_string query;
    capy_string fragment;
    capy_string ipv4;
    capy_string ipv6;
} capy_URI;

capy_URI *capy_URI_decode(capy_arena *arena, capy_string input);

#endif
