#include "capy.h"

enum
{
    HTTP_VCHAR = 1 << 0,
    HTTP_TOKEN = 1 << 1,
    HTTP_DIGIT = 1 << 2,
    HTTP_HEXDIGIT = 1 << 3
};

// INTERNAL VARIABLES

static const char *httpconnstate_cstr[] = {
    [STATE_UNKNOWN] = "STATE_UNKNOWN",
    [STATE_RESET] = "STATE_RESET",
    [STATE_READ_REQUEST] = "STATE_READ_REQUEST",
    [STATE_PARSE_REQLINE] = "STATE_PARSE_REQLINE",
    [STATE_PARSE_HEADERS] = "STATE_PARSE_HEADERS",
    [STATE_PARSE_CONTENT] = "STATE_PARSE_CONTENT",
    [STATE_PARSE_CHUNKSIZE] = "STATE_PARSE_CHUNKSIZE",
    [STATE_PARSE_CHUNKDATA] = "STATE_PARSE_CHUNKDATA",
    [STATE_PARSE_TRAILERS] = "STATE_PARSE_TRAILERS",
    [STATE_ROUTE_REQUEST] = "STATE_ROUTE_REQUEST",
    [STATE_WRITE_RESPONSE] = "STATE_WRITE_RESPONSE",
    [STATE_BAD_REQUEST] = "STATE_BAD_REQUEST",
    [STATE_SERVER_FAILURE] = "STATE_SERVER_FAILURE",
    [STATE_CLOSE] = "STATE_CLOSE",
};

