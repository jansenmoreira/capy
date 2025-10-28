#include <capy/test.h>

#include "../src/capy.c"

static int test_capy_error(void)
{
    capy_err err = {.code = 1, .msg = "test"};

    err = capy_err_fmt(2, "wrap1 (%s)", err.msg);
    ExpectEqS(err.code, 2);
    ExpectEqCstr("wrap1 (test)", err.msg);

    err = capy_err_fmt(3, "wrap2 (%s)", err.msg);
    ExpectEqS(err.code, 3);
    ExpectEqCstr("wrap2 (wrap1 (test))", err.msg);

    err = capy_err_fmt(4, "wrap3 (%s)", err.msg);
    ExpectEqS(err.code, 4);
    ExpectEqCstr("wrap3 (wrap2 (wrap1 (test)))", err.msg);

    return true;
}

static int test_capy_buffer_init(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    ExpectNull(capy_buffer_init(arena, KiB(8)));

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    ExpectEqU(buffer->size, 0);
    ExpectGteU(buffer->capacity, 8);
    ExpectEqPtr(buffer->arena, arena);
    ExpectNotNull(buffer->data);

    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_wbytes_enomem(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    size_t size = KiB(3);

    capy_string s = {.size = size, .data = Make(arena, char, size)};
    ExpectNotNull(s.data);

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    ExpectErr(capy_buffer_write_bytes(buffer, s.size, s.data));
    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_format_enomem(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    ExpectErr(capy_buffer_write_fmt(buffer, 0, "%*s", KiB(8), " "));
    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_writes(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);

    ExpectOk(capy_buffer_write_string(buffer, Str("foobar\n")));
    ExpectOk(capy_buffer_write_cstr(buffer, "baz"));
    ExpectOk(capy_buffer_write_bytes(buffer, 2, "baz"));
    ExpectOk(capy_buffer_write_fmt(buffer, 50, " %d %.1f", 5, 1.3f));
    ExpectEqMem(buffer->data, "foobar\nbazba 5 1.3", buffer->size);

    capy_buffer_shl(buffer, 7);
    ExpectEqMem(buffer->data, "bazba 5 1.3", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_fmt(buffer, 4, "%d", 123456));
    ExpectEqMem(buffer->data, "1234", buffer->size);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_base64(void)
{
    char content[256];
    size_t bytes;

    bytes = capy_base64url(content, 6, "foobar", false);
    ExpectEqU(bytes, 8);
    ExpectEqMem(content, "Zm9vYmFy", bytes);

    bytes = capy_base64url(content, 6, "abcdef", false);
    ExpectEqU(bytes, 8);
    ExpectEqMem(content, "YWJjZGVm", bytes);

    bytes = capy_base64url(content, 7, "foobarb", false);
    ExpectEqU(bytes, 10);
    ExpectEqMem(content, "Zm9vYmFyYg", bytes);

    bytes = capy_base64url(content, 7, "foobarb", true);
    ExpectEqU(bytes, 12);
    ExpectEqMem(content, "Zm9vYmFyYg==", bytes);

    bytes = capy_base64url(content, 8, "foobarbz", false);
    ExpectEqU(bytes, 11);
    ExpectEqMem(content, "Zm9vYmFyYno", bytes);

    bytes = capy_base64url(content, 8, "foobarbz", true);
    ExpectEqU(bytes, 12);
    ExpectEqMem(content, "Zm9vYmFyYno=", bytes);

    bytes = capy_base64url(content, 3, "\x00\x1F\xBF", false);
    ExpectEqU(bytes, 4);
    ExpectEqMem(content, "AB-_", bytes);

    bytes = capy_base64std(content, 3, "\x00\x1F\xBF", false);
    ExpectEqU(bytes, 4);
    ExpectEqMem(content, "AB+/", bytes);

    return true;
}

static int test_capy_string_base64(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(8));

    capy_string result;

    ExpectOk(capy_string_base64url(arena, &result, Str("foobar"), false));
    ExpectEqStr(result, Str("Zm9vYmFy"));

    ExpectOk(capy_string_base64url(arena, &result, Str("abcdef"), false));
    ExpectEqStr(result, Str("YWJjZGVm"));

    ExpectOk(capy_string_base64url(arena, &result, Str("foobarb"), false));
    ExpectEqStr(result, Str("Zm9vYmFyYg"));

    ExpectOk(capy_string_base64url(arena, &result, Str("foobarb"), true));
    ExpectEqStr(result, Str("Zm9vYmFyYg=="));

    ExpectOk(capy_string_base64url(arena, &result, Str("foobarbz"), false));
    ExpectEqStr(result, Str("Zm9vYmFyYno"));

    ExpectOk(capy_string_base64url(arena, &result, Str("foobarbz"), true));
    ExpectEqStr(result, Str("Zm9vYmFyYno="));

    ExpectOk(capy_string_base64url(arena, &result, Str("\x00\x1F\xBF"), false));
    ExpectEqStr(result, Str("AB-_"));

    ExpectOk(capy_string_base64std(arena, &result, Str("\x00\x1F\xBF"), false));
    ExpectEqStr(result, Str("AB+/"));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_buffer_base64(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(8));

    capy_buffer *buffer = capy_buffer_init(arena, 1024);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 6, "foobar", false));
    ExpectEqMem(buffer->data, "Zm9vYmFy", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 6, "abcdef", false));
    ExpectEqMem(buffer->data, "YWJjZGVm", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 7, "foobarb", false));
    ExpectEqMem(buffer->data, "Zm9vYmFyYg", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 7, "foobarb", true));
    ExpectEqMem(buffer->data, "Zm9vYmFyYg==", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 8, "foobarbz", false));
    ExpectEqMem(buffer->data, "Zm9vYmFyYno", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 8, "foobarbz", true));
    ExpectEqMem(buffer->data, "Zm9vYmFyYno=", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 3, "\x00\x1F\xBF", false));
    ExpectEqMem(buffer->data, "AB-_", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64std(buffer, 3, "\x00\x1F\xBF", false));
    ExpectEqMem(buffer->data, "AB+/", buffer->size);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_arena_init(void)
{
    ExpectNull(capy_arena_init(0, -1ULL));

    capy_arena *arena = capy_arena_init(0, KiB(32));
    ExpectNotNull(arena);

    ExpectOk(capy_arena_destroy(arena));

    return true;
}

