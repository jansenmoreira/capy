#ifndef CAPY_STRING_H
#define CAPY_STRING_H

#include <capy/arena.h>
#include <capy/assert.h>
#include <capy/std.h>
#include <capy/vec.h>

typedef struct capy_string
{
    const char *data;
    size_t size;
} capy_string;

int capy_char_is(char c, const char *chars);
must_check int capy_string_copy(capy_arena *arena, capy_string *output, capy_string input);
must_check int capy_string_lower(capy_arena *arena, capy_string *output, capy_string input);
must_check int capy_string_upper(capy_arena *arena, capy_string *output, capy_string input);
must_check int capy_string_join(capy_arena *arena, capy_string *output, const char *delimiter, int n, capy_string *list);
size_t capy_string_hex(capy_string input, int64_t *value);
capy_string capy_string_prefix(capy_string a, capy_string b);
capy_string capy_string_ltrim(capy_string s, const char *chars);
capy_string capy_string_rtrim(capy_string s, const char *chars);

inline char capy_char_uppercase(char c)
{
    return ('a' <= c && c <= 'z') ? c & (char)(0xDF) : c;
}

inline char capy_char_lowercase(char c)
{
    return ('A' <= c && c <= 'Z') ? c | (char)(0x20) : c;
}

inline int capy_string_eq(capy_string a, capy_string b)
{
    return (a.size == b.size) ? memcmp(a.data, b.data, b.size) == 0 : 0;
}

inline capy_string capy_string_bytes(size_t size, const char *data)
{
    return (capy_string){.data = data, .size = size};
}

inline capy_string capy_string_cstr(const char *data)
{
    return capy_string_bytes(strlen(data), data);
}

inline capy_string capy_string_slice(capy_string s, size_t begin, size_t end)
{
    capy_assert(begin <= end);
    capy_assert(begin <= s.size);
    capy_assert(end <= s.size);
    return (capy_string){.data = s.data + begin, .size = end - begin};
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

inline capy_string capy_string_trim(capy_string s, const char *chars)
{
    return capy_string_rtrim(capy_string_ltrim(s, chars), chars);
}

#endif
