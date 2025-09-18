#include <capy/test.h>

static int test_capy_http_parse_method(void)
{
    expect_s_eq(capy_http_parse_method(strl("GET")), CAPY_HTTP_GET);
    expect_s_eq(capy_http_parse_method(strl("_ET")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("G_T")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("GE_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(strl("PUT")), CAPY_HTTP_PUT);
    expect_s_eq(capy_http_parse_method(strl("_UT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("P_T")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("PU_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(strl("HEAD")), CAPY_HTTP_HEAD);
    expect_s_eq(capy_http_parse_method(strl("_EAD")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("H_AD")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("HE_D")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("HEA_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(strl("POST")), CAPY_HTTP_POST);
    expect_s_eq(capy_http_parse_method(strl("_OST")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("P_ST")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("PO_T")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("POS_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(strl("TRACE")), CAPY_HTTP_TRACE);
    expect_s_eq(capy_http_parse_method(strl("_RACE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("T_ACE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("TR_CE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("TRA_E")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("TRAC_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(strl("PATCH")), CAPY_HTTP_PATCH);
    expect_s_eq(capy_http_parse_method(strl("_ATCH")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("P_TCH")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("PA_CH")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("PAT_H")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("PATC_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(strl("DELETE")), CAPY_HTTP_DELETE);
    expect_s_eq(capy_http_parse_method(strl("_ELETE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("D_LETE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("DE_ETE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("DEL_TE")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("DELE_E")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("DELET_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(strl("CONNECT")), CAPY_HTTP_CONNECT);
    expect_s_eq(capy_http_parse_method(strl("_ONNECT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("C_NNECT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("CO_NECT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("CON_ECT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("CONN_CT")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("CONNE_T")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("CONNEC_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(strl("OPTIONS")), CAPY_HTTP_OPTIONS);
    expect_s_eq(capy_http_parse_method(strl("_PTIONS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("O_TIONS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("OP_IONS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("OPT_ONS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("OPTI_NS")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("OPTIO_S")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("OPTION_")), CAPY_HTTP_METHOD_UNK);

    expect_s_eq(capy_http_parse_method(strl("AA")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(strl("AAAAAAAA")), CAPY_HTTP_METHOD_UNK);

    return true;
}

static int test_capy_http_parse_version(void)
{
    expect_s_eq(capy_http_parse_version(strl("HTTP/0.9")), CAPY_HTTP_09);
    expect_s_eq(capy_http_parse_version(strl("HTTP/1.0")), CAPY_HTTP_10);
    expect_s_eq(capy_http_parse_version(strl("HTTP/1.1")), CAPY_HTTP_11);
    expect_s_eq(capy_http_parse_version(strl("HTTP/2.0")), CAPY_HTTP_20);
    expect_s_eq(capy_http_parse_version(strl("HTTP/3.0")), CAPY_HTTP_30);

    expect_s_eq(capy_http_parse_version(strl("HTTP/0.8")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("HTTP/1.2")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("HTTP/2.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("HTTP/3.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("HTTP/4.0")), CAPY_HTTP_VERSION_UNK);

    expect_s_eq(capy_http_parse_version(strl("_TTP/1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("H_TP/1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("HT_P/1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("HTT_/1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("HTTP_1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("HTTP/1_1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(strl("HTTP/1.1 ")), CAPY_HTTP_VERSION_UNK);

    return true;
}

static int test_capy_http_parse_reqline(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_http_request *request = make(arena, capy_http_request, 1);

    expect_ok(capy_http_parse_reqline(arena, request, strl("CONNECT localhost:8080 HTTP/1.1")));
    expect_ok(capy_http_parse_reqline(arena, request, strl("OPTIONS * HTTP/1.1")));
    expect_ok(capy_http_parse_reqline(arena, request, strl("OPTIONS /index.html HTTP/1.1")));
    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /index.html?id=1 HTTP/1.1")));
    expect_ok(capy_http_parse_reqline(arena, request, strl("GET http://localhost:8080/index.html HTTP/1.1")));
    expect_ok(capy_http_parse_reqline(arena, request, strl("GET HTTP://LOCALHOST:80/../%4D%30/abc/./../test HTTP/1.1")));
    expect_ok(capy_http_parse_reqline(arena, request, strl("GET ../%4D%20/abc/./../test HTTP/1.1")));
    expect_ok(capy_http_parse_reqline(arena, request, strl("GET / HTTP/1.1")));

    expect_err(capy_http_parse_reqline(arena, request, strl("GET  / HTTP/1.1")));
    expect_err(capy_http_parse_reqline(arena, request, strl("GET /  HTTP/1.1")));
    expect_err(capy_http_parse_reqline(arena, request, strl("GET / HTTP/1.1 ")));
    expect_err(capy_http_parse_reqline(arena, request, strl("get / HTTP/1.1")));
    expect_err(capy_http_parse_reqline(arena, request, strl("GET /%gg/ HTTP/1.1")));
    expect_err(capy_http_parse_reqline(arena, request, strl("GET / http/1.1")));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_http_parse_field(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_strkvmmap *fields = capy_strkvmmap_init(arena, 16);

    expect_err(capy_http_parse_field(fields, strl("\x01: localhost:8080")));
    expect_err(capy_http_parse_field(fields, strl(": localhost:8080")));
    expect_err(capy_http_parse_field(fields, strl("Host : localhost:8080")));
    expect_err(capy_http_parse_field(fields, strl("Host")));
    expect_err(capy_http_parse_field(fields, strl("Host: \x01")));
    expect_ok(capy_http_parse_field(fields, strl("transfer-Encoding:  deflate ; a=1 ")));
    expect_ok(capy_http_parse_field(fields, strl("Transfer-encoding: gzip\t; b=2 ")));
    expect_ok(capy_http_parse_field(fields, strl("transfer-encoding: chunked")));

    capy_strkvn *field = capy_strkvmmap_get(fields, strl("Transfer-Encoding"));

    expect_p_ne(field, NULL);
    expect_str_eq(field->key, strl("Transfer-Encoding"));
    expect_str_eq(field->value, strl("deflate ; a=1"));
    expect_str_eq(field->next->value, strl("gzip\t; b=2"));
    expect_str_eq(field->next->next->value, strl("chunked"));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_http_request_validate(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_http_request *request = make(arena, capy_http_request, 1);
    capy_strkvmmap *fields = capy_strkvmmap_init(arena, 16);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:80")));
    expect_ok(capy_http_parse_field(request->headers, strl("Transfer-Encoding: chunked")));
    expect_ok(capy_http_request_validate(arena, request));
    expect_u_eq(request->content_length, 0);
    expect_s_eq(request->chunked, 1);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET http://127.0.0.1/test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:80")));
    expect_ok(capy_http_request_validate(arena, request));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:80")));
    expect_ok(capy_http_parse_field(request->headers, strl("Content-Length: 100")));
    expect_ok(capy_http_request_validate(arena, request));
    expect_u_eq(request->content_length, 100);
    expect_s_eq(request->chunked, 0);

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost")));
    expect_ok(capy_http_request_validate(arena, request));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:80")));
    expect_ok(capy_http_parse_field(request->headers, strl("Transfer-Encoding: gzip")));
    expect_err(capy_http_request_validate(arena, request));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:80")));
    expect_ok(capy_http_parse_field(request->headers, strl("Transfer-Encoding: gzip")));
    expect_ok(capy_http_parse_field(request->headers, strl("Transfer-Encoding: chunked")));
    expect_err(capy_http_request_validate(arena, request));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:80")));
    expect_ok(capy_http_parse_field(request->headers, strl("Transfer-Encoding: chunked")));
    expect_ok(capy_http_parse_field(request->headers, strl("Content-Length: 100")));
    expect_err(capy_http_request_validate(arena, request));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:80")));
    expect_ok(capy_http_parse_field(request->headers, strl("Content-Length: 100")));
    expect_ok(capy_http_parse_field(request->headers, strl("Content-Length: 15")));
    expect_err(capy_http_request_validate(arena, request));

    request = make(arena, capy_http_request, 1);
    request->headers = capy_strkvmmap_init(arena, 16);

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:80")));
    expect_ok(capy_http_parse_field(request->headers, strl("Content-Length: af")));
    expect_err(capy_http_request_validate(arena, request));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_err(capy_http_request_validate(arena, request));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:aa")));
    expect_err(capy_http_request_validate(arena, request));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: foo@localhost:8080")));
    expect_err(capy_http_request_validate(arena, request));

    capy_strkvmmap_clear(fields);
    *request = (capy_http_request){.headers = fields};

    expect_ok(capy_http_parse_reqline(arena, request, strl("GET /test HTTP/1.1")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: localhost:80")));
    expect_ok(capy_http_parse_field(request->headers, strl("Host: example:80")));
    expect_err(capy_http_request_validate(arena, request));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_http_write_response(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_strkvmmap *fields = capy_strkvmmap_init(arena, 16);

    fields = capy_strkvmmap_init(arena, 16);

    capy_buffer *body = capy_buffer_init(arena, 1024);

    capy_strkvmmap_clear(fields);

    expect_ok(capy_strkvmmap_add(fields, strl("X-Foo"), strl("bar")));
    expect_ok(capy_strkvmmap_add(fields, strl("X-Foo"), strl("baz")));
    expect_ok(capy_strkvmmap_add(fields, strl("X-Foo"), strl("buz")));
    expect_ok(capy_strkvmmap_set(fields, strl("X-Bar"), strl("foo")));

    expect_ok(capy_buffer_wcstr(body, "foobar"));

    capy_buffer *buffer = capy_buffer_init(arena, 1024);

    capy_http_response response = {
        .status = 200,
        .body = body,
        .headers = fields,
    };

    expect_ok(capy_http_write_response(buffer, &response, false));

    // char expected_response[] =
    //     "HTTP/1.1 200\r\n"
    //     "Content-Length: 6\r\n"
    //     "X-Bar: foo\r\n"
    //     "X-Foo: bar\r\n"
    //     "X-Foo: baz\r\n"
    //     "X-Foo: buz\r\n"
    //     "\r\n";

    // todo: ignore date
    // expect_s_eq(memcmp(buffer->data, expected_response, buffer->size), 0);

    capy_arena_destroy(arena);
    return true;
}

static capy_err test_handler1(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return ok;
}

static capy_err test_handler2(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return ok;
}

static capy_err test_handler3(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return ok;
}

static capy_err test_handler4(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return ok;
}

static capy_err test_handler5(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return ok;
}

static capy_err test_handler6(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return ok;
}

static capy_err test_handler7(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return ok;
}

static capy_err test_handler8(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return ok;
}

static capy_err test_handler9(unused capy_arena *arena, unused capy_http_request *request, unused capy_http_response *response)
{
    return ok;
}

static int test_capy_http_router_init(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    capy_http_router *router = NULL;

    capy_http_route routes[] = {
        {CAPY_HTTP_GET, strl("/foo/bar/baz"), test_handler1},
        {CAPY_HTTP_POST, strl("/foo/baz"), test_handler2},
        {CAPY_HTTP_POST, strl("/foo/baz/bar"), test_handler3},
        {CAPY_HTTP_POST, strl("/foo/bar"), test_handler4},
        {CAPY_HTTP_GET, strl("/foo/bar"), test_handler5},
        {CAPY_HTTP_GET, strl("/foo/bar"), test_handler6},
        {CAPY_HTTP_POST, strl("/bar/foo"), test_handler7},
        {CAPY_HTTP_POST, strl("foo/bar"), test_handler8},
        {CAPY_HTTP_POST, strl("foo/^id/baz"), test_handler9},
    };

    capy_http_route *route;

    router = capy_http_router_init(arena, arrlen(routes), routes);

    route = capy_http_route_get(router, CAPY_HTTP_GET, strl("foo/bar/baz/"));
    expect_p_ne(route, NULL);
    expect_p_eq(route->handler, test_handler1);

    route = capy_http_route_get(router, CAPY_HTTP_POST, strl("foo/baz/"));
    expect_p_ne(route, NULL);
    expect_p_eq(route->handler, test_handler2);

    route = capy_http_route_get(router, CAPY_HTTP_POST, strl("foo/baz/bar"));
    expect_p_ne(route, NULL);
    expect_p_eq(route->handler, test_handler3);

    route = capy_http_route_get(router, CAPY_HTTP_POST, strl("foo/bar"));
    expect_p_ne(route, NULL);
    expect_p_eq(route->handler, test_handler8);

    route = capy_http_route_get(router, CAPY_HTTP_GET, strl("foo/bar"));
    expect_p_ne(route, NULL);
    expect_p_eq(route->handler, test_handler6);

    route = capy_http_route_get(router, CAPY_HTTP_POST, strl("bar/foo"));
    expect_p_ne(route, NULL);
    expect_p_eq(route->handler, test_handler7);

    route = capy_http_route_get(router, CAPY_HTTP_POST, strl("foo/123412341324/baz"));
    expect_p_ne(route, NULL);
    expect_p_eq(route->handler, test_handler9);

    route = capy_http_route_get(router, CAPY_HTTP_POST, strl("bar/baz"));
    expect_p_eq(route, NULL);

    route = capy_http_route_get(router, CAPY_HTTP_POST, strl("baz"));
    expect_p_eq(route, NULL);

    route = capy_http_route_get(router, CAPY_HTTP_POST, strl(""));
    expect_p_eq(route, NULL);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_http_parse_uriparams(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    capy_strkvmmap *params = capy_strkvmmap_init(arena, 8);

    expect_ok(capy_http_parse_uriparams(params, strl("foo/test/bar/fe037abd-a9b8-4881-b875-a2f667e2e4ed/baz"), strl("foo/^file/bar/^id/baz")));
    expect_p_ne(params, NULL);

    capy_strkvn *param = capy_strkvmmap_get(params, strl("id"));
    expect_p_ne(param, NULL);
    expect_str_eq(param->key, strl("id"));
    expect_str_eq(param->value, strl("fe037abd-a9b8-4881-b875-a2f667e2e4ed"));

    param = capy_strkvmmap_get(params, strl("file"));
    expect_p_ne(param, NULL);
    expect_str_eq(param->key, strl("file"));
    expect_str_eq(param->value, strl("test"));

    param = capy_strkvmmap_get(params, strl("other"));
    expect_p_eq(param, NULL);

    capy_arena_destroy(arena);
    return true;
}

static void test_http(testbench *t)
{
    runtest(t, test_capy_http_parse_method, "capy_http_parse_method");
    runtest(t, test_capy_http_parse_version, "capy_http_parse_version");
    runtest(t, test_capy_http_parse_reqline, "capy_http_parse_reqline");
    runtest(t, test_capy_http_parse_field, "capy_http_parse_field");
    runtest(t, test_capy_http_write_response, "capy_http_write_response");
    runtest(t, test_capy_http_router_init, "capy_http_router_init");
    runtest(t, test_capy_http_parse_uriparams, "capy_http_parse_uriparams");
}
