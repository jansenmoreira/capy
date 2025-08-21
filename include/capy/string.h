#ifndef CAPY_STRING_H
#define CAPY_STRING_H

#include <capy/arena.h>
#include <capy/hash.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct capy_string
{
    const char *data;
    size_t size;
} capy_string;

capy_string capy_string_copy(capy_arena *arena, capy_string s);
capy_string capy_string_tolower(capy_arena *arena, capy_string s);
capy_string capy_string_toupper(capy_arena *arena, capy_string s);

static inline capy_string capy_string_slice(capy_string s, size_t begin, size_t end)
{
    if (begin > end || s.data == NULL || begin >= s.size || end > s.size)
    {
        return (capy_string){.data = NULL};
    }

    s.data += begin;
    s.size = end - begin;

    return s;
}

static inline int capy_string_equals(capy_string a, capy_string b)
{
    return (a.size == b.size) ? strncmp(a.data, b.data, b.size) == 0 : 0;
}

#define capy_string_bytes(s, n) \
    ((capy_string){.data = (s), .size = n})

#define capy_string_lit(s) \
    capy_string_bytes((s), sizeof(s) - 1)

#define capy_string_cstr(s) \
    capy_string_bytes((s), strlen(s))

#define capy_string_resize(s, n) \
    capy_string_bytes((s).data, n)

#define capy_string_shrinkl(s, n) \
    ((capy_string){.data = (s).data + (n), .size = (s).size - (n)})

#define capy_string_shrinkr(s, n) \
    capy_string_resize((s), (s).size - (n));

#endif
