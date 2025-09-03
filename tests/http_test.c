#include <capy/test.h>

static int test_http(void)
{
    capy_arena *arena = capy_arena_init(GiB(1));

    capy_http_request *request = capy_arena_make(capy_http_request, arena, 1);

    capy_http_field *fields;
    capy_http_field *field;

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

    expect_s_eq(capy_http_parse_request_line(arena, request, str("CONNECT localhost:8080 HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("OPTIONS * HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("OPTIONS /index.html HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET /index.html?id=1 HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET http://localhost:8080/index.html HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET HTTP://LOCALHOST:80/../%4D%30/abc/./../test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET ../%4D%20/abc/./../test HTTP/1.1")), 0);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET / HTTP/1.1")), 0);

    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET  / HTTP/1.1")), EINVAL);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET /  HTTP/1.1")), EINVAL);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET / HTTP/1.1 ")), EINVAL);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("get / HTTP/1.1")), EINVAL);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET /%gg/ HTTP/1.1")), EINVAL);
    expect_s_eq(capy_http_parse_request_line(arena, request, str("GET / http/1.1")), EINVAL);

    fields = capy_smap_of(capy_http_field, arena, 16);

    expect_s_eq(capy_http_parse_field(arena, &fields, str("\x01: localhost:8080")), EINVAL);
    expect_s_eq(capy_http_parse_field(arena, &fields, str(": localhost:8080")), EINVAL);
    expect_s_eq(capy_http_parse_field(arena, &fields, str("Host : localhost:8080")), EINVAL);
    expect_s_eq(capy_http_parse_field(arena, &fields, str("Host")), EINVAL);
    expect_s_eq(capy_http_parse_field(arena, &fields, str("Host: \x01")), EINVAL);
    expect_s_eq(capy_http_parse_field(arena, &fields, str("transfer-Encoding:  deflate ; a=1 ")), 0);
    expect_s_eq(capy_http_parse_field(arena, &fields, str("Transfer-encoding: gzip\t; b=2 ")), 0);
    expect_s_eq(capy_http_parse_field(arena, &fields, str("transfer-encoding: chunked")), 0);

    field = capy_smap_get(fields, str("Transfer-Encoding"));
    expect_p_ne(field, NULL);
    expect_str_eq(field->name, str("Transfer-Encoding"));
    expect_str_eq(field->value, str("deflate ; a=1"));
    expect_str_eq(field->next->value, str("gzip\t; b=2"));
    expect_str_eq(field->next->next->value, str("chunked"));

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:80"));
    capy_http_parse_field(arena, &request->headers, str("Transfer-Encoding: chunked"));
    expect_s_eq(capy_http_request_validate(arena, request), 0);
    expect_u_eq(request->content_length, 0);
    expect_s_eq(request->chunked, 1);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET http://127.0.0.1/test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:80"));
    expect_s_eq(capy_http_request_validate(arena, request), 0);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:80"));
    capy_http_parse_field(arena, &request->headers, str("Content-Length: 100"));
    expect_s_eq(capy_http_request_validate(arena, request), 0);
    expect_u_eq(request->content_length, 100);
    expect_s_eq(request->chunked, 0);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost"));
    expect_s_eq(capy_http_request_validate(arena, request), 0);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:80"));
    capy_http_parse_field(arena, &request->headers, str("Transfer-Encoding: gzip"));
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:80"));
    capy_http_parse_field(arena, &request->headers, str("Transfer-Encoding: gzip"));
    capy_http_parse_field(arena, &request->headers, str("Transfer-Encoding: chunked"));
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:80"));
    capy_http_parse_field(arena, &request->headers, str("Transfer-Encoding: chunked"));
    capy_http_parse_field(arena, &request->headers, str("Content-Length: 100"));
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:80"));
    capy_http_parse_field(arena, &request->headers, str("Content-Length: 100"));
    capy_http_parse_field(arena, &request->headers, str("Content-Length: 15"));
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    request = capy_arena_make(capy_http_request, arena, 1);
    request->headers = capy_smap_of(capy_http_field, arena, 16);
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:80"));
    capy_http_parse_field(arena, &request->headers, str("Content-Length: af"));
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:aa"));
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: foo@localhost:8080"));
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    capy_smap_clear(fields);
    *request = (capy_http_request){.headers = fields};
    capy_http_parse_request_line(arena, request, str("GET /test HTTP/1.1"));
    capy_http_parse_field(arena, &request->headers, str("Host: localhost:80"));
    capy_http_parse_field(arena, &request->headers, str("Host: example:80"));
    expect_s_eq(capy_http_request_validate(arena, request), EINVAL);

    return 0;
}
