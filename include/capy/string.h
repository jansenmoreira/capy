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

int capy_string_copy(capy_arena *arena, capy_string src, capy_string *dst);
int capy_string_tolower(capy_arena *arena, capy_string src, capy_string *dst);
int capy_string_toupper(capy_arena *arena, capy_string src, capy_string *dst);
int capy_string_join(capy_arena *arena, capy_string delimiter, int n, capy_string *strs, capy_string *dst);
size_t capy_string_hex(capy_string input, int64_t *value);

inline capy_string capy_string_slice(capy_string s, size_t begin, size_t end)
{
    capy_assert(begin <= end);
    capy_assert(begin <= s.size);
    capy_assert(end <= s.size);

    s.data += begin;
    s.size = end - begin;

    return s;
}

inline int capy_string_eq(capy_string a, capy_string b)
{
    return (a.size == b.size) ? strncmp(a.data, b.data, b.size) == 0 : 0;
}

inline int capy_string_sw(capy_string a, capy_string prefix)
{
    return (a.size >= prefix.size) ? strncmp(a.data, prefix.data, prefix.size) == 0 : 0;
}

inline capy_string capy_string_bytes(const char *data, size_t size)
{
    return (capy_string){.data = data, .size = size};
}

inline capy_string capy_string_shl(capy_string s, size_t size)
{
    capy_assert(size <= s.size);
    return (capy_string){.data = s.data + size, .size = s.size - size};
}

inline capy_string capy_string_shr(capy_string s, size_t size)
{
    capy_assert(size <= s.size);
    return (capy_string){.data = s.data, .size = s.size - size};
}

inline capy_string capy_string_cstr(const char *data)
{
    return (capy_string){.data = data, .size = strlen(data)};
}

static inline int capy_char_in(char c, const char *chars)
{
    if (chars == NULL)
    {
        return false;
    }

    for (const char *it = chars; it[0] != 0; it++)
    {
        if (c == it[0])
        {
            return true;
        }
    }

    return false;
}

inline int capy_char_is(char c, const char *chars)
{
    for (const char *it = chars; it != NULL && it[0] != 0; it++)
    {
        if (it[0] == c)
        {
            return true;
        }
    }

    return false;
}

inline capy_string capy_string_ltrim(capy_string s, const char *chars)
{
    size_t i;

    for (i = 0; i < s.size; i++)
    {
        if (!capy_char_is(s.data[i], chars))
        {
            break;
        }
    }

    return (capy_string){.data = s.data + i, .size = s.size - i};
}

inline capy_string capy_string_rtrim(capy_string s, const char *chars)
{
    size_t i;

    for (i = s.size; i > 0; i--)
    {
        if (!capy_char_is(s.data[i - 1], chars))
        {
            break;
        }
    }

    return (capy_string){.data = s.data, .size = i};
}

inline capy_string capy_string_trim(capy_string s, const char *chars)
{
    s = capy_string_ltrim(s, chars);
    return capy_string_rtrim(s, chars);
}

#define capy_string_literal(s) ((capy_string){.data = (s), .size = sizeof(s) - 1})

#endif
