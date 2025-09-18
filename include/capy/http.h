#ifndef CAPY_HTTP_H
#define CAPY_HTTP_H

#include <capy/buffer.h>
#include <capy/std.h>
#include <capy/strmap.h>
#include <capy/uri.h>

// TYPES

typedef enum capy_http_status
{
    CAPY_HTTP_STATUS_UNK = 0,
    CAPY_HTTP_CONTINUE = 100,
    CAPY_HTTP_SWITCHING_PROTOCOLS = 101,
    CAPY_HTTP_OK = 200,
    CAPY_HTTP_CREATED = 201,
    CAPY_HTTP_ACCEPTED = 202,
    CAPY_HTTP_NON_AUTHORITATIVE_INFORMATION = 203,
    CAPY_HTTP_NO_CONTENT = 204,
    CAPY_HTTP_RESET_CONTENT = 205,
    CAPY_HTTP_PARTIAL_CONTENT = 206,
    CAPY_HTTP_MULTIPLE_CHOICES = 300,
    CAPY_HTTP_MOVED_PERMANENTLY = 301,
    CAPY_HTTP_FOUND = 302,
    CAPY_HTTP_SEE_OTHER = 303,
    CAPY_HTTP_NOT_MODIFIED = 304,
    CAPY_HTTP_USE_PROXY = 305,
    CAPY_HTTP_TEMPORARY_REDIRECT = 307,
    CAPY_HTTP_PERMANENT_REDIRECT = 308,
    CAPY_HTTP_BAD_REQUEST = 400,
    CAPY_HTTP_UNAUTHORIZED = 401,
    CAPY_HTTP_PAYMENT_REQUIRED = 402,
    CAPY_HTTP_FORBIDDEN = 403,
    CAPY_HTTP_NOT_FOUND = 404,
    CAPY_HTTP_METHOD_NOT_ALLOWED = 405,
    CAPY_HTTP_NOT_ACCEPTABLE = 406,
    CAPY_HTTP_PROXY_AUTHENTICATION_REQUIRED = 407,
    CAPY_HTTP_REQUEST_TIMEOUT = 408,
    CAPY_HTTP_CONFLICT = 409,
    CAPY_HTTP_GONE = 410,
    CAPY_HTTP_LENGTH_REQUIRED = 411,
    CAPY_HTTP_PRECONDITION_FAILED = 412,
    CAPY_HTTP_CONTENT_TOO_LARGE = 413,
    CAPY_HTTP_URI_TOO_LONG = 414,
    CAPY_HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
    CAPY_HTTP_RANGE_NOT_SATISFIABLE = 416,
    CAPY_HTTP_EXPECTATION_FAILED = 417,
    CAPY_HTTP_IM_A_TEAPOT = 418,
    CAPY_HTTP_MISDIRECTED_REQUEST = 421,
    CAPY_HTTP_UNPROCESSABLE_ENTITY = 422,
    CAPY_HTTP_UPGRADE_REQUIRED = 426,
    CAPY_HTTP_INTERNAL_SERVER_ERROR = 500,
    CAPY_HTTP_NOT_IMPLEMENTED = 501,
    CAPY_HTTP_BAD_GATEWAY = 502,
    CAPY_HTTP_SERVICE_UNAVAILABLE = 503,
    CAPY_HTTP_GATEWAY_TIMEOUT = 504,
    CAPY_HTTP_HTTP_VERSION_NOT_SUPPORTED = 505,
} capy_http_status;

typedef enum capy_http_method
{
    CAPY_HTTP_METHOD_UNK = 0,
    CAPY_HTTP_CONNECT,
    CAPY_HTTP_DELETE,
    CAPY_HTTP_GET,
    CAPY_HTTP_HEAD,
    CAPY_HTTP_OPTIONS,
    CAPY_HTTP_PATCH,
    CAPY_HTTP_POST,
    CAPY_HTTP_PUT,
    CAPY_HTTP_TRACE,
} capy_http_method;

typedef enum capy_http_version
{
    CAPY_HTTP_VERSION_UNK = 0,
    CAPY_HTTP_09,
    CAPY_HTTP_10,
    CAPY_HTTP_11,
    CAPY_HTTP_20,
    CAPY_HTTP_30,
} capy_http_version;

typedef struct capy_http_request
{
    capy_http_method method;
    capy_http_version version;
    capy_uri uri;
    capy_string uri_raw;
    capy_strkvmmap *headers;
    capy_strkvmmap *trailers;
    capy_strkvmmap *params;
    capy_strkvmmap *query;
    const char *content;
    size_t content_length;
    int chunked;
    int close;
} capy_http_request;

typedef struct capy_http_response
{
    capy_http_status status;
    capy_strkvmmap *headers;
    capy_buffer *body;
} capy_http_response;

typedef capy_err (*capy_http_handler)(capy_arena *arena, capy_http_request *request, capy_http_response *response);

typedef struct capy_http_route
{
    capy_http_method method;
    capy_string path;
    capy_http_handler handler;
} capy_http_route;

typedef struct capy_http_router_map
{
    size_t size;
    size_t capacity;
    struct capy_http_router *items;
} capy_http_router_map;

typedef struct capy_http_router
{
    capy_string segment;
    capy_http_router_map *segments;
    capy_http_route routes[10];
} capy_http_router;

typedef struct capy_http_server_options
{
    const char *host;
    const char *port;

    int routes_size;

    capy_http_route *routes;

    size_t workers;
    size_t connections;

    size_t line_buffer_size;
    size_t mem_connection_max;
} capy_http_server_options;

// DECLARATIONS

capy_http_method capy_http_parse_method(capy_string input);
capy_http_version capy_http_parse_version(capy_string input);

must_check capy_err capy_http_parse_reqline(capy_arena *arena, capy_http_request *request, capy_string input);
must_check capy_err capy_http_parse_field(capy_strkvmmap *fields, capy_string line);
must_check capy_err capy_http_parse_uriparams(capy_strkvmmap *params, capy_string path, capy_string handler_path);
must_check capy_err capy_http_parse_query(capy_strkvmmap *fields, capy_string line);
must_check capy_err capy_http_request_validate(capy_arena *arena, capy_http_request *request);
must_check capy_err capy_http_write_response(capy_buffer *buffer, capy_http_response *response, int close);

capy_err capy_http_serve(capy_http_server_options options);

capy_http_router *capy_http_router_init(capy_arena *arena, int n, capy_http_route *routes);
capy_http_route *capy_http_route_get(capy_http_router *router, capy_http_method method, capy_string path);

capy_err capy_http_router_handle(capy_arena *arena, capy_http_router *router, capy_http_request *request, capy_http_response *response);

#endif
