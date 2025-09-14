#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int fail_handler(capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    (void)!arena;
    (void)!request;
    (void)!response;

    return 1;
}

static int params_handler(capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    (void)!arena;

    capy_strkvn *param = capy_strkvmmap_get(request->params, strl("id"));

    (void)!capy_buffer_format(response->content, 0, "%.*s -> %.*s\n",
                              (int)param->key.size, param->key.data,
                              (int)param->value.size, param->value.data);

    response->status = CAPY_HTTP_OK;

    return 0;
}

static int echo_handler(capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    capy_string uri = capy_uri_string(arena, request->uri);

    (void)!capy_buffer_format(response->content, 0, "uri: %s\n", uri.data);

    for (size_t i = 0; i < request->headers->capacity; i++)
    {
        capy_strkvn *header = request->headers->items + i;

        if (header->key.size == 0)
        {
            continue;
        }

        while (header != NULL)
        {
            (void)!capy_buffer_format(response->content, 0, "%s: %s\n", header->key.data, header->value.data);
            header = header->next;
        }
    }

    (void)!capy_buffer_format(response->content, 0, "size: %lu\n", request->content_length);
    (void)!capy_buffer_wbase64url(response->content, request->content_length, request->content, true);
    (void)!capy_buffer_wcstr(response->content, "\n");

    (void)!capy_strkvmmap_set(response->headers, strl("X-Foo"), strl("bar"));
    (void)!capy_strkvmmap_add(response->headers, strl("X-Foo"), strl("baz"));
    (void)!capy_strkvmmap_add(response->headers, strl("X-Bar"), strl("foo"));

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

    capy_arena *arena = capy_arena_init(0, KiB(8));

    capy_http_route routes[] = {
        {CAPY_HTTP_POST, strl("/"), echo_handler},
        {CAPY_HTTP_GET, strl("/^id/"), params_handler},
        {CAPY_HTTP_PUT, strl("/fail/"), fail_handler},
    };

    capy_http_router *router = capy_http_router_init(arena, arrlen(routes), routes);

    capy_http_serve(host, port, router, options);

    return 0;
}