static int test_capy_arena_alloc(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    size_t s16 = Cast(size_t, capy_arena_alloc(arena, sizeof(int16_t), alignof(int16_t), true));
    ExpectEqU(s16 % alignof(int16_t), 0);

    size_t s32 = Cast(size_t, capy_arena_alloc(arena, sizeof(int32_t), alignof(int32_t), true));
    ExpectEqU(s32 % alignof(int32_t), 0);

    size_t s64 = Cast(size_t, capy_arena_alloc(arena, sizeof(int64_t), alignof(int64_t), true));
    ExpectEqU(s64 % alignof(int64_t), 0);

    size_t size = capy_arena_used(arena);
    void *chunk = capy_arena_alloc(arena, KiB(16), 0, false);
    ExpectNotNull(chunk);
    ExpectGteU(capy_arena_used(arena) - size, KiB(16));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_arena_free(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    void *end = capy_arena_end(arena);
    void *chunk = capy_arena_alloc(arena, KiB(16), 0, false);

    ExpectOk(capy_arena_free(arena, chunk));
    ExpectEqPtr(capy_arena_end(arena), chunk);
    ExpectOk(capy_arena_free(arena, end));
    ExpectEqPtr(capy_arena_end(arena), end);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_arena_realloc(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    size_t size = capy_arena_used(arena);
    void *data = capy_arena_alloc(arena, KiB(1), 8, false);
    ExpectNotNull(data);
    void *tmp = capy_arena_realloc(arena, data, KiB(1), KiB(2), false);
    ExpectEqPtr(data, tmp);
    ExpectGteU(capy_arena_used(arena) - size, KiB(2));

    data = tmp;

    ExpectNotNull(Make(arena, int, 1));
    size = capy_arena_used(arena);
    tmp = capy_arena_realloc(arena, data, KiB(2), KiB(4), false);

    ExpectNotNull(tmp);
    ExpectNePtr(data, tmp);
    ExpectGteU(capy_arena_used(arena) - size, KiB(2));

    data = tmp;

    ExpectNull(capy_arena_realloc(arena, data, KiB(2), MiB(2), false));
    ExpectNotNull(Make(arena, int, 1));
    ExpectNull(capy_arena_realloc(arena, data, KiB(2), MiB(2), false));

    capy_arena_destroy(arena);
    return true;
}

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

static int test_capy_json_deserialize(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_jsonval value;

    ExpectOk(capy_json_deserialize(arena, &value, "null"));
    ExpectEqS(value.kind, CAPY_JSON_NULL);

    ExpectOk(capy_json_deserialize(arena, &value, "true"));
    ExpectEqS(value.kind, CAPY_JSON_BOOL);
    ExpectTrue(value.boolean);

    ExpectOk(capy_json_deserialize(arena, &value, "false"));
    ExpectEqS(value.kind, CAPY_JSON_BOOL);
    ExpectFalse(value.boolean);

    ExpectOk(capy_json_deserialize(arena, &value, "[\"abcd𐐷\\uD801\\uDC37\",true,null,{},[]]"));
    ExpectEqS(value.kind, CAPY_JSON_ARRAY);
    ExpectEqU(value.array->size, 5);
    ExpectEqS(value.array->data[0].kind, CAPY_JSON_STRING);
    ExpectEqCstr(value.array->data[0].string, "abcd𐐷𐐷");
    ExpectEqS(value.array->data[1].kind, CAPY_JSON_BOOL);
    ExpectTrue(value.array->data[1].boolean);
    ExpectEqS(value.array->data[2].kind, CAPY_JSON_NULL);

    capy_buffer *buffer = capy_buffer_init(arena, KiB(1));
    capy_json_serialize(buffer, value, 3);
    // printf("%s\n", buffer->data);

    ExpectOk(capy_json_deserialize(arena, &value, "300 "));

    return true;
}

static int test_capy_json_serialize(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));
    capy_buffer *buffer = capy_buffer_init(arena, KiB(1));

    capy_jsonval arr = capy_json_array(arena);
    ExpectOk(capy_json_array_push(arr.array, capy_json_bool(true)));
    ExpectOk(capy_json_array_push(arr.array, capy_json_bool(false)));
    ExpectOk(capy_json_array_push(arr.array, capy_json_null()));

    capy_jsonval obj = capy_json_object(arena);
    ExpectOk(capy_json_object_set(obj.object, "a", capy_json_string("teste")));
    ExpectOk(capy_json_object_set(obj.object, "b", capy_json_number(10)));
    ExpectOk(capy_json_object_set(obj.object, "c", capy_json_number(16.32)));
    ExpectOk(capy_json_object_set(obj.object, "d", arr));

    capy_json_serialize(buffer, obj, 0);
    // printf("%s\n", buffer->data);
    buffer->size = 0;

    capy_json_serialize(buffer, obj, 3);
    // printf("%s\n", buffer->data);

    return true;
}

