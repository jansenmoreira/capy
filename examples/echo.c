#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static capy_err error_response(capy_http_response *response, capy_http_status status, const char *message)
{
    capy_err err;

    response->status = status;
    response->body->size = 0;

    if ((err = capy_buffer_format(response->body, 0, "%s\n", message)).code)
    {
        return err;
    }

    capy_strkvmmap_clear(response->headers);

    if ((err = capy_strkvmmap_set(response->headers, strl("Content-Type"), strl("text/plain; chartset=UTF-8"))).code)
    {
        return err;
    }

    return ok;
}

static capy_err explode_handler(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    capy_err err;

    if ((err = capy_buffer_format(response->body, 0, "%*s", 1024, " ")).code)
    {
        return err;
    }

    void *data = capy_arena_alloc(arena, MiB(1) - capy_arena_size(arena), 0, 0);

    if (data == NULL)
    {
        return capy_errno(ENOMEM);
    }

    response->status = 200;
    return ok;
}

static capy_err fail_handler(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return capy_errno(EINVAL);
}

static capy_err params_handler(unused capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    capy_err err;

    capy_strkvn *param = capy_strkvmmap_get(request->params, strl("id"));

    err = capy_buffer_format(response->body, 0, "%.*s -> %.*s\n",
                             (int)param->key.size, param->key.data,
                             (int)param->value.size, param->value.data);

    if (err.code)
    {
        return errwrap(err, "Failed to get URI params");
    }

    for (size_t i = 0; i < request->query->capacity; i++)
    {
        capy_strkvn *param = request->query->items + i;

        if (param->key.size == 0)
        {
            continue;
        }

        while (param != NULL)
        {
            if ((err = capy_buffer_format(response->body, 0, "%s: %s\n", param->key.data, param->value.data)).code)
            {
                return errwrap(err, "Failed to write query params");
            }

            param = param->next;
        }
    }

    response->status = 200;
    return ok;
}

static capy_err echo_handler(capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    capy_err err;

    capy_string uri = capy_uri_string(arena, request->uri);

    if ((err = capy_buffer_format(response->body, 0, "uri: %s\n", uri.data)).code)
    {
        return errwrap(err, "Failed to produce URI");
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
            if ((err = capy_buffer_format(response->body, 0, "%s: %s\n", header->key.data, header->value.data)).code)
            {
                return errwrap(err, "Failed to write header");
            }

            header = header->next;
        }
    }

    if ((err = capy_buffer_format(response->body, 0, "size: %lu\n", request->content_length)).code)
    {
        return errwrap(err, "Failed to write size");
    }

    capy_json_value value;

    int tabsize = 3;

    capy_strkvn *qtab = capy_strkvmmap_get(request->query, strl("tabsize"));

    if (qtab != NULL)
    {
        tabsize = atoi(qtab->value.data);
    }

    err = capy_json_deserialize(arena, &value, request->content.data);

    if (err.code == EINVAL)
    {
        return error_response(response, CAPY_HTTP_BAD_REQUEST, errwrap(err, "Failed to parse request body").msg);
    }
    else if (err.code)
    {
        return errwrap(err, "Failed to parse request body");
    }

    if ((err = capy_json_serialize(response->body, value, tabsize)).code)
    {
        return errwrap(err, "Failed to serialize JSON value to response");
    }

    if ((err = capy_buffer_wcstr(response->body, "\n")).code)
    {
        return errwrap(err, "Failed to write newline");
    }

    if ((err = capy_strkvmmap_set(response->headers, strl("Content-Type"), strl("application/json"))).code)
    {
        return errwrap(err, "Failed to set Content-Type header");
    }

    response->status = 200;
    return ok;
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
        {CAPY_HTTP_DELETE, strl("/explode/"), explode_handler},
    };

    options.routes = routes;
    options.routes_size = arrlen(routes);
    options.mem_connection_max = MiB(1);

    capy_http_serve(options);

    return 0;
}
