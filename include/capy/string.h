#ifndef CAPY_STRING_H
#define CAPY_STRING_H

#include <capy/arena.h>
#include <capy/assert.h>
#include <capy/std.h>
#include <capy/vec.h>

typedef struct capy_string
{
    const char *data;
    size_t size;
} capy_string;

bool capy_char_isdigit(char c);
bool capy_char_ishexdigit(char c);
bool capy_char_isalpha(char c);
bool capy_char_isalphanumeric(char c);

char capy_char_uppercase(char c);
char capy_char_lowercase(char c);
int capy_char_is(char c, const char *chars);

must_check capy_err capy_string_copy(capy_arena *arena, capy_string *output, capy_string input);
must_check capy_err capy_string_lower(capy_arena *arena, capy_string *output, capy_string input);
must_check capy_err capy_string_upper(capy_arena *arena, capy_string *output, capy_string input);

must_check capy_err capy_string_join(capy_arena *arena, capy_string *output, const char *delimiter, int n, capy_string *list);

size_t capy_string_hex(capy_string input, uint64_t *value);
capy_string capy_string_prefix(capy_string a, capy_string b);

capy_string capy_string_ltrim(capy_string s, const char *chars);
capy_string capy_string_rtrim(capy_string s, const char *chars);
capy_string capy_string_trim(capy_string s, const char *chars);

int capy_string_eq(capy_string a, capy_string b);

capy_string capy_string_bytes(size_t size, const char *data);
capy_string capy_string_cstr(const char *data);

capy_string capy_string_slice(capy_string s, size_t begin, size_t end);
capy_string capy_string_shl(capy_string s, size_t size);
capy_string capy_string_shr(capy_string s, size_t size);

uint32_t capy_unicode_utf16(uint16_t high, uint16_t low);
size_t capy_unicode_utf8encode(char *buffer, uint32_t code);

#endif