static int test_capy_string_copy(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string input = Str("The quick brown fox jumps over the lazy dog");
    capy_string output;

    ExpectOk(capy_string_copy(arena, &output, input));
    ExpectNePtr(output.data, input.data);
    ExpectEqStr(output, input);

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_string_copy(arena, &output, input));

    input.size = 0;
    ExpectOk(capy_string_copy(arena, &output, input));
    ExpectNull(output.data);
    ExpectEqU(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_lower(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string expected = Str("+azaz09{}");
    capy_string input = Str("+AZaz09{}");
    capy_string output;

    ExpectOk(capy_string_lower(arena, &output, input));
    ExpectEqStr(output, expected);

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_string_lower(arena, &output, input));

    input.size = 0;
    ExpectOk(capy_string_lower(arena, &output, input));
    ExpectNull(output.data);
    ExpectEqU(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_upper(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string expected = Str("+AZAZ09{}");
    capy_string input = Str("+AZaz09{}");
    capy_string output;

    ExpectOk(capy_string_upper(arena, &output, input));
    ExpectEqStr(output, expected);

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_string_upper(arena, &output, input));

    input.size = 0;
    ExpectOk(capy_string_upper(arena, &output, input));
    ExpectNull(output.data);
    ExpectEqU(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_join(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string expected = Str("one two three");
    capy_string input[] = {StrIni("one"), StrIni("two"), StrIni("three")};
    capy_string output;

    ExpectOk(capy_string_join(arena, &output, " ", 3, input));
    ExpectEqStr(output, expected);

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_string_join(arena, &output, " ", 3, input));

    ExpectOk(capy_string_join(arena, &output, " ", 0, input));
    ExpectNull(output.data);
    ExpectEqU(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_trim(void)
{
    ExpectEqStr(capy_string_trim(Str(""), " "), Str(""));
    ExpectEqStr(capy_string_trim(Str("  "), " "), Str(""));
    ExpectEqStr(capy_string_trim(Str("  a "), " "), Str("a"));
    ExpectEqStr(capy_string_trim(Str("a   "), " "), Str("a"));
    return true;
}

static int test_capy_string_prefix(void)
{
    ExpectEqStr(capy_string_prefix(Str("foobar"), Str("foo")), Str("foo"));
    ExpectEqStr(capy_string_prefix(Str("foo"), Str("foobar")), Str("foo"));
    ExpectEqStr(capy_string_prefix(Str("foobar"), Str("bar")), Str(""));
    return true;
}

static int test_capy_string_hex(void)
{
    size_t bytes;
    uint64_t value;

    bytes = capy_string_parse_hexdigits(&value, Str("a0b1c2d3e4f5"));
    ExpectEqU(bytes, 12);
    ExpectEqU(value, 0xa0b1c2d3e4f5);

    bytes = capy_string_parse_hexdigits(&value, Str("A9B8C7D6E5F4 "));
    ExpectEqU(bytes, 12);
    ExpectEqU(value, 0xA9B8C7D6E5F4);

    bytes = capy_string_parse_hexdigits(&value, Str(""));
    ExpectEqU(bytes, 0);

    bytes = capy_string_parse_hexdigits(&value, Str("-"));
    ExpectEqU(bytes, 0);

    return true;
}

static int test_capy_string_eq(void)
{
    char a1[] = "foo";
    char a2[] = {'f', 'o', 'o'};
    const char *a3 = "bar";
    const char *a4 = "";

    capy_string s1 = {.data = a1, .size = 3};
    capy_string s2 = {.data = a2, .size = 3};
    capy_string s3 = {.data = a3, .size = 3};
    capy_string s4 = {.data = a4, .size = 0};

    ExpectTrue(capy_string_eq(s1, s2));
    ExpectFalse(capy_string_eq(s1, s3));
    ExpectFalse(capy_string_eq(s1, s4));

    return true;
}

static int test_capy_string_cstr(void)
{
    const char *cstr = "foo";
    capy_string s1 = capy_string_cstr(cstr);
    ExpectEqPtr(s1.data, cstr);
    ExpectEqU(s1.size, 3);
    return true;
}

static int test_capy_string_slice(void)
{
    capy_string input = Str("foobar");
    ExpectEqStr(capy_string_slice(input, 1, 3), Str("oo"));
    ExpectEqStr(capy_string_shl(input, 3), Str("bar"));
    ExpectEqStr(capy_string_shr(input, 3), Str("foo"));
    return true;
}

static int test_capy_strset(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    ExpectNull(capy_strset_init(arena, KiB(8)));

    capy_strset *strset = capy_strset_init(arena, 2);

    ExpectNotNull(strset);
    ExpectEqU(strset->capacity, 2);
    ExpectEqU(strset->size, 0);
    ExpectEqPtr(strset->arena, arena);
    ExpectNotNull(strset->items);

    ExpectEqS(capy_strset_has(strset, Str("foo")), false);

    ExpectOk(capy_strset_add(strset, Str("foo")));
    ExpectEqS(capy_strset_has(strset, Str("foo")), true);

    capy_strset_delete(strset, Str("foo"));

    ExpectEqS(capy_strset_has(strset, Str("foo")), false);

    ExpectOk(capy_strset_add(strset, Str("foo")));
    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_strset_add(strset, Str("bar")));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_strkvmap(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    ExpectNull(capy_strkvmap_init(arena, KiB(8)));

    capy_strkvmap *strkvmap = capy_strkvmap_init(arena, 2);

    ExpectNotNull(strkvmap);
    ExpectEqU(strkvmap->capacity, 2);
    ExpectEqU(strkvmap->size, 0);
    ExpectEqPtr(strkvmap->arena, arena);
    ExpectNotNull(strkvmap->items);

    capy_strkv *kv = capy_strkvmap_get(strkvmap, Str("foo"));
    ExpectNull(kv);

    ExpectOk(capy_strkvmap_set(strkvmap, Str("foo"), Str("bar")));

    kv = capy_strkvmap_get(strkvmap, Str("foo"));
    ExpectNotNull(kv);
    ExpectEqStr(kv->key, Str("foo"));
    ExpectEqStr(kv->value, Str("bar"));

    capy_strkvmap_delete(strkvmap, Str("foo"));
    ExpectNull(capy_strkvmap_get(strkvmap, Str("foo")));

    ExpectOk(capy_strkvmap_set(strkvmap, Str("foo"), Str("bar")));

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_strkvmap_set(strkvmap, Str("bar"), Str("foo")));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_strkvnmap(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    ExpectNull(capy_strkvnmap_init(arena, KiB(8)));

    capy_strkvnmap *strkvnmap = capy_strkvnmap_init(arena, 2);

    ExpectNotNull(strkvnmap);
    ExpectEqU(strkvnmap->capacity, 2);
    ExpectEqU(strkvnmap->size, 0);
    ExpectEqPtr(strkvnmap->arena, arena);
    ExpectNotNull(strkvnmap->items);

    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("bar")));
    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("baz")));
    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("buz")));

    capy_strkvn *strkvn = capy_strkvnmap_get(strkvnmap, Str("foo"));
    ExpectNotNull(strkvn);
    ExpectEqStr(strkvn->key, Str("foo"));
    ExpectEqStr(strkvn->value, Str("bar"));
    ExpectNotNull(strkvn->next);
    ExpectEqStr(strkvn->next->key, Str("foo"));
    ExpectEqStr(strkvn->next->value, Str("baz"));
    ExpectNotNull(strkvn->next->next);
    ExpectEqStr(strkvn->next->next->key, Str("foo"));
    ExpectEqStr(strkvn->next->next->value, Str("buz"));
    ExpectNull(strkvn->next->next->next);

    ExpectOk(capy_strkvnmap_set(strkvnmap, Str("foo"), Str("bar")));

    strkvn = capy_strkvnmap_get(strkvnmap, Str("foo"));
    ExpectNotNull(strkvn);
    ExpectEqStr(strkvn->key, Str("foo"));
    ExpectEqStr(strkvn->value, Str("bar"));
    ExpectNull(strkvn->next);

    capy_strkvnmap_delete(strkvnmap, Str("foo"));
    strkvn = capy_strkvnmap_get(strkvnmap, Str("foo"));
    ExpectNull(strkvn);

    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("bar"), Str("foo")));
    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("bar")));
    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("baz")));

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("fail")));
    ExpectErr(capy_strkvnmap_add(strkvnmap, Str("baz"), Str("fail")));
    ExpectErr(capy_strkvnmap_set(strkvnmap, Str("baz"), Str("fail")));

    return true;
}

