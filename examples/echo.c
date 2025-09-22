#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static capy_err error_response(capy_httpresp *response, capy_httpstatus status, const char *message)
{
    capy_err err;

    response->status = status;
    response->body->size = 0;

    if ((err = capy_buffer_write_fmt(response->body, 0, "%s\n", message)).code)
    {
        return err;
    }

    capy_strkvnmap_clear(response->headers);

    if ((err = capy_strkvnmap_set(response->headers, Str("Content-Type"), Str("text/plain; chartset=UTF-8"))).code)
    {
        return err;
    }

    return Ok;
}

static capy_err explode_handler(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    capy_err err;

    if ((err = capy_buffer_write_fmt(response->body, 0, "%*s", 1024, " ")).code)
    {
        return err;
    }

    void *data = capy_arena_alloc(arena, MiB(1) - capy_arena_size(arena), 0, 0);

    if (data == NULL)
    {
        return ErrStd(ENOMEM);
    }

    response->status = 200;
    return Ok;
}

static capy_err fail_handler(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return ErrStd(EINVAL);
}

static capy_err params_handler(Unused capy_arena *arena, capy_httpreq *request, capy_httpresp *response)
{
    capy_err err;

    capy_strkvn *param = capy_strkvnmap_get(request->params, Str("id"));

    err = capy_buffer_write_fmt(response->body, 0, "%.*s -> %.*s\n",
                                (int)param->key.size, param->key.data,
                                (int)param->value.size, param->value.data);

    if (err.code)
    {
        return ErrWrap(err, "Failed to get URI params");
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
            if ((err = capy_buffer_write_fmt(response->body, 0, "%s: %s\n", param->key.data, param->value.data)).code)
            {
                return ErrWrap(err, "Failed to write query params");
            }

            param = param->next;
        }
    }

    response->status = 200;
    return Ok;
}

static capy_err echo_handler(capy_arena *arena, capy_httpreq *request, capy_httpresp *response)
{
    capy_err err;

    capy_string uri = capy_uri_string(arena, request->uri);

    if ((err = capy_buffer_write_fmt(response->body, 0, "uri: %s\n", uri.data)).code)
    {
        return ErrWrap(err, "Failed to produce URI");
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
            if ((err = capy_buffer_write_fmt(response->body, 0, "%s: %s\n", header->key.data, header->value.data)).code)
            {
                return ErrWrap(err, "Failed to write header");
            }

            header = header->next;
        }
    }

    if ((err = capy_buffer_write_fmt(response->body, 0, "size: %lu\n", request->content_length)).code)
    {
        return ErrWrap(err, "Failed to write size");
    }

    capy_jsonval value;

    int tabsize = 3;

    capy_strkvn *qtab = capy_strkvnmap_get(request->query, Str("tabsize"));

    if (qtab != NULL)
    {
        tabsize = atoi(qtab->value.data);
    }

    err = capy_json_deserialize(arena, &value, request->content.data);

    if (err.code == EINVAL)
    {
        return error_response(response, CAPY_HTTP_BAD_REQUEST, ErrWrap(err, "Failed to parse request body").msg);
    }
    else if (err.code)
    {
        return ErrWrap(err, "Failed to parse request body");
    }

    if ((err = capy_json_serialize(response->body, value, tabsize)).code)
    {
        return ErrWrap(err, "Failed to serialize JSON value to response");
    }

    if ((err = capy_buffer_write_cstr(response->body, "\n")).code)
    {
        return ErrWrap(err, "Failed to write newline");
    }

    if ((err = capy_strkvnmap_set(response->headers, Str("Content-Type"), Str("application/json"))).code)
    {
        return ErrWrap(err, "Failed to set Content-Type header");
    }

    response->status = 200;
    return Ok;
}

int main(int argc, char *argv[])
{
    capy_logger_init(stderr);
    capy_logger_time_format("%T");

    capy_httpserveropt options = {
        .workers = 0,
    };

    options.host = "127.0.0.1";
    options.port = "8080";

    int opt;

    while ((opt = getopt(argc, argv, "vmsw:c:a:p:")) != -1)
    {
        switch (opt)
        {
            case 'v':
                capy_logger_add_level(CAPY_LOG_DEBUG);
                break;
            case 'm':
                capy_logger_add_level(CAPY_LOG_MEM);
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
            case 's':
                options.protocol = CAPY_HTTPS;
                options.certificate_chain = "extra/certificates/server_chain.pem";
                options.certificate_key = "extra/certificates/server_key.pem";
                break;
        }
    }

    capy_httproute routes[] = {
        {CAPY_HTTP_POST, Str("/"), echo_handler},
        {CAPY_HTTP_GET, Str("/^id/"), params_handler},
        {CAPY_HTTP_PUT, Str("/fail/"), fail_handler},
        {CAPY_HTTP_DELETE, Str("/explode/"), explode_handler},
    };

    options.routes = routes;
    options.routes_size = ArrLen(routes);
    options.mem_connection_max = MiB(1);

    capy_err err = capy_http_serve(options);

    if (err.code)
    {
        LogErr("Server closed: %s", err.msg);
    }

    return 0;
}
