#ifndef CAPY_BASE64_H
#define CAPY_BASE64_H

#include <capy/buffer.h>
#include <capy/math.h>
#include <capy/std.h>
#include <capy/string.h>

// DECLARATIONS

extern char base64_std_enc[65];
extern char base64_url_enc[65];

size_t capy_base64_(char *output, const char *encoding, size_t n, const char *input, int padding);
inline size_t capy_base64url(char *output, size_t n, const char *input, int padding);
inline size_t capy_base64(char *output, size_t n, const char *input, int padding);

must_check int capy_string_base64_(capy_arena *arena, capy_string *output, capy_string input, const char *encoding, int padding);
must_check inline int capy_string_base64url(capy_arena *arena, capy_string *output, capy_string input, int padding);
must_check inline int capy_string_base64(capy_arena *arena, capy_string *output, capy_string input, int padding);

must_check int capy_buffer_wbase64_(capy_buffer *buffer, size_t n, const char *input, const char *encoding, int padding);
must_check inline int capy_buffer_wbase64url(capy_buffer *buffer, size_t n, const char *input, int padding);
must_check inline int capy_buffer_wbase64(capy_buffer *buffer, size_t n, const char *input, int padding);

// INLINE DEFINITIONS

inline size_t capy_base64url(char *output, size_t n, const char *input, int padding)
{
    return capy_base64_(output, base64_url_enc, n, input, padding);
}

inline size_t capy_base64(char *output, size_t n, const char *input, int padding)
{
    return capy_base64_(output, base64_std_enc, n, input, padding);
}

must_check inline int capy_string_base64url(capy_arena *arena, capy_string *output, capy_string input, int padding)
{
    return capy_string_base64_(arena, output, input, base64_url_enc, padding);
}

must_check inline int capy_string_base64(capy_arena *arena, capy_string *output, capy_string input, int padding)
{
    return capy_string_base64_(arena, output, input, base64_std_enc, padding);
}

must_check inline int capy_buffer_wbase64url(capy_buffer *buffer, size_t n, const char *input, int padding)
{
    return capy_buffer_wbase64_(buffer, n, input, base64_url_enc, padding);
}

must_check inline int capy_buffer_wbase64(capy_buffer *buffer, size_t n, const char *input, int padding)
{
    return capy_buffer_wbase64_(buffer, n, input, base64_std_enc, padding);
}

#endif