static int test_capy_vec_insert(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_vec vec = {
        .size = 0,
        .capacity = 8,
        .element_size = sizeof(int),
        .items = NULL,
    };

    vec.items = Make(arena, int, vec.capacity);

    ExpectOk(capy_vec_insert(arena, &vec, 0, 5, Arr(int, 6, 7, 8, 9, 10)));
    ExpectEqU(vec.size, 5);

    ExpectOk(capy_vec_insert(arena, &vec, 0, 5, Arr(int, 1, 2, 3, 4, 5)));
    ExpectEqU(vec.size, 10);

    ExpectOk(capy_vec_insert(arena, &vec, 0, 1, NULL));
    ExpectEqU(vec.size, 11);

    int *data = Cast(int *, vec.items);
    data[0] = 0;

    for (int i = 0; i < 11; i++)
    {
        ExpectEqS(data[i], i);
    }

    ExpectErr(capy_vec_insert(arena, &vec, 0, MiB(1), NULL));
    ExpectErr(capy_vec_insert(arena, &vec, 20, 10, NULL));
    ExpectErr(capy_vec_insert(NULL, &vec, 0, 6, NULL));

    return true;
}

static int test_capy_vec_delete(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_vec vec = {
        .size = 0,
        .capacity = 8,
        .element_size = sizeof(int),
        .items = NULL,
    };

    vec.items = Make(arena, int, vec.capacity);

    ExpectOk(capy_vec_insert(arena, &vec, 0, 11, Arr(int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)));
    ExpectEqU(vec.size, 11);

    ExpectOk(capy_vec_delete(&vec, 5, 0));
    ExpectEqU(vec.size, 11);

    ExpectOk(capy_vec_delete(&vec, 0, 5));
    ExpectEqU(vec.size, 6);

    ExpectOk(capy_vec_delete(&vec, 0, 6));
    ExpectEqU(vec.size, 0);

    ExpectErr(capy_vec_delete(&vec, 0, 1));

    return true;
}

