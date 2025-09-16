#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static capy_http_status failure_(unused capy_buffer *body, int err, const char *file, int line, const char *msg)
{
    capy_buffer_format_noalloc(body, "[%s:%d] => %s: %s\n", file, line, msg, strerror(err));
    return CAPY_HTTP_INTERNAL_SERVER_ERROR;
}

#define failure(body, err, msg) failure_((body), (err), __FILE__, __LINE__, (msg))

static capy_http_status fail_handler(unused capy_arena *arena, unused capy_http_request *request, unused capy_strkvmmap *headers, unused capy_buffer *body)
{
    return CAPY_HTTP_INTERNAL_SERVER_ERROR;
}

static capy_http_status params_handler(unused capy_arena *arena, capy_http_request *request, unused capy_strkvmmap *headers, capy_buffer *body)
{
    int err;

    capy_strkvn *param = capy_strkvmmap_get(request->params, strl("id"));

    if ((err = capy_buffer_format(body, 0, "%.*s -> %.*s\n",
                                  (int)param->key.size, param->key.data,
                                  (int)param->value.size, param->value.data)))
    {
        return failure(body, err, "failed to get URI params");
    }

    return CAPY_HTTP_OK;
}

static capy_http_status echo_handler(capy_arena *arena, capy_http_request *request, capy_strkvmmap *headers, capy_buffer *body)
{
    int err;

    capy_string uri = capy_uri_string(arena, request->uri);

    if ((err = capy_buffer_format(body, 0, "uri: %s\n", uri.data)))
    {
        return failure(body, err, "failed to produce URI");
    }

    for (size_t i = 0; i < request->headers->capacity; i++)
    {
        capy_strkvn *header = request->headers->items + i;

        if (header->key.size == 0)
        {
            continue;
        }

        while (header != NULL)
        {
            if ((err = capy_buffer_format(body, 0, "%s: %s\n", header->key.data, header->value.data)))
            {
                return failure(body, err, "failed to write header");
            }

            header = header->next;
        }
    }

    if ((err = capy_buffer_format(body, 0, "size: %lu\n", request->content_length)))
    {
        return failure(body, err, "failed to write size");
    }

    if ((err = capy_buffer_wbase64url(body, request->content_length, request->content, true)))
    {
        return failure(body, err, "failed to write base64 encoded body");
    }

    if ((err = capy_buffer_wcstr(body, "\n")))
    {
        return failure(body, err, "failed to newline");
    }

    if ((err = capy_strkvmmap_set(headers, strl("X-Foo"), strl("bar"))))
    {
        return failure(body, err, "failed to set header");
    }

    if ((err = capy_strkvmmap_add(headers, strl("X-Foo"), strl("baz"))))
    {
        return failure(body, err, "failed to set header");
    }

    if ((err = capy_strkvmmap_add(headers, strl("X-Bar"), strl("foo"))))
    {
        return failure(body, err, "failed to set header");
    }

    return CAPY_HTTP_OK;
}

int main(int argc, char *argv[])
{
    capy_log_init(CAPY_LOG_INFO);

    capy_http_server_options options = {
        .workers = 0,
    };

    options.host = "127.0.0.1";
    options.port = "8080";

    int opt;

    while ((opt = getopt(argc, argv, "vmw:c:a:p:")) != -1)
    {
        switch (opt)
        {
            case 'v':
                capy_stdout->mask |= CAPY_LOG_DEBUG;
                break;
            case 'm':
                capy_stdout->mask |= CAPY_LOG_MEM;
                break;
            case 'w':
                options.workers = (size_t)(strtoull(optarg, NULL, 10));
                break;
            case 'c':
                options.connections = (size_t)(strtoull(optarg, NULL, 10));
                break;
            case 'a':
                options.host = optarg;
                break;
            case 'p':
                options.port = optarg;
                break;
        }
    }

    capy_http_route routes[] = {
        {CAPY_HTTP_POST, strl("/"), echo_handler},
        {CAPY_HTTP_GET, strl("/^id/"), params_handler},
        {CAPY_HTTP_PUT, strl("/fail/"), fail_handler},
    };

    options.routes = routes;
    options.routes_size = arrlen(routes);
    options.mem_connection_max = KiB(512);

    capy_http_serve(options);

    return 0;
}
