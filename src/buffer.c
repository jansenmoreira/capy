#include <capy/capy.h>
#include <capy/macros.h>

capy_buffer *capy_buffer_init(capy_arena *arena, size_t capacity)
{
    capy_buffer *buf = capy_arena_alloc(arena, sizeof(capy_buffer) + capacity, 8, false);

    if (buf != NULL)
    {
        buf->size = 0;
        buf->capacity = capacity;
        buf->arena = arena;
        buf->data = ReinterpretCast(char *, buf + 1);
    }

    return buf;
}

MustCheck capy_err capy_buffer_write_bytes(capy_buffer *buf, size_t size, const char *bytes)
{
    void *tmp = capy_vec_insert(buf->arena, buf->data, sizeof(char), &buf->capacity, &buf->size, buf->size, size, bytes);

    if (tmp == NULL)
    {
        return ErrStd(ENOMEM);
    }

    buf->data = Cast(char *, tmp);

    return Ok;
}

MustCheck capy_err capy_buffer_write_null(capy_buffer *buf)
{
    void *tmp = capy_vec_insert(buf->arena, buf->data, sizeof(char), &buf->capacity, &buf->size, buf->size, 1, "\0");

    if (tmp == NULL)
    {
        return ErrStd(ENOMEM);
    }

    buf->data = Cast(char *, tmp);
    buf->size -= 1;

    return Ok;
}

MustCheck capy_err capy_buffer_write_string(capy_buffer *buf, capy_string input)
{
    return capy_buffer_write_bytes(buf, input.size, input.data);
}

MustCheck capy_err capy_buffer_write_cstr(capy_buffer *buf, const char *cstr)
{
    return capy_buffer_write_bytes(buf, strlen(cstr), cstr);
}

MustCheck capy_err capy_buffer_resize(capy_buffer *buf, size_t size)
{
    void *tmp = capy_vec_insert(buf->arena, buf->data, sizeof(char), &buf->capacity, &buf->size, buf->size, size, NULL);

    if (tmp == NULL)
    {
        return ErrStd(ENOMEM);
    }

    buf->data = Cast(char *, tmp);
    buf->size = size;

    return Ok;
}

MustCheck capy_err capy_buffer_write_fmt(capy_buffer *buf, size_t max, const char *fmt, ...)
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
            return ErrStd(errno);
        }

        max = Cast(size_t, n) + 1;
    }

    size_t index = buf->size;
    void *tmp = capy_vec_insert(buf->arena, buf->data, sizeof(char), &buf->capacity, &buf->size, index, max, NULL);

    if (tmp == NULL)
    {
        return ErrStd(ENOMEM);
    }

    buf->data = Cast(char *, tmp);

    va_list args;
    va_start(args, fmt);
    n = vsnprintf(buf->data + index, max + 1, fmt, args);
    va_end(args);

    if (n < 0)
    {
        return ErrStd(errno);
    }

    if (Cast(size_t, n) < max)
    {
        buf->size = index + Cast(size_t, n);
    }

    return Ok;
}

void capy_buffer_shl(capy_buffer *buf, size_t size)
{
    buf->size = capy_vec_delete(buf->data, sizeof(char), buf->size, 0, size);
}
