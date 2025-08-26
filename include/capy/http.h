#ifndef CAPY_HTTP_H
#define CAPY_HTTP_H

#include <capy/std.h>
#include <capy/uri.h>

typedef enum capy_http_method
{
    CAPY_HTTP_METHOD_UNK = 0,
    CAPY_HTTP_METHOD_GET,
    CAPY_HTTP_METHOD_HEAD,
    CAPY_HTTP_METHOD_POST,
    CAPY_HTTP_METHOD_PUT,
    CAPY_HTTP_METHOD_DELETE,
    CAPY_HTTP_METHOD_CONNECT,
    CAPY_HTTP_METHOD_OPTIONS,
    CAPY_HTTP_METHOD_TRACE,
} capy_http_method;

typedef enum capy_http_version
{
    CAPY_HTTP_VERSION_UNK = 0,
    CAPY_HTTP_VERSION_09,
    CAPY_HTTP_VERSION_10,
    CAPY_HTTP_VERSION_11,
    CAPY_HTTP_VERSION_20,
    CAPY_HTTP_VERSION_30,
} capy_http_version;

typedef struct capy_http_field
{
    capy_string name;
    capy_string value;
} capy_http_field;

typedef struct capy_http_request
{
    capy_http_method method;
    capy_http_version version;
    capy_uri uri;
    capy_http_field *headers;
    uint8_t body[];
} capy_http_request;

capy_http_request *capy_http_parse_header(capy_arena *arena, capy_string input, size_t request_line_limit);

#endif
