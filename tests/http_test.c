#include <capy/test.h>

static int test_http(void)
{
    capy_arena *arena = capy_arena_init(GiB(1));

    capy_http_request *request = capy_arena_make(arena, capy_http_request, 1);

    capy_strkvmmap *fields;
    capy_strkvn *field;

    expect_s_eq(capy_http_parse_method(str("GET")), CAPY_HTTP_GET);
    expect_s_eq(capy_http_parse_method(str("_ET")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("G_T")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("GE_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(str("PUT")), CAPY_HTTP_PUT);
    expect_s_eq(capy_http_parse_method(str("_UT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("P_T")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("PU_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(str("HEAD")), CAPY_HTTP_HEAD);
    expect_s_eq(capy_http_parse_method(str("_EAD")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("H_AD")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("HE_D")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("HEA_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(str("POST")), CAPY_HTTP_POST);
    expect_s_eq(capy_http_parse_method(str("_OST")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("P_ST")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("PO_T")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("POS_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(str("TRACE")), CAPY_HTTP_TRACE);
    expect_s_eq(capy_http_parse_method(str("_RACE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("T_ACE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("TR_CE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("TRA_E")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("TRAC_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(str("PATCH")), CAPY_HTTP_PATCH);
    expect_s_eq(capy_http_parse_method(str("_ATCH")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("P_TCH")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("PA_CH")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("PAT_H")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("PATC_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(str("DELETE")), CAPY_HTTP_DELETE);
    expect_s_eq(capy_http_parse_method(str("_ELETE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("D_LETE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("DE_ETE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("DEL_TE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("DELE_E")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("DELET_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(str("CONNECT")), CAPY_HTTP_CONNECT);
    expect_s_eq(capy_http_parse_method(str("_ONNECT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("C_NNECT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("CO_NECT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("CON_ECT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("CONN_CT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("CONNE_T")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("CONNEC_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(str("OPTIONS")), CAPY_HTTP_OPTIONS);
    expect_s_eq(capy_http_parse_method(str("_PTIONS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("O_TIONS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("OP_IONS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("OPT_ONS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("OPTI_NS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("OPTIO_S")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("OPTION_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(str("AA")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("AAAAAAAA")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_version(str("HTTP/0.9")), CAPY_HTTP_09);
    expect_s_eq(capy_http_parse_version(str("HTTP/1.0")), CAPY_HTTP_10);
    expect_s_eq(capy_http_parse_version(str("HTTP/1.1")), CAPY_HTTP_11);
    expect_s_eq(capy_http_parse_version(str("HTTP/2.0")), CAPY_HTTP_20);
    expect_s_eq(capy_http_parse_version(str("HTTP/3.0")), CAPY_HTTP_30);

    expect_s_eq(capy_http_parse_version(str("HTTP/0.8")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTTP/1.2")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTTP/2.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTTP/3.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTTP/4.0")), CAPY_HTTP_VERSION_UNK);

    expect_s_eq(capy_http_parse_version(str("_TTP/1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("H_TP/1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HT_P/1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTT_/1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTTP_1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTTP/1_1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTTP/1.1 ")), CAPY_HTTP_VERSION_UNK);

    expect_s_eq(capy_http_parse_reqline(arena, request, str("CONNECT localhost:8080 HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("OPTIONS * HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("OPTIONS /index.html HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /index.html?id=1 HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET http://localhost:8080/index.html HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET HTTP://LOCALHOST:80/../%4D%30/abc/./../test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET ../%4D%20/abc/./../test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET / HTTP/1.1")), 0);

    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET  / HTTP/1.1")), EINVAL);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /  HTTP/1.1")), EINVAL);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET / HTTP/1.1 ")), EINVAL);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("get / HTTP/1.1")), EINVAL);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /%gg/ HTTP/1.1")), EINVAL);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET / http/1.1")), EINVAL);

    fields = capy_strkvmmap_init(arena, 16);

    expect_s_eq(capy_http_parse_field(fields, str("\x01: localhost:8080")), EINVAL);
    expect_s_eq(capy_http_parse_field(fields, str(": localhost:8080")), EINVAL);
    expect_s_eq(capy_http_parse_field(fields, str("Host : localhost:8080")), EINVAL);
    expect_s_eq(capy_http_parse_field(fields, str("Host")), EINVAL);
    expect_s_eq(capy_http_parse_field(fields, str("Host: \x01")), EINVAL);
    expect_s_eq(capy_http_parse_field(fields, str("transfer-Encoding:  deflate ; a=1 ")), 0);
    expect_s_eq(capy_http_parse_field(fields, str("Transfer-encoding: gzip\t; b=2 ")), 0);
    expect_s_eq(capy_http_parse_field(fields, str("transfer-encoding: chunked")), 0);

    field = capy_strkvmmap_get(fields, str("Transfer-Encoding"));
    expect_p_ne(field, NULL);
    expect_str_eq(field->key, str("Transfer-Encoding"));
    expect_str_eq(field->value, str("deflate ; a=1"));
    expect_str_eq(field->next->value, str("gzip\t; b=2"));
    expect_str_eq(field->next->next->value, str("chunked"));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:80")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Transfer-Encoding: chunked")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), 0);
    expect_u_eq(request->content_length, 0);
    expect_s_eq(request->chunked, 1);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET http://127.0.0.1/test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:80")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), 0);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:80")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Content-Length: 100")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), 0);
    expect_u_eq(request->content_length, 100);
    expect_s_eq(request->chunked, 0);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), 0);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:80")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Transfer-Encoding: gzip")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:80")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Transfer-Encoding: gzip")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Transfer-Encoding: chunked")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:80")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Transfer-Encoding: chunked")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Content-Length: 100")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:80")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Content-Length: 100")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Content-Length: 15")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    request = capy_arena_make(arena, capy_http_request, 1);
    request->headers = capy_strkvmmap_init(arena, 16);
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:80")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Content-Length: af")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:aa")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: foo@localhost:8080")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    expect_s_eq(capy_http_parse_reqline(arena, request, str("GET /test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: localhost:80")), 0);
    expect_s_eq(capy_http_parse_field(request->headers, str("Host: example:80")), 0);
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_strkvmmap_clear(fields);

    expect_s_eq(capy_strkvmmap_add(fields, str("X-Foo"), str("bar")), 0);
    expect_s_eq(capy_strkvmmap_add(fields, str("X-Foo"), str("baz")), 0);
    expect_s_eq(capy_strkvmmap_add(fields, str("X-Foo"), str("buz")), 0);
    expect_s_eq(capy_strkvmmap_set(fields, str("X-Bar"), str("foo")), 0);

    capy_http_response response = {
        .headers = fields,
        .content = capy_buffer_init(arena, 1024),
        .status = 200,
    };

    expect_s_eq(capy_buffer_wcstr(response.content, "foobar"), 0);

    capy_buffer *buffer = capy_buffer_init(arena, 1024);
    expect_s_eq(capy_http_write_headers(buffer, &response), 0);

    char expected_headers[] =
        "HTTP/1.1 200\r\n"
        "Content-Length: 6\r\n"
        "X-Bar: foo\r\n"
        "X-Foo: bar\r\n"
        "X-Foo: baz\r\n"
        "X-Foo: buz\r\n"
        "\r\n";

    expect_s_eq(memcmp(buffer->data, expected_headers, buffer->size), 0);

    capy_http_router *router = NULL;
    capy_http_route *route = NULL;

    router = capy_http_route_add(arena, router, str("GET /foo/bar/baz"), (capy_http_handler *)(1));
    router = capy_http_route_add(arena, router, str("POST /foo/baz"), (capy_http_handler *)(2));
    router = capy_http_route_add(arena, router, str("POST /foo/baz/bar"), (capy_http_handler *)(3));
    router = capy_http_route_add(arena, router, str("POST /foo/bar"), (capy_http_handler *)(4));
    router = capy_http_route_add(arena, router, str("GET /foo/bar"), (capy_http_handler *)(5));
    router = capy_http_route_add(arena, router, str("GET /foo/bar"), (capy_http_handler *)(6));
    router = capy_http_route_add(arena, router, str("POST /bar/foo"), (capy_http_handler *)(7));
    router = capy_http_route_add(arena, router, str("POST foo/bar"), (capy_http_handler *)(8));
    router = capy_http_route_add(arena, router, str("POST foo/^id/baz"), (capy_http_handler *)(9));

    route = capy_http_route_get(router, CAPY_HTTP_GET, str("foo/bar/baz/"));
    expect_p_ne(route, NULL);
    expect_u_eq((size_t)route->handler, (size_t)(1));

    route = capy_http_route_get(router, CAPY_HTTP_POST, str("foo/baz/"));
    expect_p_ne(route, NULL);
    expect_u_eq((size_t)route->handler, (size_t)(2));

    route = capy_http_route_get(router, CAPY_HTTP_POST, str("foo/baz/bar"));
    expect_p_ne(route, NULL);
    expect_u_eq((size_t)route->handler, (size_t)(3));

    route = capy_http_route_get(router, CAPY_HTTP_POST, str("foo/bar"));
    expect_p_ne(route, NULL);
    expect_u_eq((size_t)route->handler, (size_t)(8));

    route = capy_http_route_get(router, CAPY_HTTP_GET, str("foo/bar"));
    expect_p_ne(route, NULL);
    expect_u_eq((size_t)route->handler, (size_t)(6));

    route = capy_http_route_get(router, CAPY_HTTP_POST, str("bar/foo"));
    expect_p_ne(route, NULL);
    expect_u_eq((size_t)route->handler, (size_t)(7));

    route = capy_http_route_get(router, CAPY_HTTP_POST, str("foo/123412341324/baz"));
    expect_p_ne(route, NULL);
    expect_u_eq((size_t)route->handler, (size_t)(9));

    route = capy_http_route_get(router, CAPY_HTTP_POST, str("bar/baz"));
    expect_p_eq(route, NULL);

    route = capy_http_route_get(router, CAPY_HTTP_POST, str("baz"));
    expect_p_eq(route, NULL);

    route = capy_http_route_get(router, CAPY_HTTP_POST, str(""));
    expect_p_eq(route, NULL);

    capy_strkvmmap *params = capy_http_parse_uriparams(arena, str("foo/test/bar/fe037abd-a9b8-4881-b875-a2f667e2e4ed/baz"), str("foo/^file/bar/^id/baz"));
    expect_p_ne(params, NULL);

    capy_strkvn *param = capy_strkvmmap_get(params, str("id"));
    expect_p_ne(param, NULL);
    expect_str_eq(param->key, str("id"));
    expect_str_eq(param->value, str("fe037abd-a9b8-4881-b875-a2f667e2e4ed"));

    param = capy_strkvmmap_get(params, str("file"));
    expect_p_ne(param, NULL);
    expect_str_eq(param->key, str("file"));
    expect_str_eq(param->value, str("test"));

    param = capy_strkvmmap_get(params, str("other"));
    expect_p_eq(param, NULL);

    capy_arena_destroy(arena);

    return 0;
}
