#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// MACROS

#define HTTP_VCHAR 0x1
#define HTTP_TOKEN 0x2
#define HTTP_DIGIT 0x4
#define HTTP_HEXDIGIT 0x8

// STATIC

static const capy_string capy_http_status_string[600] = {
    [CAPY_HTTP_STATUS_UNK] = strli("Unknown"),
    [CAPY_HTTP_CONTINUE] = strli("Continue"),
    [CAPY_HTTP_SWITCHING_PROTOCOLS] = strli("Switching Protocols"),
    [CAPY_HTTP_OK] = strli("Ok"),
    [CAPY_HTTP_CREATED] = strli("Created"),
    [CAPY_HTTP_ACCEPTED] = strli("Accepted"),
    [CAPY_HTTP_NON_AUTHORITATIVE_INFORMATION] = strli("Non-Authoritative Information"),
    [CAPY_HTTP_NO_CONTENT] = strli("No Content"),
    [CAPY_HTTP_RESET_CONTENT] = strli("Reset Content"),
    [CAPY_HTTP_PARTIAL_CONTENT] = strli("Partial Content"),
    [CAPY_HTTP_MULTIPLE_CHOICES] = strli("Multiple Choices"),
    [CAPY_HTTP_MOVED_PERMANENTLY] = strli("Moved Permanently"),
    [CAPY_HTTP_FOUND] = strli("Found"),
    [CAPY_HTTP_SEE_OTHER] = strli("See Other"),
    [CAPY_HTTP_NOT_MODIFIED] = strli("Not Modified"),
    [CAPY_HTTP_USE_PROXY] = strli("Use Proxy"),
    [CAPY_HTTP_TEMPORARY_REDIRECT] = strli("Temporary Redirect"),
    [CAPY_HTTP_PERMANENT_REDIRECT] = strli("Permanent Redirect"),
    [CAPY_HTTP_BAD_REQUEST] = strli("Bad Request"),
    [CAPY_HTTP_UNAUTHORIZED] = strli("Unauthorized"),
    [CAPY_HTTP_PAYMENT_REQUIRED] = strli("Payment Required"),
    [CAPY_HTTP_FORBIDDEN] = strli("Forbidden"),
    [CAPY_HTTP_NOT_FOUND] = strli("Not Found"),
    [CAPY_HTTP_METHOD_NOT_ALLOWED] = strli("Method Not Allowed"),
    [CAPY_HTTP_NOT_ACCEPTABLE] = strli("Not Acceptable"),
    [CAPY_HTTP_PROXY_AUTHENTICATION_REQUIRED] = strli("Proxy Authentication Required"),
    [CAPY_HTTP_REQUEST_TIMEOUT] = strli("Request Timeout"),
    [CAPY_HTTP_CONFLICT] = strli("Conflict"),
    [CAPY_HTTP_GONE] = strli("Gone"),
    [CAPY_HTTP_LENGTH_REQUIRED] = strli("Length Required"),
    [CAPY_HTTP_PRECONDITION_FAILED] = strli("Precondition Failed"),
    [CAPY_HTTP_CONTENT_TOO_LARGE] = strli("Content Too Large"),
    [CAPY_HTTP_URI_TOO_LONG] = strli("Request Uri Too Long"),
    [CAPY_HTTP_UNSUPPORTED_MEDIA_TYPE] = strli("Unsupported Media Type"),
    [CAPY_HTTP_RANGE_NOT_SATISFIABLE] = strli("Range Not Satisfiable"),
    [CAPY_HTTP_EXPECTATION_FAILED] = strli("Expectation Failed"),
    [CAPY_HTTP_IM_A_TEAPOT] = strli("I'm A Teapot"),
    [CAPY_HTTP_MISDIRECTED_REQUEST] = strli("Misdirected Request"),
    [CAPY_HTTP_UNPROCESSABLE_ENTITY] = strli("Unprocessable Entity"),
    [CAPY_HTTP_UPGRADE_REQUIRED] = strli("Upgrade Required"),
    [CAPY_HTTP_INTERNAL_SERVER_ERROR] = strli("Internal Server Error"),
    [CAPY_HTTP_NOT_IMPLEMENTED] = strli("Not Implemented"),
    [CAPY_HTTP_BAD_GATEWAY] = strli("Bad Gateway"),
    [CAPY_HTTP_SERVICE_UNAVAILABLE] = strli("Service Unavailable"),
    [CAPY_HTTP_GATEWAY_TIMEOUT] = strli("Gateway Timeout"),
    [CAPY_HTTP_HTTP_VERSION_NOT_SUPPORTED] = strli("HTTP Version Not Supported"),
};

