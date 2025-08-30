#ifndef CAPY_STRING_H
#define CAPY_STRING_H

#include <capy/arena.h>
#include <capy/assert.h>
#include <capy/std.h>
#include <stddef.h>

typedef struct capy_string
{
    const char *data;
    size_t size;
} capy_string;

capy_string capy_string_copy(capy_arena *arena, capy_string s);
capy_string capy_string_tolower(capy_arena *arena, capy_string s);
capy_string capy_string_toupper(capy_arena *arena, capy_string s);
capy_string capy_string_join(capy_arena *arena, capy_string delimiter, int n, capy_string *s);
size_t capy_string_hex(capy_string input, int64_t *value);

static inline capy_string capy_string_slice(capy_string s, size_t begin, size_t end)
{
    capy_assert(begin <= end);     // GCOVR_EXCL_LINE
    capy_assert(begin <= s.size);  // GCOVR_EXCL_LINE
    capy_assert(end <= s.size);    // GCOVR_EXCL_LINE

    s.data += begin;
    s.size = end - begin;

    return s;
}

static inline int capy_string_eq(capy_string a, capy_string b)
{
    return (a.size == b.size) ? strncmp(a.data, b.data, b.size) == 0 : 0;
}

static inline int capy_string_sw(capy_string a, capy_string prefix)
{
    return (a.size >= prefix.size) ? strncmp(a.data, prefix.data, prefix.size) == 0 : 0;
}

static inline capy_string capy_string_bytes(const char *data, size_t size)
{
    return (capy_string){.data = data, .size = size};
}

static inline capy_string capy_string_shl(capy_string s, size_t size)
{
    return (capy_string){.data = s.data + size, .size = s.size - size};
}

static inline capy_string capy_string_shr(capy_string s, size_t size)
{
    return (capy_string){.data = s.data, .size = s.size - size};
}

static inline capy_string capy_string_cstr(const char *data)
{
    return (capy_string){.data = data, .size = strlen(data)};
}

static inline capy_string capy_string_trim(capy_string s)
{
    if (s.size == 0)
    {
        return s;
    }

    size_t i, j;

    for (i = 0; i < s.size && s.data[i] == ' '; i++)
    {
    }

    for (j = s.size; j > i && s.data[j - 1] == ' '; j--)
    {
    }

    return (capy_string){.data = s.data + i, .size = (size_t)(j - i)};
}

#define capy_string_literal(s) ((capy_string){.data = (s), .size = sizeof(s) - 1})

#endif
