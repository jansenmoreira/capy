#include <capy/capy.h>
#include <capy/macros.h>

// STATIC

static char base64_std_enc[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char base64_url_enc[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// DEFINITIONS

size_t capy_base64(char *output, const char *encoding, size_t n, const char *input, int padding)
{
    size_t i, bytes = 0;

    uint32_t v;

    for (i = 0; (i + 2) < n; i += 3)
    {
        v = Cast(uint32_t, Cast(uint8_t, input[i])) << 16;
        v |= Cast(uint32_t, Cast(uint8_t, input[i + 1])) << 8;
        v |= Cast(uint32_t, Cast(uint8_t, input[i + 2]));

        output[bytes++] = encoding[(v >> 18) & 0x3F];
        output[bytes++] = encoding[(v >> 12) & 0x3F];
        output[bytes++] = encoding[(v >> 6) & 0x3F];
        output[bytes++] = encoding[(v) & 0x3F];
    }

    switch (n - i)
    {
        case 2:
        {
            v = Cast(uint32_t, Cast(uint8_t, input[i])) << 16;
            v |= Cast(uint32_t, Cast(uint8_t, input[i + 1])) << 8;

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
            v = Cast(uint32_t, Cast(uint8_t, input[i])) << 16;

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

size_t capy_base64url(char *output, size_t n, const char *input, int padding)
{
    return capy_base64(output, base64_url_enc, n, input, padding);
}

size_t capy_base64std(char *output, size_t n, const char *input, int padding)
{
    return capy_base64(output, base64_std_enc, n, input, padding);
}

capy_err capy_string_base64(capy_arena *arena, capy_string *output, capy_string input, const char *encoding, int padding)
{
    size_t n = align_to(input.size, 3) / 3 * 4;

    char *buffer = MakeNZ(arena, char, n + 1);

    if (buffer == NULL)
    {
        return ErrStd(ENOMEM);
    }

    n = capy_base64(buffer, encoding, input.size, input.data, padding);
    buffer[n] = 0;
    *output = capy_string_bytes(n, buffer);

    return Ok;
}

capy_err capy_string_base64url(capy_arena *arena, capy_string *output, capy_string input, int padding)
{
    return capy_string_base64(arena, output, input, base64_url_enc, padding);
}

capy_err capy_string_base64std(capy_arena *arena, capy_string *output, capy_string input, int padding)
{
    return capy_string_base64(arena, output, input, base64_std_enc, padding);
}

capy_err capy_buffer_write_base64(capy_buffer *buffer, size_t n, const char *input, const char *encoding, int padding)
{
    size_t s = align_to(n, 3) / 3 * 4;
    size_t index = buffer->size;

    capy_err err = capy_buffer_write_bytes(buffer, s + 1, NULL);

    if (!err.code)
    {
        n = capy_base64(buffer->data + index, encoding, n, input, padding);
        buffer->data[index + n] = 0;
        buffer->size = index + n;
    }

    return err;
}

capy_err capy_buffer_write_base64url(capy_buffer *buffer, size_t n, const char *input, int padding)
{
    return capy_buffer_write_base64(buffer, n, input, base64_url_enc, padding);
}

capy_err capy_buffer_write_base64std(capy_buffer *buffer, size_t n, const char *input, int padding)
{
    return capy_buffer_write_base64(buffer, n, input, base64_std_enc, padding);
}