static uint8_t http_char_categories[256] = {
    ['!'] = HTTP_VCHAR | HTTP_TOKEN,
    ['"'] = HTTP_VCHAR,
    ['#'] = HTTP_VCHAR | HTTP_TOKEN,
    ['$'] = HTTP_VCHAR | HTTP_TOKEN,
    ['%'] = HTTP_VCHAR | HTTP_TOKEN,
    ['&'] = HTTP_VCHAR | HTTP_TOKEN,
    ['\''] = HTTP_VCHAR | HTTP_TOKEN,
    ['('] = HTTP_VCHAR,
    [')'] = HTTP_VCHAR,
    ['*'] = HTTP_VCHAR | HTTP_TOKEN,
    ['+'] = HTTP_VCHAR | HTTP_TOKEN,
    [','] = HTTP_VCHAR,
    ['-'] = HTTP_VCHAR | HTTP_TOKEN,
    ['.'] = HTTP_VCHAR | HTTP_TOKEN,
    ['/'] = HTTP_VCHAR,
    ['0'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    ['1'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    ['2'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    ['3'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    ['4'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    ['5'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    ['6'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    ['7'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    ['8'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    ['9'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_DIGIT | HTTP_HEXDIGIT,
    [':'] = HTTP_VCHAR,
    [';'] = HTTP_VCHAR,
    ['<'] = HTTP_VCHAR,
    ['='] = HTTP_VCHAR,
    ['>'] = HTTP_VCHAR,
    ['?'] = HTTP_VCHAR,
    ['@'] = HTTP_VCHAR,
    ['A'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['B'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['C'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['D'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['E'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['F'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['G'] = HTTP_VCHAR | HTTP_TOKEN,
    ['H'] = HTTP_VCHAR | HTTP_TOKEN,
    ['I'] = HTTP_VCHAR | HTTP_TOKEN,
    ['J'] = HTTP_VCHAR | HTTP_TOKEN,
    ['K'] = HTTP_VCHAR | HTTP_TOKEN,
    ['L'] = HTTP_VCHAR | HTTP_TOKEN,
    ['M'] = HTTP_VCHAR | HTTP_TOKEN,
    ['N'] = HTTP_VCHAR | HTTP_TOKEN,
    ['O'] = HTTP_VCHAR | HTTP_TOKEN,
    ['P'] = HTTP_VCHAR | HTTP_TOKEN,
    ['Q'] = HTTP_VCHAR | HTTP_TOKEN,
    ['R'] = HTTP_VCHAR | HTTP_TOKEN,
    ['S'] = HTTP_VCHAR | HTTP_TOKEN,
    ['T'] = HTTP_VCHAR | HTTP_TOKEN,
    ['U'] = HTTP_VCHAR | HTTP_TOKEN,
    ['V'] = HTTP_VCHAR | HTTP_TOKEN,
    ['W'] = HTTP_VCHAR | HTTP_TOKEN,
    ['X'] = HTTP_VCHAR | HTTP_TOKEN,
    ['Y'] = HTTP_VCHAR | HTTP_TOKEN,
    ['Z'] = HTTP_VCHAR | HTTP_TOKEN,
    ['['] = HTTP_VCHAR,
    ['\\'] = HTTP_VCHAR,
    [']'] = HTTP_VCHAR,
    ['^'] = HTTP_VCHAR | HTTP_TOKEN,
    ['_'] = HTTP_VCHAR | HTTP_TOKEN,
    ['`'] = HTTP_VCHAR | HTTP_TOKEN,
    ['a'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['b'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['c'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['d'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['e'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['f'] = HTTP_VCHAR | HTTP_TOKEN | HTTP_HEXDIGIT,
    ['g'] = HTTP_VCHAR | HTTP_TOKEN,
    ['h'] = HTTP_VCHAR | HTTP_TOKEN,
    ['i'] = HTTP_VCHAR | HTTP_TOKEN,
    ['j'] = HTTP_VCHAR | HTTP_TOKEN,
    ['k'] = HTTP_VCHAR | HTTP_TOKEN,
    ['l'] = HTTP_VCHAR | HTTP_TOKEN,
    ['m'] = HTTP_VCHAR | HTTP_TOKEN,
    ['n'] = HTTP_VCHAR | HTTP_TOKEN,
    ['o'] = HTTP_VCHAR | HTTP_TOKEN,
    ['p'] = HTTP_VCHAR | HTTP_TOKEN,
    ['q'] = HTTP_VCHAR | HTTP_TOKEN,
    ['r'] = HTTP_VCHAR | HTTP_TOKEN,
    ['s'] = HTTP_VCHAR | HTTP_TOKEN,
    ['t'] = HTTP_VCHAR | HTTP_TOKEN,
    ['u'] = HTTP_VCHAR | HTTP_TOKEN,
    ['v'] = HTTP_VCHAR | HTTP_TOKEN,
    ['w'] = HTTP_VCHAR | HTTP_TOKEN,
    ['x'] = HTTP_VCHAR | HTTP_TOKEN,
    ['y'] = HTTP_VCHAR | HTTP_TOKEN,
    ['z'] = HTTP_VCHAR | HTTP_TOKEN,
    ['{'] = HTTP_VCHAR,
    ['|'] = HTTP_VCHAR | HTTP_TOKEN,
    ['}'] = HTTP_VCHAR,
    ['~'] = HTTP_VCHAR | HTTP_TOKEN,
};

static char http_hexdecode[256] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10,
    ['b'] = 11,
    ['c'] = 12,
    ['d'] = 13,
    ['e'] = 14,
    ['f'] = 15,
    ['A'] = 10,
    ['B'] = 11,
    ['C'] = 12,
    ['D'] = 13,
    ['E'] = 14,
    ['F'] = 15,
};

static const char *http_weekday[] = {
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    "Sun",
};

static const char *http_months[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
};

// STATIC DEFINITIONS

static bool http_string_validate(capy_string s, int categories, const char *chars)
{
    capy_string input = s;

    while (input.size)
    {
        if (!(http_char_categories[(uint8_t)(input.data[0])] & categories) && !capy_char_is(input.data[0], chars))
        {
            return false;
        }

        input = capy_string_shl(input, 1);
    }

    return true;
}

static capy_string http_next_token(capy_string *buffer, const char *delimiters)
{
    capy_string input = *buffer;

    size_t i;

    for (i = 0; i < input.size; i++)
    {
        if (capy_char_is(input.data[i], delimiters))
        {
            break;
        }
    }

    *buffer = capy_string_shl(input, i);

    return capy_string_slice(input, 0, i);
}

static size_t http_consume_chars(capy_string *buffer, const char *chars, size_t limit)
{
    capy_string input = *buffer;

    size_t i;

    if (limit == 0 || limit > input.size)
    {
        limit = input.size;
    }

    for (i = 0; i < limit && capy_char_is(input.data[i], chars); i++)
    {
    }

    *buffer = capy_string_shl(input, i);

    return i;
}

static capy_err http_canonical_field_name(capy_arena *arena, capy_string *output, capy_string input)
{
    char *data = make(arena, char, input.size + 1);

    if (data == NULL)
    {
        return capy_errno(ENOMEM);
    }

    int upper = 1;

    for (size_t i = 0; i < input.size; i++)
    {
        char c = input.data[i];

        if (c == '-')
        {
            upper = 1;
            data[i] = '-';
        }
        else if (upper)
        {
            data[i] = capy_char_uppercase(c);
            upper = 0;
        }
        else
        {
            data[i] = capy_char_lowercase(c);
        }
    }

    *output = capy_string_bytes(input.size, data);

    return ok;
}

static capy_http_router_map *capy_http_router_map_init(capy_arena *arena, size_t capacity)
{
    capy_http_router_map *map = capy_arena_alloc(arena, sizeof(capy_http_router_map) + (sizeof(capy_http_router) * capacity), 8, true);

    if (map == NULL)
    {
        return NULL;
    }

    map->size = 0;
    map->capacity = capacity;
    map->items = recast(capy_http_router *, map + 1);

    return map;
}

static capy_http_router *capy_http_router_map_get(capy_http_router_map *map, capy_string key)
{
    return capy_strmap_get(map->items, sizeof(capy_http_router), map->capacity, key);
}

static must_check capy_err capy_http_router_map_set(capy_arena *arena, capy_http_router_map *map, capy_http_router router)
{
    void *items = capy_strmap_set(arena, map->items, sizeof(capy_http_router), &map->capacity, &map->size, &router);

    if (items == NULL)
    {
        return capy_errno(ENOMEM);
    }

    map->items = items;

    return ok;
}

static capy_http_router *http_route_add_(capy_arena *arena, capy_http_router *router, capy_http_method method, capy_string suffix, capy_string path, capy_http_handler handler)
{
    if (router == NULL)
    {
        router = make(arena, capy_http_router, 1);

        if (router == NULL)
        {
            return NULL;
        }

        router->segments = capy_http_router_map_init(arena, 4);

        if (router->segments == NULL)
        {
            return NULL;
        }
    }

    http_consume_chars(&suffix, "/", 0);
    capy_string segment = http_next_token(&suffix, "/");

    if (segment.size == 0)
    {
        router->routes[method] = (capy_http_route){.method = method, .path = path, .handler = handler};
        return router;
    }

    if (segment.data[0] == '^')
    {
        segment = strl("^");
    }

    capy_http_router *child = capy_http_router_map_get(router->segments, segment);

    child = http_route_add_(arena, child, method, suffix, path, handler);

    if (child == NULL)
    {
        return NULL;
    }

    child->segment = segment;

    if (capy_http_router_map_set(arena, router->segments, *child).code)
    {
        return NULL;
    }

    return router;
}

static capy_err http_response_status(capy_http_response *response)
{
    capy_err err;

    capy_string status_text = capy_http_status_string[response->status];

    if ((err = capy_strkvmmap_add(response->headers, strl("Content-Type"), strl("text/plain; charset=UTF-8"))).code)
    {
        return err;
    }

    if ((err = capy_buffer_format(response->body, 0, "%lu %s\n", response->status, status_text)).code)
    {
        return err;
    }

    return ok;
}

static capy_err http_response_wcstr(capy_http_response *response, const char *msg)
{
    capy_err err;

    if ((err = capy_strkvmmap_add(response->headers, strl("Content-Type"), strl("text/plain; charset=UTF-8"))).code)
    {
        return err;
    }

    if ((err = capy_buffer_format(response->body, 0, "%s\n", msg)).code)
    {
        return err;
    }

    return ok;
}

static capy_err http_query_pctdecode(capy_arena *arena, capy_string *output, capy_string input)
{
    if (input.size == 0)
    {
        *output = (capy_string){.size = 0};
        return ok;
    }

    char *bytes = make(arena, char, output->size + 1);

    if (bytes == NULL)
    {
        return capy_errno(ENOMEM);
    }

    size_t j = 0;

    for (size_t i = 0; i < input.size; i++, j++)
    {
        if (input.data[i] == '%')
        {
            int v = http_hexdecode[cast(unsigned char, input.data[i + 1])] << 4 |
                    http_hexdecode[cast(unsigned char, input.data[i + 2])];

            bytes[j] = cast(char, v);
            i += 2;
        }
        else if (input.data[i] == '+')
        {
            bytes[j] = ' ';
        }
        else
        {
            bytes[j] = input.data[i];
        }
    }

    *output = capy_string_bytes(j, bytes);

    return ok;
}

// DEFINITIONS

capy_http_method capy_http_parse_method(capy_string input)
{
    switch (input.size)
    {
        case 3:
        {
            if (arrcmp3(input.data, 'G', 'E', 'T'))
            {
                return CAPY_HTTP_GET;
            }
            else if (arrcmp3(input.data, 'P', 'U', 'T'))
            {
                return CAPY_HTTP_PUT;
            }
        }
        break;

        case 4:
        {
            if (arrcmp4(input.data, 'H', 'E', 'A', 'D'))
            {
                return CAPY_HTTP_HEAD;
            }
            else if (arrcmp4(input.data, 'P', 'O', 'S', 'T'))
            {
                return CAPY_HTTP_POST;
            }
        }
        break;

        case 5:
        {
            if (arrcmp5(input.data, 'T', 'R', 'A', 'C', 'E'))
            {
                return CAPY_HTTP_TRACE;
            }
            else if (arrcmp5(input.data, 'P', 'A', 'T', 'C', 'H'))
            {
                return CAPY_HTTP_PATCH;
            }
        }
        break;

        case 6:
        {
            if (arrcmp6(input.data, 'D', 'E', 'L', 'E', 'T', 'E'))
            {
                return CAPY_HTTP_DELETE;
            }
        }
        break;

        case 7:
        {
            if (arrcmp7(input.data, 'C', 'O', 'N', 'N', 'E', 'C', 'T'))
            {
                return CAPY_HTTP_CONNECT;
            }
            else if (arrcmp7(input.data, 'O', 'P', 'T', 'I', 'O', 'N', 'S'))
            {
                return CAPY_HTTP_OPTIONS;
            }
        }
        break;
    }

    return CAPY_HTTP_METHOD_UNK;
}

capy_http_version capy_http_parse_version(capy_string input)
{
    if (input.size != 8)
    {
        return CAPY_HTTP_VERSION_UNK;
    }

    if (!arrcmp5(input.data, 'H', 'T', 'T', 'P', '/'))
    {
        return CAPY_HTTP_VERSION_UNK;
    }

    if (input.data[6] != '.')
    {
        return CAPY_HTTP_VERSION_UNK;
    }

    switch (input.data[5])
    {
        case '0':
        {
            if (input.data[7] == '9')
            {
                return CAPY_HTTP_09;
            }
        }
        break;

        case '1':
        {
            switch (input.data[7])
            {
                case '0':
                    return CAPY_HTTP_10;
                case '1':
                    return CAPY_HTTP_11;
            }
        }
        break;

        case '2':
        {
            if (input.data[7] == '0')
            {
                return CAPY_HTTP_20;
            }
        }
        break;

        case '3':
        {
            if (input.data[7] == '0')
            {
                return CAPY_HTTP_30;
            }
        }
        break;
    }

    return CAPY_HTTP_VERSION_UNK;
}

capy_err capy_http_write_response(capy_buffer *buffer, capy_http_response *response, int close)
{
    capy_err err;

    time_t t = time(NULL);
    struct tm ct;
    gmtime_r(&t, &ct);

    size_t content_length = (response->body) ? response->body->size : 0;
    const char *close_header = (close) ? "Connection: close\r\n" : "";

    err = capy_buffer_format(buffer, 0,
                             "HTTP/1.1 %d %s\r\n"
                             "Date: %s, %02d %s %04d %02d:%02d:%02d GMT\r\n"
                             "Content-Length: %llu\r\n"
                             "%s",
                             response->status, capy_http_status_string[response->status].data,
                             http_weekday[ct.tm_wday],
                             ct.tm_mday, http_months[ct.tm_mon], ct.tm_year + 1900,
                             ct.tm_hour, ct.tm_min, ct.tm_sec,
                             content_length, close_header);

    if (err.code)
    {
        return err;
    }

    for (size_t i = 0; i < response->headers->capacity; i++)
    {
        capy_strkvn *header = response->headers->items + i;

        if (header->key.size == 0)
        {
            continue;
        }

        while (header != NULL)
        {
            if ((err = capy_buffer_format(buffer, 0, "%s: %s\r\n", header->key.data, header->value.data)).code)
            {
                return err;
            }

            header = header->next;
        }
    }

    if ((err = capy_buffer_wbytes(buffer, 2, "\r\n")).code)
    {
        return err;
    }

    return capy_buffer_wbytes(buffer, response->body->size, response->body->data);
}

capy_err capy_http_parse_reqline(capy_arena *arena, capy_http_request *request, capy_string line)
{
    capy_err err;

    capy_string method = http_next_token(&line, " ");

    if (http_consume_chars(&line, " ", 0) != 1)
    {
        return capy_errno(EINVAL);
    }

    capy_string uri = http_next_token(&line, " ");

    request->uri_raw = uri;

    if (http_consume_chars(&line, " ", 0) != 1)
    {
        return capy_errno(EINVAL);
    }

    capy_string version = http_next_token(&line, " ");

    if (line.size != 0)
    {
        return capy_errno(EINVAL);
    }

    request->method = capy_http_parse_method(method);

    if (request->method == CAPY_HTTP_METHOD_UNK)
    {
        return capy_errno(EINVAL);
    }

    request->version = capy_http_parse_version(version);

    if (request->version == CAPY_HTTP_VERSION_UNK)
    {
        return capy_errno(EINVAL);
    }

    if (request->method == CAPY_HTTP_CONNECT)
    {
        request->uri = (capy_uri){.authority = uri};
        request->uri = capy_uri_parse_authority(request->uri);

        if (request->uri.userinfo.size)
        {
            return capy_errno(EINVAL);
        }
    }
    else if (request->method == CAPY_HTTP_OPTIONS && capy_string_eq(uri, strl("*")))
    {
        request->uri = (capy_uri){.path = uri};
    }
    else
    {
        request->uri = capy_uri_parse(uri);
    }

    if (!capy_uri_valid(request->uri))
    {
        return capy_errno(EINVAL);
    }

    if ((err = capy_string_copy(arena, &request->uri_raw, request->uri_raw)).code)
    {
        return err;
    }

    if ((err = capy_uri_normalize(arena, &request->uri.scheme, request->uri.scheme, true)).code)
    {
        return err;
    }

    if ((err = capy_uri_normalize(arena, &request->uri.authority, request->uri.authority, false)).code)
    {
        return err;
    }

    if ((err = capy_uri_normalize(arena, &request->uri.userinfo, request->uri.userinfo, false)).code)
    {
        return err;
    }

    if ((err = capy_uri_normalize(arena, &request->uri.host, request->uri.host, true)).code)
    {
        return err;
    }

    if ((err = capy_uri_normalize(arena, &request->uri.port, request->uri.port, false)).code)
    {
        return err;
    }

    if ((err = capy_uri_normalize(arena, &request->uri.path, request->uri.path, false)).code)
    {
        return err;
    }

    if ((err = capy_uri_normalize(arena, &request->uri.query, request->uri.query, false)).code)
    {
        return err;
    }

    request->uri.fragment = (capy_string){.size = 0};

    return ok;
}

capy_err capy_http_parse_query(capy_strkvmmap *fields, capy_string query)
{
    capy_err err;

    while (query.size)
    {
        capy_string item = http_next_token(&query, "&");

        http_consume_chars(&query, "&", 1);

        if (item.size == 0)
        {
            continue;
        }

        capy_string name = http_next_token(&item, "=");

        http_consume_chars(&item, "=", 1);

        if (name.size == 0)
        {
            continue;
        }

        capy_string value = item;

        if ((err = http_query_pctdecode(fields->arena, &name, name)).code)
        {
            return err;
        }

        if ((err = http_query_pctdecode(fields->arena, &value, value)).code)
        {
            return err;
        }

        if ((err = capy_strkvmmap_add(fields, name, value)).code)
        {
            return err;
        }
    }

    return ok;
}

capy_err capy_http_request_validate(capy_arena *arena, capy_http_request *request)
{
    capy_err err;

    request->content_length = 0;
    request->chunked = 0;

    switch (request->version)
    {
        case CAPY_HTTP_10:
            request->close = true;
            break;

        case CAPY_HTTP_11:
            request->close = false;
            break;

        default:
            return capy_errno(EINVAL);
    }

    capy_strkvn *host = capy_strkvmmap_get(request->headers, strl("Host"));

    if (host == NULL || host->next != NULL)
    {
        return capy_errno(EINVAL);
    }

    if ((request->uri.flags & CAPY_URI_AUTHORITY) && request->uri.authority.size == 0)
    {
        return capy_errno(EINVAL);
    }

    if (request->uri.scheme.size == 0)
    {
        request->uri.scheme = strl("http");
    }

    if (!capy_string_eq(request->uri.scheme, strl("http")))
    {
        return capy_errno(EINVAL);
    }

    if (request->method != CAPY_HTTP_CONNECT)
    {
        if (request->uri.authority.size == 0)
        {
            request->uri.authority = host->value;
            request->uri = capy_uri_parse_authority(request->uri);

            if (!capy_uri_valid(request->uri))
            {
                return capy_errno(EINVAL);
            }
        }
    }

    if (request->uri.port.size == 0)
    {
        request->uri.port = strl("80");
    }

    if (request->uri.authority.size == 0)
    {
        return capy_errno(EINVAL);
    }

    if ((err = capy_string_join(arena, &host->value, ":", 2, (capy_string[]){request->uri.host, request->uri.port})).code)
    {
        return err;
    }

    capy_strkvn *transfer_encoding = capy_strkvmmap_get(request->headers, strl("Transfer-Encoding"));

    if (transfer_encoding != NULL)
    {
        if (transfer_encoding->next || !capy_string_eq(transfer_encoding->value, strl("chunked")))
        {
            return capy_errno(EINVAL);
        }

        request->chunked = true;
        request->content_length = 0;
    }

    capy_strkvn *content_length = capy_strkvmmap_get(request->headers, strl("Content-Length"));

    if (content_length != NULL)
    {
        if (request->chunked || content_length->next || !http_string_validate(content_length->value, HTTP_DIGIT, ""))
        {
            return capy_errno(EINVAL);
        }

        request->content_length = strtoull(content_length->value.data, NULL, 10);
        request->chunked = 0;
    }

    capy_strkvn *connection = capy_strkvmmap_get(request->headers, strl("Connection"));

    if (connection != NULL)
    {
        if (connection->next != NULL)
        {
            return capy_errno(EINVAL);
        }

        if (capy_string_eq(connection->value, strl("close")))
        {
            request->close = true;
        }
        else if (capy_string_eq(connection->value, strl("keep-alive")))
        {
            request->close = false;
        }
        else
        {
            return capy_errno(EINVAL);
        }
    }

    if ((err = capy_http_parse_query(request->query, request->uri.query)).code)
    {
        return err;
    }

    return ok;
}

capy_err capy_http_parse_field(capy_strkvmmap *fields, capy_string line)
{
    capy_err err;

    capy_string name = http_next_token(&line, ":");

    if (name.size == 0 || !http_string_validate(name, HTTP_TOKEN, NULL))
    {
        return capy_errno(EINVAL);
    }

    if (http_consume_chars(&line, ":", 0) != 1)
    {
        return capy_errno(EINVAL);
    }

    capy_string value = capy_string_trim(line, " \t");

    if (!http_string_validate(value, HTTP_VCHAR, " \t"))
    {
        return capy_errno(EINVAL);
    }

    if ((err = http_canonical_field_name(fields->arena, &name, name)).code)
    {
        return err;
    }

    if ((err = capy_string_copy(fields->arena, &value, value)).code)
    {
        return err;
    }

    return capy_strkvmmap_add(fields, name, value);
}

capy_http_router *capy_http_router_init(capy_arena *arena, int n, capy_http_route *routes)
{
    capy_http_router *router = NULL;

    for (int i = 0; i < n; i++)
    {
        router = http_route_add_(arena, router, routes[i].method, routes[i].path, routes[i].path, routes[i].handler);

        if (router == NULL)
        {
            return NULL;
        }
    }

    return router;
}

capy_http_route *capy_http_route_get(capy_http_router *router, capy_http_method method, capy_string path)
{
    http_consume_chars(&path, "/", 0);
    capy_string segment = http_next_token(&path, "/");

    if (segment.size == 0)
    {
        capy_http_route *route = router->routes + method;

        if (route->handler == NULL)
        {
            return NULL;
        }

        return route;
    }

    capy_http_router *child = capy_http_router_map_get(router->segments, segment);

    if (child == NULL)
    {
        child = capy_http_router_map_get(router->segments, strl("^"));

        if (child == NULL)
        {
            return NULL;
        }
    }

    return capy_http_route_get(child, method, path);
}

capy_err capy_http_parse_uriparams(capy_strkvmmap *params, capy_string path, capy_string handler_path)
{
    capy_err err;

    for (;;)
    {
        http_consume_chars(&path, "/", 0);
        capy_string path_segment = http_next_token(&path, "/");

        if (path_segment.size == 0)
        {
            break;
        }

        http_consume_chars(&handler_path, "/", 0);
        capy_string handler_path_segment = http_next_token(&handler_path, "/");

        if (handler_path_segment.data[0] != '^')
        {
            continue;
        }

        handler_path_segment = capy_string_shl(handler_path_segment, 1);

        if ((err = capy_strkvmmap_add(params, handler_path_segment, path_segment)).code)
        {
            return err;
        }
    }

    return ok;
}

capy_err capy_http_router_handle(capy_arena *arena, capy_http_router *router, capy_http_request *request, capy_http_response *response)
{
    capy_err err;

    capy_http_route *route = capy_http_route_get(router, request->method, request->uri.path);

    if (route == NULL)
    {
        response->status = CAPY_HTTP_NOT_FOUND;
        return http_response_status(response);
    }

    if ((err = capy_http_parse_uriparams(request->params, request->uri.path, route->path)).code)
    {
        return errwrap(err, "Failed to parse uri params");
    }

    if ((err = capy_http_parse_query(request->params, request->uri.query)).code)
    {
        return errwrap(err, "Failed to parse query params");
    }

    if ((err = route->handler(arena, request, response)).code)
    {
        response->status = CAPY_HTTP_INTERNAL_SERVER_ERROR;
        return http_response_wcstr(response, err.msg);
    }

    return ok;
}
