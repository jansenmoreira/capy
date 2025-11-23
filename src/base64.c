#include <capy/macros.h>

// INTERNAL VARIABLES

static struct capy_base64enc base64_stdenc = {
    .encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
    .padding = true,
};

static struct capy_base64enc base64_urlenc = {
    .encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
    .padding = false,
};

// PUBLIC DEFINITIONS

size_t capy_base64enc_encode(struct capy_base64enc encoder, char *output, size_t n, const char *input)
{
    size_t i, bytes = 0;

    uint32_t v;

    for (i = 0; (i + 2) < n; i += 3)
    {
        v = U32(U8(input[i])) << 16;
        v |= U32(U8(input[i + 1])) << 8;
        v |= U32(U8(input[i + 2]));

        output[bytes++] = encoder.encoding[(v >> 18) & 0x3F];
        output[bytes++] = encoder.encoding[(v >> 12) & 0x3F];
        output[bytes++] = encoder.encoding[(v >> 6) & 0x3F];
        output[bytes++] = encoder.encoding[(v) & 0x3F];
    }

    switch (n - i)
    {
        case 2:
        {
            v = U32(U8(input[i])) << 16;
            v |= U32(U8(input[i + 1])) << 8;

            output[bytes++] = encoder.encoding[(v >> 18) & 0x3F];
            output[bytes++] = encoder.encoding[(v >> 12) & 0x3F];
            output[bytes++] = encoder.encoding[(v >> 6) & 0x3F];

            if (encoder.padding)
            {
                output[bytes++] = '=';
            }
        }
        break;

        case 1:
        {
            v = U32(U8(input[i])) << 16;

            output[bytes++] = encoder.encoding[(v >> 18) & 0x3F];
            output[bytes++] = encoder.encoding[(v >> 12) & 0x3F];

            if (encoder.padding)
            {
                output[bytes++] = '=';
                output[bytes++] = '=';
            }
        }
        break;
    }

    return bytes;
}

capy_err capy_base64enc_string(struct capy_base64enc encoder, capy_arena *arena, capy_string *output, size_t n, const char *input)
{
    size_t size = (capy_align_to(n, 3) / 3) * 4;

    char *buffer = MakeNZ(arena, char, size + 1);

    if (buffer == NULL)
    {
        return ErrStd(ENOMEM);
    }

    size = capy_base64enc_encode(encoder, buffer, n, input);
    buffer[size] = 0;
    *output = capy_string_bytes(size, buffer);

    return Ok;
}

capy_err capy_base64enc_buffer(struct capy_base64enc encoder, capy_buffer *buffer, size_t n, const char *input)
{
    size_t size = capy_align_to(n, 3) / 3 * 4;
    size_t index = buffer->size;

    capy_err err = capy_buffer_write_bytes(buffer, size + 1, NULL);

    if (err.code)
    {
        return err;
    }

    size = capy_base64enc_encode(encoder, buffer->data + index, n, input);
    buffer->data[index + size] = 0;
    buffer->size = index + size;

    return Ok;
}

size_t capy_base64(char *output, size_t n, const char *input)
{
    return capy_base64enc_encode(base64_stdenc, output, n, input);
}

capy_err capy_base64_string(capy_arena *arena, capy_string *output, size_t n, const char *input)
{
    return capy_base64enc_string(base64_stdenc, arena, output, n, input);
}

capy_err capy_base64_buffer(capy_buffer *buffer, size_t n, const char *input)
{
    return capy_base64enc_buffer(base64_stdenc, buffer, n, input);
}

size_t capy_base64_url(char *output, size_t n, const char *input)
{
    return capy_base64enc_encode(base64_urlenc, output, n, input);
}

capy_err capy_base64_url_string(capy_arena *arena, capy_string *output, size_t n, const char *input)
{
    return capy_base64enc_string(base64_urlenc, arena, output, n, input);
}

capy_err capy_base64_url_buffer(capy_buffer *buffer, size_t n, const char *input)
{
    return capy_base64enc_buffer(base64_urlenc, buffer, n, input);
}
