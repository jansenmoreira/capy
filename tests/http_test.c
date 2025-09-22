#include <capy/test.h>

static int test_capy_http_parse_method(void)
{
    ExpectEqS(capy_http_parse_method(Str("GET")), CAPY_HTTP_GET);
    ExpectEqS(capy_http_parse_method(Str("_ET")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("G_T")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("GE_")), CAPY_HTTP_METHOD_UNK);

    ExpectEqS(capy_http_parse_method(Str("PUT")), CAPY_HTTP_PUT);
    ExpectEqS(capy_http_parse_method(Str("_UT")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("P_T")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("PU_")), CAPY_HTTP_METHOD_UNK);

    ExpectEqS(capy_http_parse_method(Str("HEAD")), CAPY_HTTP_HEAD);
    ExpectEqS(capy_http_parse_method(Str("_EAD")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("H_AD")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("HE_D")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("HEA_")), CAPY_HTTP_METHOD_UNK);

    ExpectEqS(capy_http_parse_method(Str("POST")), CAPY_HTTP_POST);
    ExpectEqS(capy_http_parse_method(Str("_OST")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("P_ST")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("PO_T")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("POS_")), CAPY_HTTP_METHOD_UNK);

    ExpectEqS(capy_http_parse_method(Str("TRACE")), CAPY_HTTP_TRACE);
    ExpectEqS(capy_http_parse_method(Str("_RACE")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("T_ACE")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("TR_CE")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("TRA_E")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("TRAC_")), CAPY_HTTP_METHOD_UNK);

    ExpectEqS(capy_http_parse_method(Str("PATCH")), CAPY_HTTP_PATCH);
    ExpectEqS(capy_http_parse_method(Str("_ATCH")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("P_TCH")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("PA_CH")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("PAT_H")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("PATC_")), CAPY_HTTP_METHOD_UNK);

    ExpectEqS(capy_http_parse_method(Str("DELETE")), CAPY_HTTP_DELETE);
    ExpectEqS(capy_http_parse_method(Str("_ELETE")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("D_LETE")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("DE_ETE")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("DEL_TE")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("DELE_E")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("DELET_")), CAPY_HTTP_METHOD_UNK);

    ExpectEqS(capy_http_parse_method(Str("CONNECT")), CAPY_HTTP_CONNECT);
    ExpectEqS(capy_http_parse_method(Str("_ONNECT")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("C_NNECT")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("CO_NECT")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("CON_ECT")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("CONN_CT")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("CONNE_T")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("CONNEC_")), CAPY_HTTP_METHOD_UNK);

    ExpectEqS(capy_http_parse_method(Str("OPTIONS")), CAPY_HTTP_OPTIONS);
    ExpectEqS(capy_http_parse_method(Str("_PTIONS")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("O_TIONS")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("OP_IONS")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("OPT_ONS")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("OPTI_NS")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("OPTIO_S")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("OPTION_")), CAPY_HTTP_METHOD_UNK);

    ExpectEqS(capy_http_parse_method(Str("AA")), CAPY_HTTP_METHOD_UNK);
    ExpectEqS(capy_http_parse_method(Str("AAAAAAAA")), CAPY_HTTP_METHOD_UNK);

    return true;
}

static int test_capy_http_parse_version(void)
{
    ExpectEqS(capy_http_parse_version(Str("HTTP/0.9")), CAPY_HTTP_09);
    ExpectEqS(capy_http_parse_version(Str("HTTP/1.0")), CAPY_HTTP_10);
    ExpectEqS(capy_http_parse_version(Str("HTTP/1.1")), CAPY_HTTP_11);
    ExpectEqS(capy_http_parse_version(Str("HTTP/2.0")), CAPY_HTTP_20);
    ExpectEqS(capy_http_parse_version(Str("HTTP/3.0")), CAPY_HTTP_30);

    ExpectEqS(capy_http_parse_version(Str("HTTP/0.8")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("HTTP/1.2")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("HTTP/2.1")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("HTTP/3.1")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("HTTP/4.0")), CAPY_HTTP_VERSION_UNK);

    ExpectEqS(capy_http_parse_version(Str("_TTP/1.1")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("H_TP/1.1")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("HT_P/1.1")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("HTT_/1.1")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("HTTP_1.1")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("HTTP/1_1")), CAPY_HTTP_VERSION_UNK);
    ExpectEqS(capy_http_parse_version(Str("HTTP/1.1 ")), CAPY_HTTP_VERSION_UNK);

    return true;
}

