#include <capy/capy.h>
#include <ctype.h>

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
