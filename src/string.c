#include <capy/capy.h>
#include <ctype.h>

capy_string capy_string_copy(capy_arena *arena, capy_string s)
{
    if (s.size == 0)
    {
        return (capy_string){.data = NULL};
    }

    char *buffer = capy_arena_make(char, arena, s.size + 1);
    memcpy(buffer, s.data, s.size);
    s.data = buffer;

    return s;
}

capy_string capy_string_tolower(capy_arena *arena, capy_string s)
{
    s = capy_string_copy(arena, s);

    char *data = (char *)(s.data);

    for (int i = 0; i < s.size; i++)
    {
        data[i] = tolower(s.data[i]);
    }

    return s;
}

capy_string capy_string_toupper(capy_arena *arena, capy_string s)
{
    s = capy_string_copy(arena, s);

    char *data = (char *)(s.data);

    for (int i = 0; i < s.size; i++)
    {
        data[i] = toupper(s.data[i]);
    }

    return s;
}
