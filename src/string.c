#include <capy/capy.h>
#include <ctype.h>
#include <errno.h>

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
    size_t bytes = 0;
    int64_t tmp = 0;
    int64_t mult = 1;

    if (input.size && input.data[0] == '-')
    {
        bytes += 1;
        mult = -1;
        input = capy_string_shl(input, 1);
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
            case 'A':
                tmp = (tmp << 4) + 10;
                break;
            case 'B':
                tmp = (tmp << 4) + 11;
                break;
            case 'C':
                tmp = (tmp << 4) + 12;
                break;
            case 'D':
                tmp = (tmp << 4) + 13;
                break;
            case 'E':
                tmp = (tmp << 4) + 14;
                break;
            case 'F':
                tmp = (tmp << 4) + 15;
                break;
            case 'a':
                tmp = (tmp << 4) + 10;
                break;
            case 'b':
                tmp = (tmp << 4) + 11;
                break;
            case 'c':
                tmp = (tmp << 4) + 12;
                break;
            case 'd':
                tmp = (tmp << 4) + 13;
                break;
            case 'e':
                tmp = (tmp << 4) + 14;
                break;
            case 'f':
                tmp = (tmp << 4) + 15;
                break;
            default:
                *value = tmp * mult;
                return bytes;
        }

        bytes += 1;
        input = capy_string_shl(input, 1);
    }

    *value = tmp * mult;
    return bytes;
}

int capy_string_copy(capy_arena *arena, capy_string src, capy_string *dst)
{
    if (src.size == 0)
    {
        *dst = (capy_string){NULL};
        return 0;
    }

    char *buffer = capy_arena_make(char, arena, src.size + 1);

    if (buffer == NULL)
    {
        return ENOMEM;
    }

    memcpy(buffer, src.data, src.size);
    *dst = capy_string_bytes(buffer, src.size);

    return 0;
}

int capy_string_tolower(capy_arena *arena, capy_string src, capy_string *dst)
{
    int err = capy_string_copy(arena, src, dst);

    if (err)
    {
        return err;
    }

    char *data = (char *)(dst->data);

    for (size_t i = 0; i < src.size; i++)
    {
        data[i] = (char)(tolower(src.data[i]));
    }

    return 0;
}

int capy_string_toupper(capy_arena *arena, capy_string src, capy_string *dst)
{
    int err = capy_string_copy(arena, src, dst);

    if (err)
    {
        return err;
    }

    char *data = (char *)(dst->data);

    for (size_t i = 0; i < src.size; i++)
    {
        data[i] = (char)(toupper(src.data[i]));
    }

    return 0;
}

int capy_string_join(capy_arena *arena, capy_string delimiter, int n, capy_string *strs, capy_string *dst)
{
    if (n < 1)
    {
        *dst = (capy_string){NULL};
        return 0;
    }

    size_t size = 0;

    for (int i = 0; i < n; i++)
    {
        if (i != 0)
        {
            size += delimiter.size;
        }

        size += strs[i].size;
    }

    char *buffer = capy_arena_make(char, arena, size + 1);

    if (buffer == NULL)
    {
        return ENOMEM;
    }

    char *cursor = buffer;

    for (int i = 0; i < n; i++)
    {
        if (i != 0)
        {
            strncpy(cursor, delimiter.data, delimiter.size);
            cursor += delimiter.size;
        }

        strncpy(cursor, strs[i].data, strs[i].size);
        cursor += strs[i].size;
    }

    *dst = capy_string_bytes(buffer, size);
    return 0;
}
