#include <capy/capy.h>
#include <capy/macros.h>
#include <ctype.h>
#include <errno.h>

#define capy_string_empty ((capy_string){.data = NULL, .size = 0})

// EXTERN INLINES

extern inline char capy_char_uppercase(char c);
extern inline char capy_char_lowercase(char c);
extern inline capy_string capy_string_slice(capy_string s, size_t begin, size_t end);
extern inline int capy_string_eq(capy_string a, capy_string b);
extern inline capy_string capy_string_bytes(size_t size, const char *data);
extern inline capy_string capy_string_cstr(const char *data);
extern inline capy_string capy_string_shl(capy_string s, size_t size);
extern inline capy_string capy_string_shr(capy_string s, size_t size);
extern inline capy_string capy_string_trim(capy_string s, const char *chars);
extern inline int capy_char_is(char c, const char *chars);

// DEFINITIONS

size_t capy_string_hex(capy_string input, int64_t *value)
{
    if (input.size == 0)
    {
        return 0;
    }

    size_t bytes = 0;
    int64_t tmp = 0;

    if (input.data[0] == '-')
    {
        bytes = capy_string_hex(capy_string_shl(input, 1), value);

        if (bytes == 0)
        {
            return 0;
        }

        *value *= -1;
        return bytes + 1;
    }

    while (input.size)
    {
        switch (input.data[0])
        {
            case '0':
                tmp = (tmp << 4) + 0;
                break;
            case '1':
                tmp = (tmp << 4) + 1;
                break;
            case '2':
                tmp = (tmp << 4) + 2;
                break;
            case '3':
                tmp = (tmp << 4) + 3;
                break;
            case '4':
                tmp = (tmp << 4) + 4;
                break;
            case '5':
                tmp = (tmp << 4) + 5;
                break;
            case '6':
                tmp = (tmp << 4) + 6;
                break;
            case '7':
                tmp = (tmp << 4) + 7;
                break;
            case '8':
                tmp = (tmp << 4) + 8;
                break;
            case '9':
                tmp = (tmp << 4) + 9;
                break;
            case 'a':
            case 'A':
                tmp = (tmp << 4) + 10;
                break;
            case 'b':
            case 'B':
                tmp = (tmp << 4) + 11;
                break;
            case 'c':
            case 'C':
                tmp = (tmp << 4) + 12;
                break;
            case 'd':
            case 'D':
                tmp = (tmp << 4) + 13;
                break;
            case 'e':
            case 'E':
                tmp = (tmp << 4) + 14;
                break;
            case 'f':
            case 'F':
                tmp = (tmp << 4) + 15;
                break;

            default:
            {
                if (bytes > 0)
                {
                    *value = tmp;
                }

                return bytes;
            }
        }

        bytes += 1;
        input = capy_string_shl(input, 1);
    }

    *value = tmp;
    return bytes;
}

must_check capy_err capy_string_copy(capy_arena *arena, capy_string *output, capy_string input)
{
    capy_assert(output != NULL);

    if (input.size == 0)
    {
        *output = capy_string_empty;
        return ok;
    }

    char *buffer = make(arena, char, input.size + 1);

    if (buffer == NULL)
    {
        return capy_errno(ENOMEM);
    }

    memcpy(buffer, input.data, input.size);
    *output = capy_string_bytes(input.size, buffer);

    return ok;
}

must_check capy_err capy_string_lower(capy_arena *arena, capy_string *output, capy_string input)
{
    if (input.size == 0)
    {
        *output = capy_string_empty;
        return ok;
    }

    char *data = make(arena, char, input.size);

    if (data == NULL)
    {
        return capy_errno(ENOMEM);
    }

    for (size_t i = 0; i < input.size; i++)
    {
        data[i] = capy_char_lowercase(input.data[i]);
    }

    *output = capy_string_bytes(input.size, data);

    return ok;
}

must_check capy_err capy_string_upper(capy_arena *arena, capy_string *output, capy_string input)
{
    if (input.size == 0)
    {
        *output = capy_string_empty;
        return ok;
    }

    char *data = make(arena, char, input.size);

    if (data == NULL)
    {
        return capy_errno(ENOMEM);
    }

    for (size_t i = 0; i < input.size; i++)
    {
        data[i] = capy_char_uppercase(input.data[i]);
    }

    *output = capy_string_bytes(input.size, data);

    return ok;
}

must_check capy_err capy_string_join(capy_arena *arena, capy_string *output, const char *delimiter, int n, capy_string *list)
{
    size_t size = 0;
    size_t delimiter_size = strlen(delimiter);

    for (int i = 0; i < n; i++)
    {
        if (i != 0)
        {
            size += delimiter_size;
        }

        size += list[i].size;
    }

    if (size == 0)
    {
        *output = capy_string_empty;
        return ok;
    }

    char *buffer = make(arena, char, size + 1);

    if (buffer == NULL)
    {
        return capy_errno(ENOMEM);
    }

    char *cursor = buffer;

    for (int i = 0; i < n; i++)
    {
        if (i != 0)
        {
            memcpy(cursor, delimiter, delimiter_size);
            cursor += delimiter_size;
        }

        memcpy(cursor, list[i].data, list[i].size);
        cursor += list[i].size;
    }

    *output = capy_string_bytes(size, buffer);

    return ok;
}

capy_string capy_string_prefix(capy_string a, capy_string b)
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

int capy_char_is(char c, const char *chars)
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

capy_string capy_string_ltrim(capy_string s, const char *chars)
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

capy_string capy_string_rtrim(capy_string s, const char *chars)
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

char capy_char_uppercase(char c)
{
    return ('a' <= c && c <= 'z') ? c & cast(char, 0xDF) : c;
}

char capy_char_lowercase(char c)
{
    return ('A' <= c && c <= 'Z') ? c | cast(char, 0x20) : c;
}

int capy_string_eq(capy_string a, capy_string b)
{
    return (a.size == b.size) ? memcmp(a.data, b.data, b.size) == 0 : 0;
}

capy_string capy_string_bytes(size_t size, const char *data)
{
    return (capy_string){.data = data, .size = size};
}

capy_string capy_string_cstr(const char *data)
{
    return capy_string_bytes(strlen(data), data);
}

capy_string capy_string_slice(capy_string s, size_t begin, size_t end)
{
    capy_assert(begin <= end);
    capy_assert(begin <= s.size);
    capy_assert(end <= s.size);
    return (capy_string){.data = s.data + begin, .size = end - begin};
}

capy_string capy_string_shl(capy_string s, size_t size)
{
    capy_assert(size <= s.size);
    return (capy_string){.data = s.data + size, .size = s.size - size};
}

capy_string capy_string_shr(capy_string s, size_t size)
{
    capy_assert(size <= s.size);
    return (capy_string){.data = s.data, .size = s.size - size};
}

capy_string capy_string_trim(capy_string s, const char *chars)
{
    return capy_string_rtrim(capy_string_ltrim(s, chars), chars);
}
