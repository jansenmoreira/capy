#ifndef CAPY_STRING_H
#define CAPY_STRING_H

#include <capy/arena.h>
#include <capy/assert.h>
#include <capy/std.h>
#include <capy/vec.h>
#include <stddef.h>

typedef struct capy_string
{
    const char *data;
    size_t size;
} capy_string;

capy_string capy_string_copy(capy_arena *arena, capy_string input);
capy_string capy_string_lower(capy_arena *arena, capy_string src);
capy_string capy_string_upper(capy_arena *arena, capy_string src);
capy_string capy_string_join(capy_arena *arena, const char *delimiter, int n, capy_string *list);
size_t capy_string_hex(capy_string input, int64_t *value);

inline char capy_char_uppercase(char c)
{
    return ('a' <= c && c <= 'z') ? c & (char)(0xDF) : c;
}

inline char capy_char_lowercase(char c)
{
    return ('A' <= c && c <= 'Z') ? c | (char)(0x20) : c;
}

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
    return (a.size == b.size) ? memcmp(a.data, b.data, b.size) == 0 : 0;
}

inline int capy_string_sw(capy_string a, capy_string prefix)
{
    return (a.size >= prefix.size) ? memcmp(a.data, prefix.data, prefix.size) == 0 : 0;
}

inline capy_string capy_string_bytes(size_t size, const char *data)
{
    return (capy_string){.data = data, .size = size};
}

inline capy_string capy_string_shl(capy_string s, size_t size)
{
    capy_assert(size <= s.size);
    return (capy_string){.data = s.data + size, .size = s.size - size};
}

inline capy_string capy_string_prefix(capy_string a, capy_string b)
{
    size_t i;

    if (a.size < b.size)
    {
        for (i = 0; i < a.size && a.data[i] == b.data[i]; i++)
        {
        }
    }
    else
    {
        for (i = 0; i < b.size && a.data[i] == b.data[i]; i++)
        {
        }
    }

    return capy_string_slice(a, 0, i);
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

typedef struct capy_buffer
{
    size_t size;
    size_t capacity;
    size_t _;
    capy_arena *arena;
    char *data;
} capy_buffer;

int capy_buffer_format(capy_buffer *buffer, size_t max, const char *fmt, ...);

inline capy_buffer *capy_buffer_init(capy_arena *arena, size_t capacity)
{
    return (capy_buffer *)capy_vec_init(arena, sizeof(char), capacity);
}

inline void capy_buffer_write(capy_buffer *buffer, capy_string input)
{
    capy_vec_insert((capy_vec *)buffer, buffer->size, input.size, input.data);
}

inline void capy_buffer_write_bytes(capy_buffer *buffer, size_t size, const char *bytes)
{
    capy_vec_insert((capy_vec *)buffer, buffer->size, size, bytes);
}

inline void capy_buffer_write_cstr(capy_buffer *buffer, const char *cstr)
{
    size_t size = strlen(cstr);
    capy_vec_insert((capy_vec *)buffer, buffer->size, size, cstr);
}

inline void capy_buffer_resize(capy_buffer *buffer, size_t size)
{
    capy_vec_resize((capy_vec *)buffer, size);
}

inline void capy_buffer_shl(capy_buffer *buffer, size_t size)
{
    capy_vec_delete((capy_vec *)buffer, 0, size);
}

#endif