static const capy_string http_status_string[600] = {
    [CAPY_HTTP_STATUS_UNK] = StrIni("Unknown"),
    [CAPY_HTTP_CONTINUE] = StrIni("Continue"),
    [CAPY_HTTP_SWITCHING_PROTOCOLS] = StrIni("Switching Protocols"),
    [CAPY_HTTP_OK] = StrIni("Ok"),
    [CAPY_HTTP_CREATED] = StrIni("Created"),
    [CAPY_HTTP_ACCEPTED] = StrIni("Accepted"),
    [CAPY_HTTP_NON_AUTHORITATIVE_INFORMATION] = StrIni("Non-Authoritative Information"),
    [CAPY_HTTP_NO_CONTENT] = StrIni("No Content"),
    [CAPY_HTTP_RESET_CONTENT] = StrIni("Reset Content"),
    [CAPY_HTTP_PARTIAL_CONTENT] = StrIni("Partial Content"),
    [CAPY_HTTP_MULTIPLE_CHOICES] = StrIni("Multiple Choices"),
    [CAPY_HTTP_MOVED_PERMANENTLY] = StrIni("Moved Permanently"),
    [CAPY_HTTP_FOUND] = StrIni("Found"),
    [CAPY_HTTP_SEE_OTHER] = StrIni("See Other"),
    [CAPY_HTTP_NOT_MODIFIED] = StrIni("Not Modified"),
    [CAPY_HTTP_USE_PROXY] = StrIni("Use Proxy"),
    [CAPY_HTTP_TEMPORARY_REDIRECT] = StrIni("Temporary Redirect"),
    [CAPY_HTTP_PERMANENT_REDIRECT] = StrIni("Permanent Redirect"),
    [CAPY_HTTP_BAD_REQUEST] = StrIni("Bad Request"),
    [CAPY_HTTP_UNAUTHORIZED] = StrIni("Unauthorized"),
    [CAPY_HTTP_PAYMENT_REQUIRED] = StrIni("Payment Required"),
    [CAPY_HTTP_FORBIDDEN] = StrIni("Forbidden"),
    [CAPY_HTTP_NOT_FOUND] = StrIni("Not Found"),
    [CAPY_HTTP_METHOD_NOT_ALLOWED] = StrIni("Method Not Allowed"),
    [CAPY_HTTP_NOT_ACCEPTABLE] = StrIni("Not Acceptable"),
    [CAPY_HTTP_PROXY_AUTHENTICATION_REQUIRED] = StrIni("Proxy Authentication Required"),
    [CAPY_HTTP_REQUEST_TIMEOUT] = StrIni("Request Timeout"),
    [CAPY_HTTP_CONFLICT] = StrIni("Conflict"),
    [CAPY_HTTP_GONE] = StrIni("Gone"),
    [CAPY_HTTP_LENGTH_REQUIRED] = StrIni("Length Required"),
    [CAPY_HTTP_PRECONDITION_FAILED] = StrIni("Precondition Failed"),
    [CAPY_HTTP_CONTENT_TOO_LARGE] = StrIni("Content Too Large"),
    [CAPY_HTTP_URI_TOO_LONG] = StrIni("Request Uri Too Long"),
    [CAPY_HTTP_UNSUPPORTED_MEDIA_TYPE] = StrIni("Unsupported Media Type"),
    [CAPY_HTTP_RANGE_NOT_SATISFIABLE] = StrIni("Range Not Satisfiable"),
    [CAPY_HTTP_EXPECTATION_FAILED] = StrIni("Expectation Failed"),
    [CAPY_HTTP_IM_A_TEAPOT] = StrIni("I'm A Teapot"),
    [CAPY_HTTP_MISDIRECTED_REQUEST] = StrIni("Misdirected Request"),
    [CAPY_HTTP_UNPROCESSABLE_ENTITY] = StrIni("Unprocessable Entity"),
    [CAPY_HTTP_UPGRADE_REQUIRED] = StrIni("Upgrade Required"),
    [CAPY_HTTP_INTERNAL_SERVER_ERROR] = StrIni("Internal Server Error"),
    [CAPY_HTTP_NOT_IMPLEMENTED] = StrIni("Not Implemented"),
    [CAPY_HTTP_BAD_GATEWAY] = StrIni("Bad Gateway"),
    [CAPY_HTTP_SERVICE_UNAVAILABLE] = StrIni("Service Unavailable"),
    [CAPY_HTTP_GATEWAY_TIMEOUT] = StrIni("Gateway Timeout"),
    [CAPY_HTTP_HTTP_VERSION_NOT_SUPPORTED] = StrIni("HTTP Version Not Supported"),
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

// INTERNAL DEFINITIONS

static bool http_validate_string(capy_string s, int categories, const char *chars)
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
    char *data = Make(arena, char, input.size + 1);

    if (data == NULL)
    {
        return ErrStd(ENOMEM);
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

    return Ok;
}

static capy_httproutermap *capy_http_router_map_init(capy_arena *arena, size_t capacity)
{
    char *addr = capy_arena_alloc(arena, sizeof(capy_httproutermap) + (sizeof(capy_httprouter) * capacity), 8, true);

    if (addr == NULL)
    {
        return NULL;
    }

    capy_httproutermap *map = Cast(capy_httproutermap *, addr);

    map->size = 0;
    map->capacity = capacity;
    map->element_size = sizeof(capy_httprouter);
    map->items = Cast(capy_httprouter *, addr + sizeof(capy_httproutermap));

    return map;
}

static capy_httprouter *capy_http_router_map_get(capy_httproutermap *map, capy_string key)
{
    return capy_strmap_get(&map->strmap, key);
}

static MustCheck capy_err capy_http_router_map_set(capy_arena *arena, capy_httproutermap *map, capy_httprouter router)
{
    return capy_strmap_set(arena, &map->strmap, &router);
}

static capy_httprouter *http_route_add_(capy_arena *arena, capy_httprouter *router, capy_httpmethod method, capy_string suffix, capy_string path, capy_http_handler handler)
{
    if (router == NULL)
    {
        router = Make(arena, capy_httprouter, 1);

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
        router->routes[method] = (capy_httproute){.method = method, .path = path, .handler = handler};
        return router;
    }

    if (segment.data[0] == '^')
    {
        segment = Str("^");
    }

    capy_httprouter *child = capy_http_router_map_get(router->segments, segment);

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

static capy_err http_response_status(capy_httpresp *response)
{
    capy_string status_text = http_status_string[response->status];

    capy_err err = capy_strkvnmap_add(response->headers, Str("Content-Type"), Str("text/plain; charset=UTF-8"));

    if (err.code)
    {
        return err;
    }

    err = capy_buffer_write_fmt(response->body, 0, "%u %s\n", response->status, status_text.data);

    if (err.code)
    {
        return err;
    }

    return Ok;
}

static capy_err http_response_wcstr(capy_httpresp *response, const char *msg)
{
    capy_err err = capy_strkvnmap_add(response->headers, Str("Content-Type"), Str("text/plain; charset=UTF-8"));

    if (err.code)
    {
        return err;
    }

    err = capy_buffer_write_fmt(response->body, 0, "%s\n", msg);

    if (err.code)
    {
        return err;
    }

    return Ok;
}

static capy_err http_query_pctdecode(capy_arena *arena, capy_string *output, capy_string input)
{
    if (input.size == 0)
    {
        *output = (capy_string){.size = 0};
        return Ok;
    }

    char *bytes = Make(arena, char, output->size + 1);

    if (bytes == NULL)
    {
        return ErrStd(ENOMEM);
    }

    size_t j = 0;

    for (size_t i = 0; i < input.size; i++, j++)
    {
        if (input.data[i] == '%')
        {
            int v = http_hexdecode[Cast(unsigned char, input.data[i + 1])] << 4 |
                    http_hexdecode[Cast(unsigned char, input.data[i + 2])];

            bytes[j] = Cast(char, v);
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

    return Ok;
}

static capy_httprouter *capy_http_router_init(capy_arena *arena, int n, capy_httproute *routes)
{
    capy_httprouter *router = NULL;

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

static capy_httproute *capy_http_route_get(capy_httprouter *router, capy_httpmethod method, capy_string path)
{
    http_consume_chars(&path, "/", 0);
    capy_string segment = http_next_token(&path, "/");

    if (segment.size == 0)
    {
        capy_httproute *route = router->routes + method;

        if (route->handler == NULL)
        {
            return NULL;
        }

        return route;
    }

    capy_httprouter *child = capy_http_router_map_get(router->segments, segment);

    if (child == NULL)
    {
        child = capy_http_router_map_get(router->segments, Str("^"));

        if (child == NULL)
        {
            return NULL;
        }
    }

    return capy_http_route_get(child, method, path);
}

static capy_err capy_http_router_handle(capy_arena *arena, capy_httprouter *router, capy_httpreq *request, capy_httpresp *response)
{
    capy_err err;

    capy_httproute *route = capy_http_route_get(router, request->method, request->uri.path);

    if (route == NULL)
    {
        response->status = CAPY_HTTP_NOT_FOUND;
        return http_response_status(response);
    }

    err = capy_http_parse_uriparams(request->params, request->uri.path, route->path);

    if (err.code)
    {
        return ErrWrap(err, "Failed to parse uri params");
    }

    err = capy_http_parse_query(request->params, request->uri.query);

    if (err.code)
    {
        return ErrWrap(err, "Failed to parse query params");
    }

    err = route->handler(arena, request, response);

    if (err.code)
    {
        response->status = CAPY_HTTP_INTERNAL_SERVER_ERROR;
        return http_response_wcstr(response, err.msg);
    }

    return Ok;
}

static void httpconn_close(httpconn *conn)
{
    tcpconn_close(conn->tcp);
    conn->state = STATE_CLOSE;
}

static int httpconn_parse_msgline(httpconn *conn, capy_string *line)
{
    while (conn->line_cursor <= conn->line_buffer->size)
    {
        if (conn->line_cursor >= 2 &&
            conn->line_buffer->data[conn->line_cursor - 2] == '\r' &&
            conn->line_buffer->data[conn->line_cursor - 1] == '\n')
        {
            break;
        }

        conn->line_cursor += 1;
    }

    if (conn->line_cursor > conn->line_buffer->size)
    {
        return -1;
    }

    *line = capy_string_bytes(conn->line_cursor - 2, conn->line_buffer->data);

    return 0;
}

static void httpconn_shl_msg(httpconn *conn, size_t size)
{
    capy_buffer_shl(conn->line_buffer, size);

    if (size >= conn->line_cursor)
    {
        conn->line_cursor = 2;
    }
    else
    {
        conn->line_cursor -= size;
    }
}

static void httpconn_shl_msgline(httpconn *conn)
{
    httpconn_shl_msg(conn, conn->line_cursor);
}

static capy_err httpconn_parse_reqline(httpconn *conn)
{
    capy_string line;

    if (httpconn_parse_msgline(conn, &line))
    {
        conn->after_read = STATE_PARSE_REQLINE;
        conn->state = STATE_READ_REQUEST;
        return Ok;
    }

    capy_err err = capy_http_parse_reqline(conn->arena, &conn->request, line);

    if (err.code == EINVAL)
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }
    else if (err.code)
    {
        return ErrWrap(err, "Failed to parse request line");
    }

    httpconn_shl_msgline(conn);
    conn->state = STATE_PARSE_HEADERS;

    return Ok;
}

static capy_err httpconn_parse_headers(httpconn *conn)
{
    capy_err err;

    for (;;)
    {
        capy_string line;

        if (httpconn_parse_msgline(conn, &line))
        {
            conn->after_read = STATE_PARSE_HEADERS;
            conn->state = STATE_READ_REQUEST;
            return Ok;
        }

        if (line.size == 0)
        {
            httpconn_shl_msgline(conn);
            break;
        }

        err = capy_http_parse_field(conn->request.headers, line);

        if (err.code == EINVAL)
        {
            conn->state = STATE_BAD_REQUEST;
            return Ok;
        }
        else if (err.code)
        {
            return ErrWrap(err, "Failed to parse headers");
        }

        httpconn_shl_msgline(conn);
    }

    err = capy_http_validate_request(conn->arena, &conn->request);

    if (err.code == EINVAL)
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }
    else if (err.code)
    {
        return ErrWrap(err, "Failed to validate request");
    }

    if (conn->request.content_length)
    {
        conn->chunk_size = conn->request.content_length;
        conn->state = STATE_PARSE_CONTENT;
    }
    else if (conn->request.chunked)
    {
        conn->state = STATE_PARSE_CHUNKSIZE;
    }
    else
    {
        conn->state = STATE_ROUTE_REQUEST;
    }

    return Ok;
}

static capy_err httpconn_parse_trailers(httpconn *conn)
{
    capy_err err;

    for (;;)
    {
        capy_string line;

        if (httpconn_parse_msgline(conn, &line))
        {
            conn->after_read = STATE_PARSE_TRAILERS;
            conn->state = STATE_READ_REQUEST;
            return Ok;
        }

        if (line.size == 0)
        {
            httpconn_shl_msgline(conn);
            break;
        }

        err = capy_http_parse_field(conn->request.trailers, line);

        if (err.code == EINVAL)
        {
            conn->state = STATE_BAD_REQUEST;
            return Ok;
        }
        else if (err.code)
        {
            return ErrWrap(err, "Failed to parse trailers");
        }

        httpconn_shl_msgline(conn);
    }

    conn->state = STATE_ROUTE_REQUEST;
    return Ok;
}

static capy_err httpconn_parse_chunksize(httpconn *conn)
{
    capy_string line;

    if (httpconn_parse_msgline(conn, &line))
    {
        conn->after_read = STATE_PARSE_CHUNKSIZE;
        conn->state = STATE_READ_REQUEST;
        return Ok;
    }

    uint64_t value;

    // todo: validate chunck_size extensions
    if (capy_string_parse_hexdigits(&value, line) == 0)
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }

    conn->chunk_size = (size_t)(value);

    httpconn_shl_msgline(conn);

    if (conn->chunk_size == 0)
    {
        conn->state = STATE_PARSE_TRAILERS;
    }
    else
    {
        conn->state = STATE_PARSE_CHUNKDATA;
    }

    return Ok;
}

static capy_err httpconn_parse_chunkdata(httpconn *conn)
{
    capy_err err;

    size_t msg_size = conn->line_buffer->size;

    if (conn->chunk_size + 2 > msg_size)
    {
        err = capy_buffer_write_bytes(conn->content_buffer, msg_size, conn->line_buffer->data);

        if (err.code)
        {
            return ErrWrap(err, "Failed to read chunk data");
        }

        httpconn_shl_msg(conn, msg_size);

        conn->chunk_size -= msg_size;

        conn->after_read = STATE_PARSE_CHUNKDATA;
        conn->state = STATE_READ_REQUEST;
        return Ok;
    }

    const char *end = conn->line_buffer->data + conn->chunk_size;

    if (end[0] != '\r' || end[1] != '\n')
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }

    err = capy_buffer_write_bytes(conn->content_buffer, conn->chunk_size, conn->line_buffer->data);

    if (err.code)
    {
        return ErrWrap(err, "Failed to read chunk data");
    }

    httpconn_shl_msg(conn, conn->chunk_size + 2);

    conn->state = STATE_PARSE_CHUNKSIZE;
    return Ok;
}

static capy_err httpconn_parse_reqbody(httpconn *conn)
{
    capy_err err;

    size_t message_size = conn->line_buffer->size;

    if (conn->chunk_size > message_size)
    {
        err = capy_buffer_write_bytes(conn->content_buffer, message_size, conn->line_buffer->data);

        if (err.code)
        {
            return ErrWrap(err, "Failed to read content data");
        }

        httpconn_shl_msg(conn, message_size);

        conn->chunk_size -= message_size;

        conn->after_read = STATE_PARSE_CONTENT;
        conn->state = STATE_READ_REQUEST;
        return Ok;
    }

    err = capy_buffer_write_bytes(conn->content_buffer, conn->chunk_size, conn->line_buffer->data);

    if (err.code)
    {
        return ErrWrap(err, "Failed to read content data");
    }

    httpconn_shl_msg(conn, conn->chunk_size);

    conn->state = STATE_ROUTE_REQUEST;
    return Ok;
}

static capy_err httpconn_prepare_badrequest(httpconn *conn)
{
    capy_err err;

    conn->request.close = true;
    conn->response.status = CAPY_HTTP_BAD_REQUEST;

    err = http_response_status(&conn->response);

    if (err.code)
    {
        return ErrWrap(err, "Failed to generate BAD_REQUEST");
    }

    err = capy_http_write_response(conn->response_buffer, &conn->response, true);

    if (err.code)
    {
        return ErrWrap(err, "Failed to write to response_buffer");
    }

    return Ok;
}

static capy_err httpconn_route_request(httpconn *conn)
{
    capy_err err = capy_buffer_write_null(conn->content_buffer);

    if (err.code)
    {
        return ErrWrap(err, "Failed to write null terminator");
    }

    conn->request.content = capy_string_bytes(conn->content_buffer->size, conn->content_buffer->data);

    err = capy_http_router_handle(conn->arena, conn->router, &conn->request, &conn->response);

    if (err.code)
    {
        return ErrWrap(err, "Failed to handle request");
    }

    err = capy_http_write_response(conn->response_buffer, &conn->response, conn->request.close);

    if (err.code)
    {
        return ErrWrap(err, "Failed to write to response_buffer");
    }

    conn->state = STATE_WRITE_RESPONSE;
    return Ok;
}

static capy_err httpconn_reset(httpconn *conn)
{
    capy_err err = capy_arena_free(conn->arena, conn->marker_init);

    if (err.code)
    {
        return err;
    }

    conn->line_buffer->size = 0;
    conn->line_cursor = 2;
    conn->after_read = STATE_UNKNOWN;

    conn->request = (capy_httpreq){
        .headers = capy_strkvnmap_init(conn->arena, 16),
        .trailers = capy_strkvnmap_init(conn->arena, 4),
        .params = capy_strkvnmap_init(conn->arena, 8),
        .query = capy_strkvnmap_init(conn->arena, 8),
    };

    conn->response = (capy_httpresp){
        .headers = capy_strkvnmap_init(conn->arena, 16),
        .body = capy_buffer_init(conn->arena, 256),
    };

    conn->content_buffer = capy_buffer_init(conn->arena, 256);
    conn->response_buffer = capy_buffer_init(conn->arena, 512);

    conn->mem_headers = 0;
    conn->mem_content = 0;
    conn->mem_trailers = 0;
    conn->mem_response = 0;

    conn->state = STATE_PARSE_REQLINE;

    return Ok;
}

static void httpconn_trace(httpconn *conn)
{
    struct timespec timestamp = capy_now();

    int64_t elapsed = capy_timespec_diff(timestamp, conn->timestamp);
    const char *elapsed_unit;
    nanoseconds_normalize(&elapsed, &elapsed_unit);

    conn->timestamp = timestamp;

    size_t mem_total = capy_arena_used(conn->arena);
    size_t to_read = (conn->line_buffer) ? conn->line_buffer->size : 0;
    size_t to_write = (conn->response_buffer) ? conn->response_buffer->size : 0;

    LogDbg("worker: %-24s | WK:%-4zu SK:%-4zu MH:%-8zu MC:%-8zu MT:%-8zu MR:%-8zu MA:%-8zu RS:%-8zu WS:%-8zu | %3" PRIi64 " %s",
           httpconnstate_cstr[conn->state], capy_thread_id(), conn->conn_id,
           conn->mem_headers, conn->mem_content, conn->mem_trailers, conn->mem_response, mem_total,
           to_read, to_write, elapsed, elapsed_unit);
}

static capy_err httpconn_write_response(httpconn *conn)
{
    capy_err err;

    size_t old_size = conn->response_buffer->size;

    err = tcpconn_send(conn->tcp, conn->response_buffer, conn->options->inactivity_timeout);

    if (err.code)
    {
        if (err.code == ECONNRESET || err.code == EPROTO)
        {
            conn->state = STATE_CLOSE;
            return Ok;
        }
        else
        {
            return ErrWrap(err, "Failed to write response");
        }
    }

    if (conn->response_buffer->size == old_size)
    {
        conn->state = STATE_CLOSE;
        return Ok;
    }

    if (conn->response_buffer->size > 0)
    {
        conn->state = STATE_WRITE_RESPONSE;
    }
    else if (conn->request.close || capy_task_canceled())
    {
        tcpconn_shutdown(conn->tcp);
        conn->state = STATE_CLOSE;
    }
    else
    {
        conn->state = STATE_RESET;
    }

    return Ok;
}

static capy_err httpconn_read_request(httpconn *conn)
{
    capy_err err;

    size_t line_limit = conn->line_buffer->capacity;
    size_t line_size = conn->line_buffer->size;
    size_t bytes_wanted = line_limit - line_size;

    if (bytes_wanted == 0)
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }

    size_t old_size = conn->line_buffer->size;

    err = tcpconn_recv(conn->tcp, conn->line_buffer, conn->options->inactivity_timeout);

    if (err.code)
    {
        if (err.code == ECONNRESET || err.code == EPROTO)
        {
            conn->state = STATE_CLOSE;
            return Ok;
        }
        else
        {
            return ErrWrap(err, "Failed to read socket");
        }
    }

    if (conn->line_buffer->size - old_size == 0)
    {
        conn->state = STATE_CLOSE;
        return Ok;
    }

    conn->state = conn->after_read;
    return Ok;
}

