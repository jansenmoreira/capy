#ifndef CAPY_HTTP_H
#define CAPY_HTTP_H

#include <capy/std.h>
#include <capy/uri.h>

typedef enum capy_http_status
{
    CAPY_HTTP_STATUS_UNK = 0,
    CAPY_HTTP_CONTINUE = 100,
    CAPY_HTTP_SWITCHING_PROTOCOLS = 101,
    CAPY_HTTP_PROCESSING = 102,
    CAPY_HTTP_OK = 200,
    CAPY_HTTP_CREATED = 201,
    CAPY_HTTP_ACCEPTED = 202,
    CAPY_HTTP_NON_AUTHORITATIVE_INFORMATION = 203,
    CAPY_HTTP_NO_CONTENT = 204,
    CAPY_HTTP_RESET_CONTENT = 205,
    CAPY_HTTP_PARTIAL_CONTENT = 206,
    CAPY_HTTP_MULTI_STATUS = 207,
    CAPY_HTTP_ALREADY_REPORTED = 208,
    CAPY_HTTP_IM_USED = 226,
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
    CAPY_HTTP_PAYLOAD_TOO_LARGE = 413,
    CAPY_HTTP_REQUEST_URI_TOO_LONG = 414,
    CAPY_HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
    CAPY_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
    CAPY_HTTP_EXPECTATION_FAILED = 417,
    CAPY_HTTP_IM_A_TEAPOT = 418,
    CAPY_HTTP_MISDIRECTED_REQUEST = 421,
    CAPY_HTTP_UNPROCESSABLE_ENTITY = 422,
    CAPY_HTTP_LOCKED = 423,
    CAPY_HTTP_FAILED_DEPENDENCY = 424,
    CAPY_HTTP_UPGRADE_REQUIRED = 426,
    CAPY_HTTP_PRECONDITION_REQUIRED = 428,
    CAPY_HTTP_TOO_MANY_REQUESTS = 429,
    CAPY_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    CAPY_HTTP_CONNECTION_CLOSED_WITHOUT_RESPONSE = 444,
    CAPY_HTTP_UNAVAILABLE_FOR_LEGAL_REASONS = 451,
    CAPY_HTTP_CLIENT_CLOSED_REQUEST = 499,
    CAPY_HTTP_INTERNAL_SERVER_ERROR = 500,
    CAPY_HTTP_NOT_IMPLEMENTED = 501,
    CAPY_HTTP_BAD_GATEWAY = 502,
    CAPY_HTTP_SERVICE_UNAVAILABLE = 503,
    CAPY_HTTP_GATEWAY_TIMEOUT = 504,
    CAPY_HTTP_HTTP_VERSION_NOT_SUPPORTED = 505,
    CAPY_HTTP_VARIANT_ALSO_NEGOCIATES = 506,
    CAPY_HTTP_INSUFFICIENT_STORAGE = 507,
    CAPY_HTTP_LOOP_DETECTED = 508,
    CAPY_HTTP_NOT_EXTENDED = 510,
    CAPY_HTTP_NETWORK_AUTHENTICATION_REQUIRED = 511,
    CAPY_HTTP_NETWORK_CONNECTION_TIMEOUT_ERROR = 599,
} capy_http_status;

typedef enum capy_http_method
{
    CAPY_HTTP_METHOD_UNK = 0,
    CAPY_HTTP_GET,
    CAPY_HTTP_HEAD,
    CAPY_HTTP_POST,
    CAPY_HTTP_PUT,
    CAPY_HTTP_DELETE,
    CAPY_HTTP_CONNECT,
    CAPY_HTTP_OPTIONS,
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

typedef struct capy_http_field
{
    capy_string name;
    capy_string value;
    struct capy_http_field *next;
} capy_http_field;

typedef struct capy_http_request
{
    capy_http_method method;
    capy_http_version version;
    capy_uri uri;
    capy_http_field *headers;
    capy_http_field *trailers;
    const char *content;
    size_t content_length;
    int chunked;
} capy_http_request;

typedef struct capy_http_response
{
    capy_http_field *headers;
    capy_string reason;
    capy_string content;
    capy_http_status status;
} capy_http_response;

capy_http_method capy_http_parse_method(capy_string input);
capy_http_version capy_http_parse_version(capy_string input);

int capy_http_parse_request_line(capy_arena *arena, capy_http_request *request, capy_string input);
int capy_http_request_validate(capy_arena *arena, capy_http_request *request);
int capy_http_parse_field(capy_arena *arena, capy_http_field **fields, capy_string line);

capy_string capy_http_write_headers(capy_arena *arena, capy_http_response *response);

typedef int(capy_http_handler)(capy_arena *arena, capy_http_request *request, capy_http_response *response);

int capy_http_serve(const char *host, const char *port, size_t workers_count, capy_http_handler handler);

#endif
