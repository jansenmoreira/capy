#ifndef CAPY_BUFFER_H
#define CAPY_BUFFER_H

#include <capy/assert.h>
#include <capy/std.h>
#include <capy/string.h>
#include <capy/vec.h>

// TYPES

typedef struct capy_buffer
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    char *data;
} capy_buffer;

// DECLARATIONS

must_check capy_buffer *capy_buffer_init(capy_arena *arena, size_t capacity);
must_check int capy_buffer_wstring(capy_buffer *buffer, capy_string input);
must_check int capy_buffer_wbytes(capy_buffer *buffer, size_t size, const char *bytes);
must_check int capy_buffer_wcstr(capy_buffer *buffer, const char *cstr);
must_check int capy_buffer_resize(capy_buffer *buffer, size_t size);
int capy_buffer_format_noalloc(capy_buffer *buf, const char *fmt, ...);
must_check int capy_buffer_format(capy_buffer *buffer, size_t max, const char *fmt, ...);
int capy_buffer_shl(capy_buffer *buffer, size_t size);

#endif
