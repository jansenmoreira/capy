#include <capy/capy.h>
#include <ctype.h>

size_t capy_string_hex(capy_string input, int64_t *value)
{
    size_t bytes = 0;
    int64_t tmp = 0;
    int64_t mult = 1;

    if (input.size && input.data[0] == '-')
    {
        mult = -1;
        capy_string_shl(input, 1);
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

capy_string capy_string_copy(capy_arena *arena, capy_string s)
{
    if (s.size == 0)
    {
        return (capy_string){NULL};
    }

    char *buffer = capy_arena_make(char, arena, s.size + 1);
    memcpy(buffer, s.data, s.size);
    s.data = buffer;

    return s;
}

capy_string capy_string_tolower(capy_arena *arena, capy_string s)
{
    char *data = capy_arena_make(char, arena, s.size + 1);

    for (size_t i = 0; i < s.size; i++)
    {
        data[i] = (char)(tolower(s.data[i]));
    }

    return capy_string_bytes(data, s.size);
}

capy_string capy_string_toupper(capy_arena *arena, capy_string s)
{
    char *data = capy_arena_make(char, arena, s.size + 1);

    for (size_t i = 0; i < s.size; i++)
    {
        data[i] = (char)(toupper(s.data[i]));
    }

    return capy_string_bytes(data, s.size);
}

capy_string capy_string_join(capy_arena *arena, capy_string delimiter, int n, capy_string *s)
{
    if (n < 1)
    {
        return (capy_string){NULL};
    }

    size_t size = 0;

    for (int i = 0; i < n; i++)
    {
        if (i == 0)
        {
            size += delimiter.size;
        }

        size += s[i].size;
    }

    char *buffer = capy_arena_make(char, arena, size + 1);
    char *cursor = buffer;

    for (int i = 0; i < n; i++)
    {
        if (i != 0)
        {
            strncpy(cursor, delimiter.data, delimiter.size);
            cursor += delimiter.size;
        }

        strncpy(cursor, s[i].data, s[i].size);
        cursor += s[i].size;
    }

    return capy_string_bytes(buffer, size);
}
