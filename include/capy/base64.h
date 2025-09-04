#ifndef CAPY_BASE64_H
#define CAPY_BASE64_H

#include <capy/math.h>
#include <capy/std.h>
#include <capy/string.h>

extern char base64_std_enc[65];
extern char base64_url_enc[65];

size_t capy_base64_(char *output, const char *encoding, size_t n, const char *input, int padding);
capy_string capy_string_base64_(capy_arena *arena, capy_string input, const char *encoding, int padding);
void capy_strbuf_base64_(capy_strbuf *strbuf, size_t n, const char *input, const char *encoding, int padding);

inline size_t capy_base64_url(char *output, size_t n, const char *input, int padding)
{
    return capy_base64_(output, base64_url_enc, n, input, padding);
}

inline size_t capy_base64(char *output, size_t n, const char *input, int padding)
{
    return capy_base64_(output, base64_std_enc, n, input, padding);
}

inline capy_string capy_string_base64_url(capy_arena *arena, capy_string input, int padding)
{
    return capy_string_base64_(arena, input, base64_url_enc, padding);
}

inline capy_string capy_string_base64(capy_arena *arena, capy_string input, int padding)
{
    return capy_string_base64_(arena, input, base64_std_enc, padding);
}

inline void capy_strbuf_base64_url(capy_strbuf *strbuf, size_t n, const char *input, int padding)
{
    capy_strbuf_base64_(strbuf, n, input, base64_url_enc, padding);
}

inline void capy_strbuf_base64(capy_strbuf *strbuf, size_t n, const char *input, int padding)
{
    capy_strbuf_base64_(strbuf, n, input, base64_std_enc, padding);
}

#endif
