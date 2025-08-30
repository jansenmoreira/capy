#include <unistd.h>

#include "test.h"

static int http_handler(capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    char buffer[32 * 1024] = {0};

    int s = sprintf(buffer, "uri: %s\n", capy_uri_string(arena, request->uri).data);

    for (size_t i = 0; i < capy_smap_capacity(request->headers); i++)
    {
        capy_http_field header = request->headers[i];

        if (header.name.size > 0)
        {
            s += sprintf(buffer + s, "%s: %s\n", header.name.data, header.value.data);
        }
    }

    s += sprintf(buffer + s, "size: %lu\n", request->content_length);

    // printf("%s\n", buffer);

    response->status = CAPY_HTTP_OK;
    response->content = capy_string_copy(arena, capy_string_bytes(buffer, s));

    return 0;
}

int main(void)
{
    capy_http_serve("0.0.0.0", "8080", 8, http_handler);

    return 0;
}
