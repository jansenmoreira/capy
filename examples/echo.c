#include <capy/capy.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int http_handler(capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    capy_string uri = capy_uri_string(arena, request->uri);

    capy_strbuf_format(response->content, 0, "uri: %s\n", uri.data);

    for (size_t i = 0; i < request->headers->capacity; i++)
    {
        capy_http_field header = request->headers->data[i];

        if (header.name.size > 0)
        {
            capy_strbuf_format(response->content, 0, "%s: %s\n", header.name.data, header.value.data);
        }
    }

    capy_strbuf_format(response->content, 0, "size: %lu\n", request->content_length);
    capy_strbuf_base64_url(response->content, request->content_length, request->content, true);
    capy_strbuf_write_cstr(response->content, "\n");

    capy_http_fields_set(response->headers, capy_string_literal("X-Foo"), capy_string_literal("bar"));
    capy_http_fields_add(response->headers, capy_string_literal("X-Foo"), capy_string_literal("baz"));
    capy_http_fields_add(response->headers, capy_string_literal("X-Bar"), capy_string_literal("foo"));

    response->status = CAPY_HTTP_OK;

    return 0;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IOLBF, 0);

    capy_http_server_options options = {
        .trace = 0,
        .workers = 0,
    };

    const char *host = "127.0.0.1";
    const char *port = "8080";

    int opt;

    while ((opt = getopt(argc, argv, "vw:a:p:")) != -1)
    {
        switch (opt)
        {
            case 'v':
                options.trace = true;
                break;
            case 'w':
                options.workers = (size_t)(strtoull(optarg, NULL, 10));
                break;
            case 'a':
                host = optarg;
                break;
            case 'p':
                port = optarg;
                break;
        }
    }

    capy_http_serve(host, port, http_handler, &options);

    return 0;
}
