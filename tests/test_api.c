#include <capy/test.h>
#include <unistd.h>

static char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static int base64(char *buffer, int n, const char *input)
{
    int i, j;

    uint32_t v;

    for (i = 0, j = 0; i < n; i += 3, j += 4)
    {
        v = ((uint32_t)(input[i + 0]) << 16) |
            ((uint32_t)(input[i + 1]) << 8) |
            ((uint32_t)(input[i + 2]));

        buffer[j + 0] = base64_table[(v >> 18) & 0x3F];
        buffer[j + 1] = base64_table[(v >> 12) & 0x3F];
        buffer[j + 2] = base64_table[(v >> 6) & 0x3F];
        buffer[j + 3] = base64_table[(v >> 0) & 0x3F];
    }

    int rem = i - n;

    if (rem == 0)
    {
        return j;
    }

    v = ((uint32_t)(input[i++]) << 16);

    if (rem == 2)
    {
        v |= ((uint32_t)(input[i++]) << 0);
    }

    buffer[j++] = base64_table[(v >> 18) & 0x3F];
    buffer[j++] = base64_table[(v >> 12) & 0x3F];

    if (rem == 2)
    {
        buffer[j++] = base64_table[(v >> 6) & 0x3F];
    }

    return j;
}

static int http_handler(capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    size_t size = 8 * 1024;

    char *buffer = capy_arena_make(char, arena, size);

    int s = 0;

    s = sprintf(buffer, "uri: %s\n", capy_uri_string(arena, request->uri).data);

    for (size_t i = 0; i < capy_smap_capacity(request->headers); i++)
    {
        capy_http_field header = request->headers[i];

        if (header.name.size > 0)
        {
            s += sprintf(buffer + s, "%s: %s\n", header.name.data, header.value.data);
        }
    }

    s += sprintf(buffer + s, "size: %lu\n", request->content_length);
    s += base64(buffer + s, (int)request->content_length, request->content);

    buffer[s++] = '\n';
    buffer[s++] = '\n';

    response->status = CAPY_HTTP_OK;
    response->content = capy_string_bytes(buffer, (size_t)s);

    return 0;
}

int main(int argc, const char **argv)
{
    setvbuf(stdout, NULL, _IOLBF, 0);

    if (argc < 2)
    {
        return EINVAL;
    }

    capy_http_serve("0.0.0.0", "8080", strtoull(argv[1], NULL, 10), http_handler);

    return 0;
}