// Task

static int test_taskqueue(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    union taskqueue *queue = taskqueue_init(arena);

    struct task tasks[] = {
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 10},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 9},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 8},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 7},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 6},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 5},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 4},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 3},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 2},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 1},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 0},
    };

    for (size_t i = 0; i < ArrLen(tasks); i++)
    {
        ExpectOk(taskqueue_add(queue, tasks + i));
    }

    ExpectEqPtr(taskqueue_remove(queue, tasks[0].queuepos), tasks + 0);
    ExpectEqU(tasks[0].queuepos, TASKQUEUE_REMOVED);

    for (size_t i = ArrLen(tasks); i > 1; i--)
    {
        struct task *task = taskqueue_pop(queue);
        ExpectEqPtr(task, tasks + i - 1);
    }

    capy_arena_destroy(arena);
    return true;
}

// URI

static int test_uri_parse(void)
{
    struct table_uri
    {
        capy_string raw;
        capy_string scheme;
        capy_string authority;
        capy_string userinfo;
        capy_string host;
        capy_string port;
        capy_string path;
        capy_string query;
        capy_string fragment;
        int64_t is_valid;
    };

    struct table_uri uris[] = {
        {
            .raw = Str("http://foo@localhost:8080/post?id=1#title"),
            .scheme = Str("http"),
            .authority = Str("foo@localhost:8080"),
            .userinfo = Str("foo"),
            .host = Str("localhost"),
            .port = Str("8080"),
            .path = Str("/post"),
            .query = Str("id=1"),
            .fragment = Str("title"),
            .is_valid = true,
        },
        {
            .raw = Str("data:text/plain;charset=utf-8,foo%20bar"),
            .scheme = Str("data"),
            .path = Str("text/plain;charset=utf-8,foo%20bar"),
            .is_valid = true,
        },
        {
            .raw = Str("foo bar://localhost"),
            .scheme = Str("foo bar"),
            .authority = Str("localhost"),
            .host = Str("localhost"),
            .is_valid = false,
        },
        {
            .raw = Str(" http://localhost"),
            .scheme = Str(" http"),
            .authority = Str("localhost"),
            .host = Str("localhost"),
            .is_valid = false,
        },
        {
            .raw = Str(":"),
            .scheme = Str(""),
            .is_valid = false,
        },
        {
            .raw = Str("h:"),
            .scheme = Str("h"),
            .is_valid = true,
        },
        {
            .raw = Str("//127.0.0.1"),
            .authority = Str("127.0.0.1"),
            .host = Str("127.0.0.1"),
            .is_valid = true,
        },
        {
            .raw = Str("//10.20.30.5"),
            .authority = Str("10.20.30.5"),
            .host = Str("10.20.30.5"),
            .is_valid = true,
        },
        {
            .raw = Str("//1O.0.0.1"),
            .authority = Str("1O.0.0.1"),
            .host = Str("1O.0.0.1"),
            .is_valid = true,
        },
        {
            .raw = Str("//10"),
            .authority = Str("10"),
            .host = Str("10"),
            .is_valid = true,
        },
        {
            .raw = Str("//30.40.50.60"),
            .authority = Str("30.40.50.60"),
            .host = Str("30.40.50.60"),
            .is_valid = true,
        },
        {
            .raw = Str("//70.80.90.9O"),
            .authority = Str("70.80.90.9O"),
            .host = Str("70.80.90.9O"),
            .is_valid = true,
        },
        {
            .raw = Str("//foo%2"),
            .authority = Str("foo%2"),
            .host = Str("foo%2"),
            .is_valid = false,
        },
        {
            .raw = Str("//foo%20bar"),
            .authority = Str("foo%20bar"),
            .host = Str("foo%20bar"),
            .is_valid = true,
        },
        {
            .raw = Str("//foo bar"),
            .authority = Str("foo bar"),
            .host = Str("foo bar"),
            .is_valid = false,
        },
        {
            .raw = Str("//foo%0gbar"),
            .authority = Str("foo%0gbar"),
            .host = Str("foo%0gbar"),
            .is_valid = false,
        },
        {
            .raw = Str("//foo%g0bar"),
            .authority = Str("foo%g0bar"),
            .host = Str("foo%g0bar"),
            .is_valid = false,
        },
        {
            .raw = Str("/foo@bar"),
            .path = Str("/foo@bar"),
            .is_valid = true,
        },
        {
            .raw = Str("/foo/^bar"),
            .path = Str("/foo/^bar"),
            .is_valid = false,
        },
        {
            .raw = Str("/foo^bar/"),
            .path = Str("/foo^bar/"),
            .is_valid = false,
        },
        {
            .raw = Str("foo/"),
            .path = Str("foo/"),
            .is_valid = true,
        },
        {
            .raw = Str("/foo"),
            .path = Str("/foo"),
            .is_valid = true,
        },
        {
            .raw = Str("//foo@[::1]:8080"),
            .authority = Str("foo@[::1]:8080"),
            .userinfo = Str("foo"),
            .host = Str("[::1]"),
            .port = Str("8080"),
            .is_valid = true,
        },
        {
            .raw = Str("//[::12345]"),
            .authority = Str("[::12345]"),
            .host = Str("[::12345]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[::127.0.0.1]"),
            .authority = Str("[::127.0.0.1]"),
            .host = Str("[::127.0.0.1]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[::1:2:3:4:5:6:7]"),
            .authority = Str("[::1:2:3:4:5:6:7]"),
            .host = Str("[::1:2:3:4:5:6:7]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[::1:2:3:4:5:6:7:8]"),
            .authority = Str("[::1:2:3:4:5:6:7:8]"),
            .host = Str("[::1:2:3:4:5:6:7:8]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[1::2:3:4:5:6:7]"),
            .authority = Str("[1::2:3:4:5:6:7]"),
            .host = Str("[1::2:3:4:5:6:7]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[1::2:3:4:5:6:7:8]"),
            .authority = Str("[1::2:3:4:5:6:7:8]"),
            .host = Str("[1::2:3:4:5:6:7:8]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[]"),
            .authority = Str("[]"),
            .host = Str("[]"),
            .is_valid = false,
        },
        {
            .raw = Str("//["),
            .authority = Str("["),
            .host = Str("["),
            .is_valid = false,
        },
        {
            .raw = Str("//[1:]"),
            .authority = Str("[1:]"),
            .host = Str("[1:]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[1:2:3:4:5:6:7:8]"),
            .authority = Str("[1:2:3:4:5:6:7:8]"),
            .host = Str("[1:2:3:4:5:6:7:8]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[0:0:0:0:0:0:127.0.0.1]"),
            .authority = Str("[0:0:0:0:0:0:127.0.0.1]"),
            .host = Str("[0:0:0:0:0:0:127.0.0.1]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[0:0:0:0:0:0:0:127.0.0.1]"),
            .authority = Str("[0:0:0:0:0:0:0:127.0.0.1]"),
            .host = Str("[0:0:0:0:0:0:0:127.0.0.1]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[::0:0:0:0:0:0:127.0.0.1]"),
            .authority = Str("[::0:0:0:0:0:0:127.0.0.1]"),
            .host = Str("[::0:0:0:0:0:0:127.0.0.1]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[::0::1]"),
            .authority = Str("[::0::1]"),
            .host = Str("[::0::1]"),
            .is_valid = false,
        },
        {
            .raw = Str("//foo bar@localhost"),
            .authority = Str("foo bar@localhost"),
            .userinfo = Str("foo bar"),
            .host = Str("localhost"),
            .is_valid = false,
        },
        {
            .raw = Str("//localhost: 8080"),
            .authority = Str("localhost: 8080"),
            .host = Str("localhost"),
            .port = Str(" 8080"),
            .is_valid = false,
        },
        {
            .raw = Str("index.html?id=foo bar"),
            .path = Str("index.html"),
            .query = Str("id=foo bar"),
            .is_valid = false,
        },
        {
            .raw = Str("index.html#id=foo bar"),
            .path = Str("index.html"),
            .fragment = Str("id=foo bar"),
            .is_valid = false,
        },
        {
            .raw = Str("//localhost?id=1"),
            .authority = Str("localhost"),
            .host = Str("localhost"),
            .query = Str("id=1"),
            .is_valid = true,
        },
        {
            .raw = Str("//localhost#title"),
            .authority = Str("localhost"),
            .host = Str("localhost"),
            .fragment = Str("title"),
            .is_valid = true,
        },
    };

    for (size_t i = 0; i < ArrLen(uris); i++)
    {
        capy_uri uri = capy_uri_parse(uris[i].raw);

        ExpectEqS(capy_uri_valid(uri), uris[i].is_valid);
        ExpectEqStr(uri.scheme, uris[i].scheme);
        ExpectEqStr(uri.authority, uris[i].authority);
        ExpectEqStr(uri.userinfo, uris[i].userinfo);
        ExpectEqStr(uri.host, uris[i].host);
        ExpectEqStr(uri.port, uris[i].port);
        ExpectEqStr(uri.path, uris[i].path);
        ExpectEqStr(uri.query, uris[i].query);
        ExpectEqStr(uri.fragment, uris[i].fragment);
    }

    return true;
}

