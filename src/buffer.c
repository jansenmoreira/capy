#include <capy/macros.h>

// PUBLIC DEFINITIONS

capy_buffer *capy_buffer_init(capy_arena *arena, size_t capacity)
{
    char *addr = capy_arena_alloc(arena, sizeof(capy_buffer) + capacity, 8, false);

    if (addr == NULL)
    {
        return NULL;
    }

    capy_buffer *buf = Cast(capy_buffer *, addr);

    buf->size = 0;
    buf->capacity = capacity;
    buf->element_size = sizeof(char);
    buf->arena = arena;
    buf->data = addr + sizeof(capy_buffer);

    return buf;
}

capy_err capy_buffer_write_bytes(capy_buffer *buf, size_t size, const char *bytes)
{
    return capy_vec_insert(buf->arena, &buf->vec, buf->size, size, bytes);
}

capy_err capy_buffer_write_null(capy_buffer *buf)
{
    capy_err err = capy_buffer_write_bytes(buf, 1, "\0");

    if (err.code)
    {
        return err;
    }

    buf->size -= 1;
    return Ok;
}

capy_err capy_buffer_write_string(capy_buffer *buf, capy_string input)
{
    return capy_buffer_write_bytes(buf, input.size, input.data);
}

capy_err capy_buffer_write_cstr(capy_buffer *buf, const char *cstr)
{
    return capy_buffer_write_bytes(buf, strlen(cstr), cstr);
}

capy_err capy_buffer_write_fmt(capy_buffer *buf, size_t max, const char *fmt, ...)
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

    size_t size = buf->size;

    capy_err err = capy_buffer_write_bytes(buf, max, NULL);

    if (err.code)
    {
        return err;
    }

    va_list args;
    va_start(args, fmt);
    n = vsnprintf(buf->data + size, max + 1, fmt, args);
    va_end(args);

    if (n < 0)
    {
        return ErrStd(errno);
    }

    if (Cast(size_t, n) < max)
    {
        buf->size = size + Cast(size_t, n);
    }

    return Ok;
}

void capy_buffer_shl(capy_buffer *buf, size_t size)
{
    capy_vec_delete(&buf->vec, 0, size);
}