static capy_err httpconn_run(httpconn *conn)
{
    for (;;)
    {
        capy_err err = Ok;

        httpconn_trace(conn);

        ssize_t begin = (ssize_t)capy_arena_used(conn->arena);

        switch (conn->state)
        {
            case STATE_RESET:
            {
                err = httpconn_reset(conn);
            }
            break;

            case STATE_READ_REQUEST:
            {
                err = httpconn_read_request(conn);
            }
            break;

            case STATE_PARSE_REQLINE:
            {
                err = httpconn_parse_reqline(conn);
                conn->mem_headers += (ssize_t)capy_arena_used(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_HEADERS:
            {
                err = httpconn_parse_headers(conn);
                conn->mem_headers += (ssize_t)capy_arena_used(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_CONTENT:
            {
                err = httpconn_parse_reqbody(conn);
                conn->mem_content += (ssize_t)capy_arena_used(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_CHUNKSIZE:
            {
                err = httpconn_parse_chunksize(conn);
                conn->mem_content += (ssize_t)capy_arena_used(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_CHUNKDATA:
            {
                err = httpconn_parse_chunkdata(conn);
                conn->mem_content += (ssize_t)capy_arena_used(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_TRAILERS:
            {
                err = httpconn_parse_trailers(conn);
                conn->mem_trailers += (ssize_t)capy_arena_used(conn->arena) - begin;
            }
            break;

            case STATE_ROUTE_REQUEST:
            {
                err = httpconn_route_request(conn);
                conn->mem_response += (ssize_t)capy_arena_used(conn->arena) - begin;
            }
            break;

            case STATE_BAD_REQUEST:
            {
                err = httpconn_prepare_badrequest(conn);
                conn->mem_response += (ssize_t)capy_arena_used(conn->arena) - begin;
            }
            break;

            case STATE_WRITE_RESPONSE:
            {
                err = httpconn_write_response(conn);
            }
            break;

            case STATE_CLOSE:
            {
                httpconn_close(conn);
                return Ok;
            }

            default:
            {
                capy_assert(false);
            }
        }

        if (err.code)
        {
            LogErr("While processing request: %s", err.msg);
            httpconn_close(conn);
            return err;
        }
    }
}

// PUBLIC DEFINITIONS

capy_httpmethod capy_http_parse_method(capy_string input)
{
    switch (input.size)
    {
        case 3:
        {
            if (ArrCmp3(input.data, 'G', 'E', 'T'))
            {
                return CAPY_HTTP_GET;
            }
            else if (ArrCmp3(input.data, 'P', 'U', 'T'))
            {
                return CAPY_HTTP_PUT;
            }
        }
        break;

        case 4:
        {
            if (ArrCmp4(input.data, 'H', 'E', 'A', 'D'))
            {
                return CAPY_HTTP_HEAD;
            }
            else if (ArrCmp4(input.data, 'P', 'O', 'S', 'T'))
            {
                return CAPY_HTTP_POST;
            }
        }
        break;

        case 5:
        {
            if (ArrCmp5(input.data, 'T', 'R', 'A', 'C', 'E'))
            {
                return CAPY_HTTP_TRACE;
            }
            else if (ArrCmp5(input.data, 'P', 'A', 'T', 'C', 'H'))
            {
                return CAPY_HTTP_PATCH;
            }
        }
        break;

        case 6:
        {
            if (ArrCmp6(input.data, 'D', 'E', 'L', 'E', 'T', 'E'))
            {
                return CAPY_HTTP_DELETE;
            }
        }
        break;

        case 7:
        {
            if (ArrCmp7(input.data, 'C', 'O', 'N', 'N', 'E', 'C', 'T'))
            {
                return CAPY_HTTP_CONNECT;
            }
            else if (ArrCmp7(input.data, 'O', 'P', 'T', 'I', 'O', 'N', 'S'))
            {
                return CAPY_HTTP_OPTIONS;
            }
        }
        break;
    }

    return CAPY_HTTP_METHOD_UNK;
}

capy_httpversion capy_http_parse_version(capy_string input)
{
    if (input.size != 8)
    {
        return CAPY_HTTP_VERSION_UNK;
    }

    if (!ArrCmp5(input.data, 'H', 'T', 'T', 'P', '/'))
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

capy_err capy_http_write_response(capy_buffer *buffer, capy_httpresp *response, int close)
{
    capy_err err;

    time_t t = time(NULL);
    struct tm ct;
    gmtime_r(&t, &ct);

    size_t content_length = (response->body) ? response->body->size : 0;
    const char *close_header = (close) ? "Connection: close\r\n" : "";

    err = capy_buffer_write_fmt(buffer, 0,
                                "HTTP/1.1 %d %s\r\n"
                                "Date: %s, %02d %s %04d %02d:%02d:%02d GMT\r\n"
                                "Content-Length: %zu\r\n"
                                "%s",
                                response->status, http_status_string[response->status].data,
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
        for (capy_strkvn *header = capy_strkvnmap_at(response->headers, i); header != NULL; header = header->next)
        {
            err = capy_buffer_write_fmt(buffer, 0, "%s: %s\r\n", header->key.data, header->value.data);

            if (err.code)
            {
                return err;
            }
        }
    }

    err = capy_buffer_write_bytes(buffer, 2, "\r\n");

    if (err.code)
    {
        return err;
    }

    return capy_buffer_write_bytes(buffer, response->body->size, response->body->data);
}

capy_err capy_http_parse_reqline(capy_arena *arena, capy_httpreq *request, capy_string line)
{
    capy_err err;

    capy_string method = http_next_token(&line, " ");

    if (http_consume_chars(&line, " ", 0) != 1)
    {
        return ErrStd(EINVAL);
    }

    capy_string uri = http_next_token(&line, " ");

    request->uri_raw = uri;

    if (http_consume_chars(&line, " ", 0) != 1)
    {
        return ErrStd(EINVAL);
    }

    capy_string version = http_next_token(&line, " ");

    if (line.size != 0)
    {
        return ErrStd(EINVAL);
    }

    request->method = capy_http_parse_method(method);

    if (request->method == CAPY_HTTP_METHOD_UNK)
    {
        return ErrStd(EINVAL);
    }

    request->version = capy_http_parse_version(version);

    if (request->version == CAPY_HTTP_VERSION_UNK)
    {
        return ErrStd(EINVAL);
    }

    if (request->method == CAPY_HTTP_CONNECT)
    {
        request->uri = (capy_uri){.authority = uri};
        request->uri = capy_uri_parse_authority(request->uri);

        if (request->uri.userinfo.size)
        {
            return ErrStd(EINVAL);
        }
    }
    else if (request->method == CAPY_HTTP_OPTIONS && capy_string_eq(uri, Str("*")))
    {
        request->uri = (capy_uri){.path = uri};
    }
    else
    {
        request->uri = capy_uri_parse(uri);
    }

    if (!capy_uri_valid(request->uri))
    {
        return ErrStd(EINVAL);
    }

    err = capy_string_copy(arena, &request->uri_raw, request->uri_raw);

    if (err.code)
    {
        return err;
    }

    err = capy_uri_normalize(arena, &request->uri.scheme, request->uri.scheme, true);

    if (err.code)
    {
        return err;
    }

    err = capy_uri_normalize(arena, &request->uri.authority, request->uri.authority, false);

    if (err.code)
    {
        return err;
    }

    err = capy_uri_normalize(arena, &request->uri.userinfo, request->uri.userinfo, false);

    if (err.code)
    {
        return err;
    }

    err = capy_uri_normalize(arena, &request->uri.host, request->uri.host, true);

    if (err.code)
    {
        return err;
    }

    err = capy_uri_normalize(arena, &request->uri.port, request->uri.port, false);

    if (err.code)
    {
        return err;
    }

    err = capy_uri_normalize(arena, &request->uri.path, request->uri.path, false);

    if (err.code)
    {
        return err;
    }

    err = capy_uri_normalize(arena, &request->uri.query, request->uri.query, false);

    if (err.code)
    {
        return err;
    }

    request->uri.fragment = (capy_string){.size = 0};

    return Ok;
}

capy_err capy_http_parse_query(capy_strkvnmap *fields, capy_string query)
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

        err = http_query_pctdecode(fields->arena, &name, name);

        if (err.code)
        {
            return err;
        }

        err = http_query_pctdecode(fields->arena, &value, value);

        if (err.code)
        {
            return err;
        }

        err = capy_strkvnmap_add(fields, name, value);

        if (err.code)
        {
            return err;
        }
    }

    return Ok;
}

capy_err capy_http_validate_request(capy_arena *arena, capy_httpreq *request)
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
            return ErrStd(EINVAL);
    }

    capy_strkvn *host = capy_strkvnmap_get(request->headers, Str("Host"));

    if (host == NULL || host->next != NULL)
    {
        return ErrStd(EINVAL);
    }

    if ((request->uri.flags & CAPY_URI_AUTHORITY) && request->uri.authority.size == 0)
    {
        return ErrStd(EINVAL);
    }

    if (request->uri.scheme.size == 0)
    {
        request->uri.scheme = Str("http");
    }

    if (!capy_string_eq(request->uri.scheme, Str("http")))
    {
        return ErrStd(EINVAL);
    }

    if (request->method != CAPY_HTTP_CONNECT)
    {
        if (request->uri.authority.size == 0)
        {
            request->uri.authority = host->value;
            request->uri = capy_uri_parse_authority(request->uri);

            if (!capy_uri_valid(request->uri))
            {
                return ErrStd(EINVAL);
            }
        }
    }

    if (request->uri.port.size == 0)
    {
        request->uri.port = Str("80");
    }

    if (request->uri.authority.size == 0)
    {
        return ErrStd(EINVAL);
    }

    err = capy_string_join(arena, &host->value, ":", 2, (capy_string[]){request->uri.host, request->uri.port});

    if (err.code)
    {
        return err;
    }

    capy_strkvn *transfer_encoding = capy_strkvnmap_get(request->headers, Str("Transfer-Encoding"));

    if (transfer_encoding != NULL)
    {
        if (transfer_encoding->next || !capy_string_eq(transfer_encoding->value, Str("chunked")))
        {
            return ErrStd(EINVAL);
        }

        request->chunked = true;
        request->content_length = 0;
    }

    capy_strkvn *content_length = capy_strkvnmap_get(request->headers, Str("Content-Length"));

    if (content_length != NULL)
    {
        if (request->chunked || content_length->next || !http_validate_string(content_length->value, HTTP_DIGIT, ""))
        {
            return ErrStd(EINVAL);
        }

        request->content_length = strtoull(content_length->value.data, NULL, 10);
        request->chunked = 0;
    }

    capy_strkvn *connection = capy_strkvnmap_get(request->headers, Str("Connection"));

    if (connection != NULL)
    {
        if (connection->next != NULL)
        {
            return ErrStd(EINVAL);
        }

        if (capy_string_eq(connection->value, Str("close")))
        {
            request->close = true;
        }
        else if (capy_string_eq(connection->value, Str("keep-alive")))
        {
            request->close = false;
        }
        else
        {
            return ErrStd(EINVAL);
        }
    }

    err = capy_http_parse_query(request->query, request->uri.query);

    if (err.code)
    {
        return err;
    }

    return Ok;
}

capy_err capy_http_parse_field(capy_strkvnmap *fields, capy_string line)
{
    capy_err err;

    capy_string name = http_next_token(&line, ":");

    if (name.size == 0 || !http_validate_string(name, HTTP_TOKEN, NULL))
    {
        return ErrStd(EINVAL);
    }

    if (http_consume_chars(&line, ":", 0) != 1)
    {
        return ErrStd(EINVAL);
    }

    capy_string value = capy_string_trim(line, " \t");

    if (!http_validate_string(value, HTTP_VCHAR, " \t"))
    {
        return ErrStd(EINVAL);
    }

    err = http_canonical_field_name(fields->arena, &name, name);

    if (err.code)
    {
        return err;
    }

    err = capy_string_copy(fields->arena, &value, value);

    if (err.code)
    {
        return err;
    }

    return capy_strkvnmap_add(fields, name, value);
}

capy_err capy_http_parse_uriparams(capy_strkvnmap *params, capy_string path, capy_string handler_path)
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

        err = capy_strkvnmap_add(params, handler_path_segment, path_segment);

        if (err.code)
        {
            return err;
        }
    }

    return Ok;
}
