#include <capy/capy.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

extern inline char capy_char_uppercase(char c);
extern inline char capy_char_lowercase(char c);
extern inline capy_string capy_string_slice(capy_string s, size_t begin, size_t end);
extern inline int capy_string_eq(capy_string a, capy_string b);
extern inline int capy_string_sw(capy_string a, capy_string prefix);
extern inline capy_string capy_string_bytes(const char *data, size_t size);
extern inline capy_string capy_string_shl(capy_string s, size_t size);
extern inline capy_string capy_string_shr(capy_string s, size_t size);
extern inline capy_string capy_string_cstr(const char *data);
extern inline capy_string capy_string_trim(capy_string s, const char *chars);
extern inline capy_string capy_string_ltrim(capy_string s, const char *chars);
extern inline capy_string capy_string_rtrim(capy_string s, const char *chars);
extern inline int capy_char_is(char c, const char *chars);

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

capy_string capy_string_copy(capy_arena *arena, capy_string input)
{
    if (input.size == 0)
    {
        return (capy_string){.size = 0};
    }

    char *buffer = capy_arena_make(char, arena, input.size + 1);
    memcpy(buffer, input.data, input.size);
    return capy_string_bytes(buffer, input.size);
}

void capy_string_iuppercase(capy_string s)
{
    char *data = (char *)(s.data);

    for (size_t i = 0; i < s.size; i++)
    {
        data[i] = capy_char_uppercase(data[i]);
    }
}

void capy_string_ilowercase(capy_string s)
{
    char *data = (char *)(s.data);

    for (size_t i = 0; i < s.size; i++)
    {
        data[i] = capy_char_lowercase(data[i]);
    }
}

capy_string capy_string_lowercase(capy_arena *arena, capy_string src)
{
    capy_string dst = capy_string_copy(arena, src);
    capy_string_ilowercase(dst);
    return dst;
}

capy_string capy_string_uppercase(capy_arena *arena, capy_string src)
{
    capy_string dst = capy_string_copy(arena, src);
    capy_string_iuppercase(dst);
    return dst;
}

capy_string capy_string_join(capy_arena *arena, const char *delimiter, int n, capy_string *list)
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
        return (capy_string){.size = 0};
    }

    char *buffer = capy_arena_make(char, arena, size + 1);
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

    return capy_string_bytes(buffer, size);
}
