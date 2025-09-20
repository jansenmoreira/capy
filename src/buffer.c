#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

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

must_check capy_err capy_buffer_wbytes(capy_buffer *buf, size_t size, const char *bytes)
{
    void *tmp = capy_vec_insert(buf->arena, buf->data, sizeof(char), &buf->capacity, &buf->size, buf->size, size, bytes);

    if (tmp == NULL)
    {
        return capy_errno(ENOMEM);
    }

    buf->data = cast(char *, tmp);

    return ok;
}

must_check capy_err capy_buffer_wnull(capy_buffer *buf)
{
    void *tmp = capy_vec_insert(buf->arena, buf->data, sizeof(char), &buf->capacity, &buf->size, buf->size, 1, "\0");

    if (tmp == NULL)
    {
        return capy_errno(ENOMEM);
    }

    buf->data = cast(char *, tmp);
    buf->size -= 1;

    return ok;
}

must_check capy_err capy_buffer_wstring(capy_buffer *buf, capy_string input)
{
    return capy_buffer_wbytes(buf, input.size, input.data);
}

must_check capy_err capy_buffer_wcstr(capy_buffer *buf, const char *cstr)
{
    return capy_buffer_wbytes(buf, strlen(cstr), cstr);
}

must_check capy_err capy_buffer_resize(capy_buffer *buf, size_t size)
{
    void *tmp = capy_vec_reserve(buf->arena, buf->data, sizeof(char), &buf->capacity, size);

    if (tmp == NULL)
    {
        return capy_errno(ENOMEM);
    }

    buf->data = cast(char *, tmp);
    buf->size = size;

    return ok;
}

capy_err capy_buffer_format_noalloc(capy_buffer *buf, const char *fmt, ...)
{
    int n;

    size_t max = buf->capacity - buf->size;

    va_list args;
    va_start(args, fmt);
    n = vsnprintf(buf->data + buf->size, max + 1, fmt, args);
    va_end(args);

    if (n < 0)
    {
        return capy_errno(errno);
    }

    if (cast(size_t, n) < max)
    {
        buf->size += cast(size_t, n);
    }
    else
    {
        buf->size += max;
    }

    return ok;
}

must_check capy_err capy_buffer_format(capy_buffer *buf, size_t max, const char *fmt, ...)
{
    int n;

    if (max == 0)
    {
        va_list args;
        va_start(args, fmt);
        n = vsnprintf(NULL, 0, fmt, args);
        va_end(args);

        if (n < 0)
        {
            return capy_errno(errno);
        }

        max = cast(size_t, n) + 1;
    }

    size_t index = buf->size;
    void *tmp = capy_vec_insert(buf->arena, buf->data, sizeof(char), &buf->capacity, &buf->size, index, max, NULL);

    if (tmp == NULL)
    {
        return capy_errno(ENOMEM);
    }

    buf->data = cast(char *, tmp);

    va_list args;
    va_start(args, fmt);
    n = vsnprintf(buf->data + index, max + 1, fmt, args);
    va_end(args);

    if (n < 0)
    {
        return capy_errno(errno);
    }

    if (cast(size_t, n) < max)
    {
        buf->size = index + cast(size_t, n);
    }

    return ok;
}

void capy_buffer_shl(capy_buffer *buf, size_t size)
{
    capy_vec_delete(buf->data, sizeof(char), &buf->size, 0, size);
}
