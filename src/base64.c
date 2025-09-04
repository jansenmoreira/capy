#include <capy/capy.h>

char base64_std_enc[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char base64_url_enc[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

extern inline size_t capy_base64_url(char *output, size_t n, const char *input, int padding);
extern inline size_t capy_base64(char *output, size_t n, const char *input, int padding);
extern inline capy_string capy_string_base64_url(capy_arena *arena, capy_string input, int padding);
extern inline capy_string capy_string_base64(capy_arena *arena, capy_string input, int padding);
extern inline void capy_strbuf_base64_url(capy_strbuf *strbuf, size_t n, const char *input, int padding);
extern inline void capy_strbuf_base64(capy_strbuf *strbuf, size_t n, const char *input, int padding);

size_t capy_base64_(char *output, const char *encoding, size_t n, const char *input, int padding)
{
    size_t i, bytes = 0;

    uint32_t v;

    for (i = 0; (i + 2) < n; i += 3)
    {
        v = (uint32_t)((uint8_t)(input[i])) << 16;
        v |= (uint32_t)((uint8_t)(input[i + 1])) << 8;
        v |= (uint32_t)((uint8_t)(input[i + 2]));

        output[bytes++] = encoding[(v >> 18) & 0x3F];
        output[bytes++] = encoding[(v >> 12) & 0x3F];
        output[bytes++] = encoding[(v >> 6) & 0x3F];
        output[bytes++] = encoding[v & 0x3F];
    }

    switch (n - i)
    {
        case 2:
        {
            v = (uint32_t)((uint8_t)(input[i])) << 16;
            v |= (uint32_t)((uint8_t)(input[i + 1])) << 8;

            output[bytes++] = encoding[(v >> 18) & 0x3F];
            output[bytes++] = encoding[(v >> 12) & 0x3F];
            output[bytes++] = encoding[(v >> 6) & 0x3F];

            if (padding)
            {
                output[bytes++] = '=';
            }
        }
        break;

        case 1:
        {
            v = (uint32_t)((uint8_t)(input[i])) << 16;

            output[bytes++] = encoding[(v >> 18) & 0x3F];
            output[bytes++] = encoding[(v >> 12) & 0x3F];

            if (padding)
            {
                output[bytes++] = '=';
                output[bytes++] = '=';
            }
        }
        break;
    }

    return bytes;
}

capy_string capy_string_base64_(capy_arena *arena, capy_string input, const char *encoding, int padding)
{
    size_t n = align_to(input.size, 3) / 3 * 4;

    char *buffer = capy_arena_umake(char, arena, n + 1);

    n = capy_base64_(buffer, encoding, input.size, input.data, padding);

    buffer[n] = 0;

    return capy_string_bytes(n, buffer);
}

void capy_strbuf_base64_(capy_strbuf *strbuf, size_t n, const char *input, const char *encoding, int padding)
{
    size_t s = align_to(n, 3) / 3 * 4;
    size_t index = strbuf->size;

    capy_strbuf_resize(strbuf, index + s + 1);

    n = capy_base64_(strbuf->data + index, encoding, n, input, padding);
    strbuf->data[index + n] = 0;

    capy_strbuf_resize(strbuf, index + n);
}