static int test_uri_resolve_reference(void)
{
    struct table_resolve
    {
        capy_string base;
        capy_string reference;
        capy_string expected;
    };

    struct table_resolve uris_reference[] = {
        {Str("//localhost"), Str(""), Str("//localhost")},
        {Str("//localhost"), Str("index.html"), Str("//localhost/index.html")},
        {Str("//localhost"), Str("https:a/.."), Str("https:")},

        // RFC 3986: https://datatracker.ietf.org/doc/html/rfc3986#section-5.4.1
        {Str("http://a/b/c/d;p?q"), Str("g:h"), Str("g:h")},
        {Str("http://a/b/c/d;p?q"), Str("g"), Str("http://a/b/c/g")},
        {Str("http://a/b/c/d;p?q"), Str("./g"), Str("http://a/b/c/g")},
        {Str("http://a/b/c/d;p?q"), Str("g/"), Str("http://a/b/c/g/")},
        {Str("http://a/b/c/d;p?q"), Str("/g"), Str("http://a/g")},
        {Str("http://a/b/c/d;p?q"), Str("//g"), Str("http://g")},
        {Str("http://a/b/c/d;p?q"), Str("?y"), Str("http://a/b/c/d;p?y")},
        {Str("http://a/b/c/d;p?q"), Str("g?y"), Str("http://a/b/c/g?y")},
        {Str("http://a/b/c/d;p?q"), Str("#s"), Str("http://a/b/c/d;p?q#s")},
        {Str("http://a/b/c/d;p?q"), Str("g#s"), Str("http://a/b/c/g#s")},
        {Str("http://a/b/c/d;p?q"), Str("g?y#s"), Str("http://a/b/c/g?y#s")},
        {Str("http://a/b/c/d;p?q"), Str(";x"), Str("http://a/b/c/;x")},
        {Str("http://a/b/c/d;p?q"), Str("g;x"), Str("http://a/b/c/g;x")},
        {Str("http://a/b/c/d;p?q"), Str("g;x?y#s"), Str("http://a/b/c/g;x?y#s")},
        {Str("http://a/b/c/d;p?q"), Str(""), Str("http://a/b/c/d;p?q")},
        {Str("http://a/b/c/d;p?q"), Str("."), Str("http://a/b/c/")},
        {Str("http://a/b/c/d;p?q"), Str("./"), Str("http://a/b/c/")},
        {Str("http://a/b/c/d;p?q"), Str(".."), Str("http://a/b/")},
        {Str("http://a/b/c/d;p?q"), Str("../"), Str("http://a/b/")},
        {Str("http://a/b/c/d;p?q"), Str("../g"), Str("http://a/b/g")},
        {Str("http://a/b/c/d;p?q"), Str("../.."), Str("http://a/")},
        {Str("http://a/b/c/d;p?q"), Str("../../"), Str("http://a/")},
        {Str("http://a/b/c/d;p?q"), Str("../../g"), Str("http://a/g")},
        {Str("http://a/b/c/d;p?q"), Str("/./g"), Str("http://a/g")},
        {Str("http://a/b/c/d;p?q"), Str("/../g"), Str("http://a/g")},
        {Str("http://a/b/c/d;p?q"), Str("g."), Str("http://a/b/c/g.")},
        {Str("http://a/b/c/d;p?q"), Str(".g"), Str("http://a/b/c/.g")},
        {Str("http://a/b/c/d;p?q"), Str("g.."), Str("http://a/b/c/g..")},
        {Str("http://a/b/c/d;p?q"), Str("..g"), Str("http://a/b/c/..g")},
        {Str("http://a/b/c/d;p?q"), Str("./../g"), Str("http://a/b/g")},
        {Str("http://a/b/c/d;p?q"), Str("./g/."), Str("http://a/b/c/g/")},
        {Str("http://a/b/c/d;p?q"), Str("g/./h"), Str("http://a/b/c/g/h")},
        {Str("http://a/b/c/d;p?q"), Str("g/../h"), Str("http://a/b/c/h")},
        {Str("http://a/b/c/d;p?q"), Str("g;x=1/./y"), Str("http://a/b/c/g;x=1/y")},
        {Str("http://a/b/c/d;p?q"), Str("g;x=1/../y"), Str("http://a/b/c/y")},
        {Str("http://a/b/c/d;p?q"), Str("g?y/./x"), Str("http://a/b/c/g?y/./x")},
        {Str("http://a/b/c/d;p?q"), Str("g?y/../x"), Str("http://a/b/c/g?y/../x")},
        {Str("http://a/b/c/d;p?q"), Str("g#s/./x"), Str("http://a/b/c/g#s/./x")},
        {Str("http://a/b/c/d;p?q"), Str("g#s/../x"), Str("http://a/b/c/g#s/../x")},
        {Str("http://a/b/c/d;p?q"), Str("http:g"), Str("http:g")},
    };

    capy_arena *arena = capy_arena_init(0, KiB(4));

    for (size_t i = 0; i < ArrLen(uris_reference); i++)
    {
        capy_uri base = capy_uri_parse(uris_reference[i].base);
        capy_uri reference = capy_uri_parse(uris_reference[i].reference);
        capy_uri uri = capy_uri_resolve_reference(arena, base, reference);
        capy_string result = capy_uri_string(arena, uri);

        ExpectEqStr(result, uris_reference[i].expected);
    }

    capy_arena_destroy(arena);

    return true;
}