static int test_capy_http_parse_reqline(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_httpreq *request = Make(arena, capy_httpreq, 1);

    ExpectOk(capy_http_parse_reqline(arena, request, Str("CONNECT localhost:8080 HTTP/1.1")));
    ExpectOk(capy_http_parse_reqline(arena, request, Str("OPTIONS * HTTP/1.1")));
    ExpectOk(capy_http_parse_reqline(arena, request, Str("OPTIONS /index.html HTTP/1.1")));
    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /index.html?id=1 HTTP/1.1")));
    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET http://localhost:8080/index.html HTTP/1.1")));
    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET HTTP://LOCALHOST:80/../%4D%30/abc/./../test HTTP/1.1")));
    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET ../%4D%20/abc/./../test HTTP/1.1")));
    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET / HTTP/1.1")));

    ExpectErr(capy_http_parse_reqline(arena, request, Str("GET  / HTTP/1.1")));
    ExpectErr(capy_http_parse_reqline(arena, request, Str("GET /  HTTP/1.1")));
    ExpectErr(capy_http_parse_reqline(arena, request, Str("GET / HTTP/1.1 ")));
    ExpectErr(capy_http_parse_reqline(arena, request, Str("get / HTTP/1.1")));
    ExpectErr(capy_http_parse_reqline(arena, request, Str("GET /%gg/ HTTP/1.1")));
    ExpectErr(capy_http_parse_reqline(arena, request, Str("GET / http/1.1")));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_http_parse_field(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_strkvnmap *fields = capy_strkvnmap_init(arena, 16);

    ExpectErr(capy_http_parse_field(fields, Str("\x01: localhost:8080")));
    ExpectErr(capy_http_parse_field(fields, Str(": localhost:8080")));
    ExpectErr(capy_http_parse_field(fields, Str("Host : localhost:8080")));
    ExpectErr(capy_http_parse_field(fields, Str("Host")));
    ExpectErr(capy_http_parse_field(fields, Str("Host: \x01")));
    ExpectOk(capy_http_parse_field(fields, Str("transfer-Encoding:  deflate ; a=1 ")));
    ExpectOk(capy_http_parse_field(fields, Str("Transfer-encoding: gzip\t; b=2 ")));
    ExpectOk(capy_http_parse_field(fields, Str("transfer-encoding: chunked")));

    capy_strkvn *field = capy_strkvnmap_get(fields, Str("Transfer-Encoding"));

    ExpectNotNull(field);
    ExpectEqStr(field->key, Str("Transfer-Encoding"));
    ExpectEqStr(field->value, Str("deflate ; a=1"));
    ExpectEqStr(field->next->value, Str("gzip\t; b=2"));
    ExpectEqStr(field->next->next->value, Str("chunked"));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_http_request_validate(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_httpreq *request = Make(arena, capy_httpreq, 1);
    capy_strkvnmap *fields = capy_strkvnmap_init(arena, 16);

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:80")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Transfer-Encoding: chunked")));
    ExpectOk(capy_http_validate_request(arena, request));
    ExpectEqU(request->content_length, 0);
    ExpectEqS(request->chunked, 1);

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET http://127.0.0.1/test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:80")));
    ExpectOk(capy_http_validate_request(arena, request));

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:80")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Content-Length: 100")));
    ExpectOk(capy_http_validate_request(arena, request));
    ExpectEqU(request->content_length, 100);
    ExpectEqS(request->chunked, 0);

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost")));
    ExpectOk(capy_http_validate_request(arena, request));

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:80")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Transfer-Encoding: gzip")));
    ExpectErr(capy_http_validate_request(arena, request));

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:80")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Transfer-Encoding: gzip")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Transfer-Encoding: chunked")));
    ExpectErr(capy_http_validate_request(arena, request));

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:80")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Transfer-Encoding: chunked")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Content-Length: 100")));
    ExpectErr(capy_http_validate_request(arena, request));

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:80")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Content-Length: 100")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Content-Length: 15")));
    ExpectErr(capy_http_validate_request(arena, request));

    request = Make(arena, capy_httpreq, 1);
    request->headers = capy_strkvnmap_init(arena, 16);

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:80")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Content-Length: af")));
    ExpectErr(capy_http_validate_request(arena, request));

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectErr(capy_http_validate_request(arena, request));

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:aa")));
    ExpectErr(capy_http_validate_request(arena, request));

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: foo@localhost:8080")));
    ExpectErr(capy_http_validate_request(arena, request));

    capy_strkvnmap_clear(fields);
    *request = (capy_httpreq){.headers = fields};

    ExpectOk(capy_http_parse_reqline(arena, request, Str("GET /test HTTP/1.1")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: localhost:80")));
    ExpectOk(capy_http_parse_field(request->headers, Str("Host: example:80")));
    ExpectErr(capy_http_validate_request(arena, request));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_http_write_response(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_strkvnmap *fields = capy_strkvnmap_init(arena, 16);

    fields = capy_strkvnmap_init(arena, 16);

    capy_buffer *body = capy_buffer_init(arena, 1024);

    capy_strkvnmap_clear(fields);

    ExpectOk(capy_strkvnmap_add(fields, Str("X-Foo"), Str("bar")));
    ExpectOk(capy_strkvnmap_add(fields, Str("X-Foo"), Str("baz")));
    ExpectOk(capy_strkvnmap_add(fields, Str("X-Foo"), Str("buz")));
    ExpectOk(capy_strkvnmap_set(fields, Str("X-Bar"), Str("foo")));

    ExpectOk(capy_buffer_write_cstr(body, "foobar"));

    capy_buffer *buffer = capy_buffer_init(arena, 1024);

    capy_httpresp response = {
        .status = 200,
        .body = body,
        .headers = fields,
    };

    ExpectOk(capy_http_write_response(buffer, &response, false));

    // char expected_response[] =
    //     "HTTP/1.1 200\r\n"
    //     "Content-Length: 6\r\n"
    //     "X-Bar: foo\r\n"
    //     "X-Foo: bar\r\n"
    //     "X-Foo: baz\r\n"
    //     "X-Foo: buz\r\n"
    //     "\r\n";

    // todo: ignore date
    // ExpectEqS(memcmp(buffer->data, expected_response, buffer->size), 0);

    capy_arena_destroy(arena);
    return true;
}

static capy_err test_handler1(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return Ok;
}

static capy_err test_handler2(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return Ok;
}

static capy_err test_handler3(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return Ok;
}

static capy_err test_handler4(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return Ok;
}

static capy_err test_handler5(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return Ok;
}

static capy_err test_handler6(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return Ok;
}

static capy_err test_handler7(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return Ok;
}

static capy_err test_handler8(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return Ok;
}

static capy_err test_handler9(Unused capy_arena *arena, Unused capy_httpreq *request, Unused capy_httpresp *response)
{
    return Ok;
}

// static int test_capy_http_router_init(void)
// {
//     capy_arena *arena = capy_arena_init(0, KiB(32));

//     capy_httprouter *router = NULL;

//     capy_httproute routes[] = {
//         {CAPY_HTTP_GET, Str("/foo/bar/baz"), test_handler1},
//         {CAPY_HTTP_POST, Str("/foo/baz"), test_handler2},
//         {CAPY_HTTP_POST, Str("/foo/baz/bar"), test_handler3},
//         {CAPY_HTTP_POST, Str("/foo/bar"), test_handler4},
//         {CAPY_HTTP_GET, Str("/foo/bar"), test_handler5},
//         {CAPY_HTTP_GET, Str("/foo/bar"), test_handler6},
//         {CAPY_HTTP_POST, Str("/bar/foo"), test_handler7},
//         {CAPY_HTTP_POST, Str("foo/bar"), test_handler8},
//         {CAPY_HTTP_POST, Str("foo/^id/baz"), test_handler9},
//     };

//     capy_httproute *route;

//     router = capy_http_router_init(arena, ArrLen(routes), routes);

//     route = capy_http_route_get(router, CAPY_HTTP_GET, Str("foo/bar/baz/"));
//     ExpectEqPtr(route, NULL);
//     expect_ptreq(route->handler, test_handler1);

//     route = capy_http_route_get(router, CAPY_HTTP_POST, Str("foo/baz/"));
//     ExpectEqPtr(route, NULL);
//     expect_ptreq(route->handler, test_handler2);

//     route = capy_http_route_get(router, CAPY_HTTP_POST, Str("foo/baz/bar"));
//     ExpectEqPtr(route, NULL);
//     expect_ptreq(route->handler, test_handler3);

//     route = capy_http_route_get(router, CAPY_HTTP_POST, Str("foo/bar"));
//     ExpectEqPtr(route, NULL);
//     expect_ptreq(route->handler, test_handler8);

//     route = capy_http_route_get(router, CAPY_HTTP_GET, Str("foo/bar"));
//     ExpectEqPtr(route, NULL);
//     expect_ptreq(route->handler, test_handler6);

//     route = capy_http_route_get(router, CAPY_HTTP_POST, Str("bar/foo"));
//     ExpectEqPtr(route, NULL);
//     expect_ptreq(route->handler, test_handler7);

//     route = capy_http_route_get(router, CAPY_HTTP_POST, Str("foo/123412341324/baz"));
//     ExpectEqPtr(route, NULL);
//     expect_ptreq(route->handler, test_handler9);

//     route = capy_http_route_get(router, CAPY_HTTP_POST, Str("bar/baz"));
//     expect_ptreq(route, NULL);

//     route = capy_http_route_get(router, CAPY_HTTP_POST, Str("baz"));
//     expect_ptreq(route, NULL);

//     route = capy_http_route_get(router, CAPY_HTTP_POST, Str(""));
//     expect_ptreq(route, NULL);

//     capy_arena_destroy(arena);
//     return true;
// }

static int test_capy_http_parse_uriparams(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    capy_strkvnmap *params = capy_strkvnmap_init(arena, 8);

    ExpectOk(capy_http_parse_uriparams(params, Str("foo/test/bar/fe037abd-a9b8-4881-b875-a2f667e2e4ed/baz"), Str("foo/^file/bar/^id/baz")));
    ExpectNotNull(params);

    capy_strkvn *param = capy_strkvnmap_get(params, Str("id"));
    ExpectNotNull(param);
    ExpectEqStr(param->key, Str("id"));
    ExpectEqStr(param->value, Str("fe037abd-a9b8-4881-b875-a2f667e2e4ed"));

    param = capy_strkvnmap_get(params, Str("file"));
    ExpectNotNull(param);
    ExpectEqStr(param->key, Str("file"));
    ExpectEqStr(param->value, Str("test"));

    param = capy_strkvnmap_get(params, Str("other"));
    ExpectNull(param);

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
    // runtest(t, test_capy_http_router_init, "capy_http_router_init");
    runtest(t, test_capy_http_parse_uriparams, "capy_http_parse_uriparams");
}
