#ifndef CAPY_BASE64_H
#define CAPY_BASE64_H

#include <capy/buffer.h>
#include <capy/error.h>
#include <capy/math.h>
#include <capy/std.h>
#include <capy/string.h>

size_t capy_base64_(char *output, const char *encoding, size_t n, const char *input, int padding);
size_t capy_base64url(char *output, size_t n, const char *input, int padding);
size_t capy_base64(char *output, size_t n, const char *input, int padding);

must_check capy_err capy_string_base64_(capy_arena *arena, capy_string *output, capy_string input, const char *encoding, int padding);
must_check capy_err capy_string_base64url(capy_arena *arena, capy_string *output, capy_string input, int padding);
must_check capy_err capy_string_base64(capy_arena *arena, capy_string *output, capy_string input, int padding);

must_check capy_err capy_buffer_wbase64_(capy_buffer *buffer, size_t n, const char *input, const char *encoding, int padding);
must_check capy_err capy_buffer_wbase64url(capy_buffer *buffer, size_t n, const char *input, int padding);
must_check capy_err capy_buffer_wbase64(capy_buffer *buffer, size_t n, const char *input, int padding);

#endif