int main(void)
{
    testbench t = {0, 0};

    printf("Running tests...\n\n");

    runtest(&t, test_capy_error, "capy_error");
    runtest(&t, test_capy_buffer_init, "capy_buffer_init");
    runtest(&t, test_buffer_wbytes_enomem, "capy_buffer_write_bytes: should fail when alloc fails");
    runtest(&t, test_buffer_format_enomem, "capy_buffer_write_fmt: should fail when alloc fails");
    runtest(&t, test_buffer_writes, "capy_buffer_(w*|shl|resize|format): should produce expected text");
    runtest(&t, test_capy_base64, "capy_base64");
    runtest(&t, test_capy_string_base64, "capy_string_base64");
    runtest(&t, test_capy_buffer_base64, "capy_buffer_base64");
    runtest(&t, test_capy_arena_init, "capy_arena_init");
    runtest(&t, test_capy_arena_alloc, "capy_arena_alloc");
    runtest(&t, test_capy_arena_free, "capy_arena_free");
    runtest(&t, test_capy_arena_realloc, "capy_arena_realloc");
    runtest(&t, test_capy_http_request_validate, "capy_http_request_validate");
    runtest(&t, test_capy_http_parse_method, "capy_http_parse_method");
    runtest(&t, test_capy_http_parse_version, "capy_http_parse_version");
    runtest(&t, test_capy_http_parse_reqline, "capy_http_parse_reqline");
    runtest(&t, test_capy_http_parse_field, "capy_http_parse_field");
    runtest(&t, test_capy_http_write_response, "capy_http_write_response");
    runtest(&t, test_capy_http_parse_uriparams, "capy_http_parse_uriparams");
    runtest(&t, test_capy_json_serialize, "capy_json_serialize");
    runtest(&t, test_capy_json_deserialize, "capy_json_deserialize");
    runtest(&t, test_capy_string_cstr, "capy_string_cstr");
    runtest(&t, test_capy_string_eq, "capy_string_eq");
    runtest(&t, test_capy_string_slice, "capy_string_(slice|shl|shr)");
    runtest(&t, test_capy_string_copy, "capy_string_copy");
    runtest(&t, test_capy_string_lower, "capy_string_lower");
    runtest(&t, test_capy_string_upper, "capy_string_upper");
    runtest(&t, test_capy_string_join, "capy_string_Join");
    runtest(&t, test_capy_string_trim, "capy_string_(trim|ltrim|rtrim)");
    runtest(&t, test_capy_string_prefix, "capy_string_prefix");
    runtest(&t, test_capy_string_hex, "capy_string_hex");
    runtest(&t, test_capy_strset, "capy_strset_(init|has|add)");
    runtest(&t, test_capy_strkvmap, "capy_strset_(init|get|set)");
    runtest(&t, test_capy_strkvnmap, "capy_strset_(init|get|set|add)");
    runtest(&t, test_capy_vec_insert, "capy_vec_insert");
    runtest(&t, test_capy_vec_delete, "capy_vec_delete");

    // URI
    runtest(&t, test_uri_parse, "capy_uri_parse");
    runtest(&t, test_uri_resolve_reference, "capy_uri_resolve_reference");

    // Tasks
    runtest(&t, test_taskqueue, "taskqueue");

    printf("\nSummary - %d of %d tests succeeded\n", t.succeded, t.succeded + t.failed);

    if (t.failed)
    {
        return 1;
    }

    return 0;
}
