#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

extern inline capy_buffer *capy_buffer_init(capy_arena *arena, size_t capacity);
extern inline int capy_buffer_wstring(capy_buffer *buffer, capy_string input);
extern inline int capy_buffer_wbytes(capy_buffer *buffer, size_t size, const char *bytes);
extern inline int capy_buffer_wcstr(capy_buffer *buffer, const char *cstr);
extern inline int capy_buffer_resize(capy_buffer *buffer, size_t size);

capy_buffer *capy_buffer_init(capy_arena *arena, size_t capacity)
{
    capy_buffer *buf = capy_arena_alloc(arena, sizeof(capy_buffer) + capacity, 8, false);

    if (buf != NULL)
    {
        buf->size = 0;
        buf->capacity = capacity;
        buf->arena = arena;
        buf->data = recast(char *, buf + 1);
    }

    return buf;
}

must_check int capy_buffer_wbytes(capy_buffer *buf, size_t size, const char *bytes)
{
    char *data = capy_vec_insert(buf->arena, buf->data, sizeof(char), &buf->capacity, &buf->size, buf->size, size, bytes);

    if (data == NULL)
    {
        return ENOMEM;
    }

    buf->data = data;

    return 0;
}

must_check int capy_buffer_wstring(capy_buffer *buf, capy_string input)
{
    return capy_buffer_wbytes(buf, input.size, input.data);
}

must_check int capy_buffer_wcstr(capy_buffer *buf, const char *cstr)
{
    size_t size = strlen(cstr);
    return capy_buffer_wbytes(buf, size, cstr);
}

must_check int capy_buffer_resize(capy_buffer *buf, size_t size)
{
    char *data = capy_vec_reserve(buf->arena, buf->data, sizeof(char), &buf->capacity, size);

    if (data == NULL)
    {
        return ENOMEM;
    }

    buf->data = data;
    buf->size = size;

    return 0;
}

int capy_buffer_format(capy_buffer *buf, size_t max, const char *fmt, ...)
{
    int n;

    if (max == 0)
    {
        va_list args;
        va_start(args, fmt);
        n = vsnprintf(NULL, 0, fmt, args);
        va_end(args);

        max = (size_t)(n) + 1;
    }

    size_t index = buf->size;

    char *data = capy_vec_insert(buf->arena, buf->data, sizeof(char), &buf->capacity, &buf->size, index, max, NULL);

    if (data == NULL)
    {
        return ENOMEM;
    }

    buf->data = data;

    va_list args;
    va_start(args, fmt);
    n = vsnprintf(buf->data + index, max + 1, fmt, args);
    va_end(args);

    if ((size_t)n < max)
    {
        buf->size = index + (size_t)(n);
    }

    return 0;
}

int capy_buffer_shl(capy_buffer *buf, size_t size)
{
    return capy_vec_delete(buf->data, sizeof(char), &buf->size, 0, size);
}
