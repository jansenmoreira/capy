#include "test.h"

static int test_http(void)
{
    void *marker;

    expect_s_eq(capy_http_parse_method(str("GET")), CAPY_HTTP_GET);
    expect_s_eq(capy_http_parse_method(str("HEAD")), CAPY_HTTP_HEAD);
    expect_s_eq(capy_http_parse_method(str("POST")), CAPY_HTTP_POST);
    expect_s_eq(capy_http_parse_method(str("PUT")), CAPY_HTTP_PUT);
    expect_s_eq(capy_http_parse_method(str("DELETE")), CAPY_HTTP_DELETE);
    expect_s_eq(capy_http_parse_method(str("CONNECT")), CAPY_HTTP_CONNECT);
    expect_s_eq(capy_http_parse_method(str("OPTIONS")), CAPY_HTTP_OPTIONS);
    expect_s_eq(capy_http_parse_method(str("TRACE")), CAPY_HTTP_TRACE);

    expect_s_eq(capy_http_parse_method(str("GE_")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("HEA_")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("POS_")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("PU_")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("DELET_")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("CONNEC_")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("OPTION_")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("TRAC_")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("AA")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("AAAAAAAA")), CAPY_HTTP_METHOD_UNK);
    expect_s_eq(capy_http_parse_method(str("AAA")), CAPY_HTTP_METHOD_UNK);

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
    expect_s_eq(capy_http_parse_version(str("HTTP_1.1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTTP/1_1")), CAPY_HTTP_VERSION_UNK);
    expect_s_eq(capy_http_parse_version(str("HTTPS/1.1")), CAPY_HTTP_VERSION_UNK);

    capy_arena *arena = capy_arena_init(256);
    capy_http_request *request = capy_arena_make(capy_http_request, arena, 1);

    expect_s_eq(capy_http_parse_request_line(arena, str("GET  / HTTP/1.1"), request), EINVAL);
    expect_s_eq(capy_http_parse_request_line(arena, str("GET /  HTTP/1.1"), request), EINVAL);
    expect_s_eq(capy_http_parse_request_line(arena, str("GET / HTTP/1.1 "), request), EINVAL);

    expect_s_eq(capy_http_parse_request_line(arena, str("get / HTTP/1.1"), request), EINVAL);
    expect_s_eq(capy_http_parse_request_line(arena, str("GET /%gg/ HTTP/1.1"), request), EINVAL);
    expect_s_eq(capy_http_parse_request_line(arena, str("GET / http/1.1"), request), EINVAL);

    marker = capy_arena_top(arena);

    expect_s_eq(capy_http_parse_request_line(arena, str("GET / HTTP/1.1"), request), 0);
    capy_arena_shrink(arena, marker);
    expect_s_eq(capy_http_parse_request_line(arena, str("GET http://127.0.0.1/foo/bar?id=76294629-b23f-4e4d-a400-fe5b3b3d9f73 HTTP/1.1"), request), ENOMEM);

    capy_arena_free(arena);

    arena = capy_arena_init(350);
    capy_http_field *fields = capy_smap_of(capy_http_field, arena, 2);

    expect_s_eq(capy_http_parse_field(arena, str("\x01: localhost:8080"), &fields), EINVAL);
    expect_s_eq(capy_http_parse_field(arena, str(": localhost:8080"), &fields), EINVAL);
    expect_s_eq(capy_http_parse_field(arena, str("Host : localhost:8080"), &fields), EINVAL);
    expect_s_eq(capy_http_parse_field(arena, str("Host"), &fields), EINVAL);
    expect_s_eq(capy_http_parse_field(arena, str("Host: \x01"), &fields), EINVAL);

    expect_s_eq(capy_http_parse_field(arena, str("Transfer-Encoding:  deflate "), &fields), 0);
    expect_s_eq(capy_http_parse_field(arena, str("Transfer-Encoding: gzip "), &fields), 0);
    expect_s_eq(capy_http_parse_field(arena, str("Transfer-Encoding: chunked"), &fields), 0);

    capy_http_field *field = capy_smap_get(fields, str("transfer-encoding"));
    expect_p_ne(field, NULL);
    expect_str_eq(field->name, str("transfer-encoding"));
    expect_str_eq(field->value, str("deflate, gzip, chunked"));

    marker = capy_arena_top(arena);

    expect_s_eq(capy_http_parse_field(arena, str("Transfer-Encoding: 76294629-b23f-4e4d-a400-fe5b3b3d9f73"), &fields), ENOMEM);
    capy_arena_shrink(arena, marker);

    expect_s_eq(capy_http_parse_field(arena, str("ETag: 1"), &fields), ENOMEM);

    capy_arena_free(arena);

    arena = capy_arena_init(4 * 1024);
    request = capy_arena_make(capy_http_request, arena, 1);
    request->headers = capy_smap_of(capy_http_field, arena, 16);

    capy_http_parse_field(arena, str("Transfer-Encoding: chunked"), &request->headers);
    expect_s_eq(capy_http_content_attributes(request), 0);
    expect_u_eq(request->content_length, 0);
    expect_s_eq(request->chunked, 1);

    capy_http_parse_field(arena, str("Content-Length: 100"), &request->headers);
    expect_s_eq(capy_http_content_attributes(request), EINVAL);

    field = capy_smap_get(request->headers, str("transfer-encoding"));
    field->value = str("gzip");

    expect_s_eq(capy_http_content_attributes(request), 0);
    expect_u_eq(request->content_length, 100);
    expect_s_eq(request->chunked, 0);

    capy_smap_delete(request->headers, str("transfer-encoding"));

    expect_s_eq(capy_http_content_attributes(request), 0);
    expect_u_eq(request->content_length, 100);
    expect_s_eq(request->chunked, 0);

    capy_http_parse_field(arena, str("Content-Length: 15"), &request->headers);
    expect_s_eq(capy_http_content_attributes(request), EINVAL);

    return 0;
}
