#include <capy/capy.h>
#include <errno.h>

// STATIC

char base64_std_enc[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char base64_url_enc[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// EXTERN INLINES

extern inline size_t capy_base64url(char *output, size_t n, const char *input, int padding);
extern inline size_t capy_base64(char *output, size_t n, const char *input, int padding);
extern inline int capy_string_base64url(capy_arena *arena, capy_string *output, capy_string input, int padding);
extern inline int capy_string_base64(capy_arena *arena, capy_string *output, capy_string input, int padding);
extern inline int capy_buffer_wbase64url(capy_buffer *buffer, size_t n, const char *input, int padding);
extern inline int capy_buffer_wbase64(capy_buffer *buffer, size_t n, const char *input, int padding);

// DEFINITIONS

size_t capy_base64_(char *output, const char *encoding, size_t n, const char *input, int padding)
{
    size_t i, bytes = 0;

    uint32_t v;

    for (i = 0; (i + 2) < n; i += 3)
    {
        v = (uint32_t)((uint8_t)(input[i + 0])) << 16;
        v |= (uint32_t)((uint8_t)(input[i + 1])) << 8;
        v |= (uint32_t)((uint8_t)(input[i + 2]));

        output[bytes++] = encoding[(v >> 18) & 0x3F];
        output[bytes++] = encoding[(v >> 12) & 0x3F];
        output[bytes++] = encoding[(v >> 6) & 0x3F];
        output[bytes++] = encoding[(v >> 0) & 0x3F];
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

int capy_string_base64_(capy_arena *arena, capy_string *output, capy_string input, const char *encoding, int padding)
{
    size_t n = align_to(input.size, 3) / 3 * 4;

    char *buffer = capy_arena_umake(arena, char, n + 1);

    if (buffer == NULL)
    {
        return ENOMEM;
    }

    n = capy_base64_(buffer, encoding, input.size, input.data, padding);
    buffer[n] = 0;
    *output = capy_string_bytes(n, buffer);

    return 0;
}

int capy_buffer_wbase64_(capy_buffer *buffer, size_t n, const char *input, const char *encoding, int padding)
{
    size_t s = align_to(n, 3) / 3 * 4;
    size_t index = buffer->size;

    int err = capy_buffer_resize(buffer, index + s + 1);

    if (err)
    {
        return err;
    }

    n = capy_base64_(buffer->data + index, encoding, n, input, padding);
    buffer->data[index + n] = 0;
    buffer->size = index + n;

    return 0;
}
