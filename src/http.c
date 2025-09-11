#include <capy/capy.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// MACROS

#define HTTP_VCHAR 0x1
#define HTTP_TOKEN 0x2
#define HTTP_DIGIT 0x4
#define HTTP_HEXDIGIT 0x8

#define http_bcmp2(m, c0, c1) \
    ((m)[0] == (c0) && (m)[1] == (c1))

#define http_bcmp3(m, c0, c1, c2) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2))

#define http_bcmp4(m, c0, c1, c2, c3) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3))

#define http_bcmp5(m, c0, c1, c2, c3, c4) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3) && (m)[4] == (c4))

#define http_bcmp6(m, c0, c1, c2, c3, c4, c5) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3) && (m)[4] == (c4) && (m)[5] == (c5))

#define http_bcmp7(m, c0, c1, c2, c3, c4, c5, c6) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3) && (m)[4] == (c4) && (m)[5] == (c5) && (m)[6] == (c6))

// STATIC

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

// STATIC DEFINITIONS

static int http_string_validate(capy_string s, int categories, const char *chars)
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

static size_t http_consume_chars(capy_string *buffer, const char *chars)
{
    capy_string input = *buffer;

    size_t i;

    for (i = 0; i < input.size && capy_char_is(input.data[i], chars); i++)
    {
    }

    *buffer = capy_string_shl(input, i);

    return i;
}

static int http_canonical_field_name(capy_arena *arena, capy_string input, capy_string *output)
{
    int upper = 1;

    char *data = capy_arena_make(arena, char, input.size + 1);

    if (data == NULL)
    {
        return ENOMEM;
    }

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

    return 0;
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
    map->items = ptrcast(capy_http_router *, map + 1);

    return map;
}

static capy_http_router *capy_http_router_map_get(capy_http_router_map *map, capy_string key)
{
    return capy_strmap_get(map->items, sizeof(capy_http_router), map->capacity, key);
}

static must_check int capy_http_router_map_set(capy_arena *arena, capy_http_router_map *map, capy_http_router router)
{
    void *items = capy_strmap_set(arena, map->items, sizeof(capy_http_router), &map->capacity, &map->size, &router);

    if (items == NULL)
    {
        return ENOMEM;
    }

    map->items = items;

    return 0;
}

static capy_http_route_map *capy_http_route_map_init(capy_arena *arena, size_t capacity)
{
    capy_http_route_map *map = capy_arena_alloc(arena, sizeof(capy_http_route_map) + (sizeof(capy_http_route) * capacity), 8, true);

    if (map == NULL)
    {
        return NULL;
    }

    map->size = 0;
    map->capacity = capacity;
    map->items = ptrcast(capy_http_route *, map + 1);

    return map;
}

static capy_http_route *capy_http_route_map_get(capy_http_route_map *map, capy_string key)
{
    return capy_strmap_get(map->items, sizeof(capy_http_route), map->capacity, key);
}

static must_check int capy_http_route_map_set(capy_arena *arena, capy_http_route_map *map, capy_http_route route)
{
    void *items = capy_strmap_set(arena, map->items, sizeof(capy_http_route), &map->capacity, &map->size, &route);

    if (items == NULL)
    {
        return ENOMEM;
    }

    map->items = items;

    return 0;
}

static capy_http_router *http_route_add_(capy_arena *arena, capy_http_router *router, capy_string method, capy_string suffix, capy_string path, capy_http_handler *handler)
{
    if (router == NULL)
    {
        router = capy_arena_make(arena, capy_http_router, 1);
        router->segments = capy_http_router_map_init(arena, 4);
        router->routes = capy_http_route_map_init(arena, 2);
    }

    http_consume_chars(&suffix, "/");
    capy_string segment = http_next_token(&suffix, "/");

    if (segment.size == 0)
    {
        capy_http_route route = {.method = method, .path = path, .handler = handler};

        if (capy_http_route_map_set(arena, router->routes, route))
        {
            return NULL;
        }

        return router;
    }

    if (segment.data[0] == '^')
    {
        segment = capy_string_literal("^");
    }

    capy_http_router *child = capy_http_router_map_get(router->segments, segment);

    child = http_route_add_(arena, child, method, suffix, path, handler);
    child->segment = segment;

    if (capy_http_router_map_set(arena, router->segments, *child))
    {
        return NULL;
    }

    return router;
}

static int http_404_handler(capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    (void)request;

    response->content = capy_buffer_init(arena, 128);

    if (response->content == NULL)
    {
        return ENOMEM;
    }

    int err = capy_buffer_wcstr(response->content, "404 Not Found!\n");

    if (err)
    {
        return err;
    }

    response->status = CAPY_HTTP_NOT_FOUND;

    return 0;
}

// EXTERN INLINES

extern inline capy_string capy_http_method_string(capy_http_method method);

// DEFINITIONS

capy_http_method capy_http_parse_method(capy_string input)
{
    switch (input.size)
    {
        case 3:
        {
            if (http_bcmp3(input.data, 'G', 'E', 'T'))
            {
                return CAPY_HTTP_GET;
            }
            else if (http_bcmp3(input.data, 'P', 'U', 'T'))
            {
                return CAPY_HTTP_PUT;
            }
        }
        break;

        case 4:
        {
            if (http_bcmp4(input.data, 'H', 'E', 'A', 'D'))
            {
                return CAPY_HTTP_HEAD;
            }
            else if (http_bcmp4(input.data, 'P', 'O', 'S', 'T'))
            {
                return CAPY_HTTP_POST;
            }
        }
        break;

        case 5:
        {
            if (http_bcmp5(input.data, 'T', 'R', 'A', 'C', 'E'))
            {
                return CAPY_HTTP_TRACE;
            }
            else if (http_bcmp5(input.data, 'P', 'A', 'T', 'C', 'H'))
            {
                return CAPY_HTTP_PATCH;
            }
        }
        break;

        case 6:
        {
            if (http_bcmp6(input.data, 'D', 'E', 'L', 'E', 'T', 'E'))
            {
                return CAPY_HTTP_DELETE;
            }
        }
        break;

        case 7:
        {
            if (http_bcmp7(input.data, 'C', 'O', 'N', 'N', 'E', 'C', 'T'))
            {
                return CAPY_HTTP_CONNECT;
            }
            else if (http_bcmp7(input.data, 'O', 'P', 'T', 'I', 'O', 'N', 'S'))
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

    if (!http_bcmp5(input.data, 'H', 'T', 'T', 'P', '/'))
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

int capy_http_write_headers(capy_buffer *buffer, capy_http_response *response)
{
    int err = capy_buffer_format(buffer, 0,
                                 "HTTP/1.1 %d\r\nContent-Length: %llu\r\n",
                                 response->status,
                                 response->content[0].size);

    if (err)
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
            err = capy_buffer_format(buffer, 0, "%s: %s\r\n", header->key.data, header->value.data);

            if (err)
            {
                return err;
            }

            header = header->next;
        }
    }

    return capy_buffer_wcstr(buffer, "\r\n");
}

int capy_http_parse_reqline(capy_arena *arena, capy_http_request *request, capy_string line)
{
    capy_string method = http_next_token(&line, " ");

    if (http_consume_chars(&line, " ") != 1)
    {
        return EINVAL;
    }

    capy_string uri = http_next_token(&line, " ");

    if (http_consume_chars(&line, " ") != 1)
    {
        return EINVAL;
    }

    capy_string version = http_next_token(&line, " ");

    if (line.size != 0)
    {
        return EINVAL;
    }

    request->method = capy_http_parse_method(method);

    if (request->method == CAPY_HTTP_METHOD_UNK)
    {
        return EINVAL;
    }

    request->version = capy_http_parse_version(version);

    if (request->version == CAPY_HTTP_VERSION_UNK)
    {
        return EINVAL;
    }

    request->uri = (capy_uri){{NULL}};

    if (request->method == CAPY_HTTP_CONNECT)
    {
        request->uri.authority = uri;
        capy_uri_parse_authority(&request->uri);
        request->uri.userinfo = (capy_string){.size = 0};
    }
    else if (request->method == CAPY_HTTP_OPTIONS && capy_string_eq(uri, capy_string_literal("*")))
    {
    }
    else
    {
        request->uri = capy_uri_parse(uri);
    }

    if (!capy_uri_valid(request->uri))
    {
        return EINVAL;
    }

    request->uri.scheme = capy_uri_normalize(arena, request->uri.scheme, true);
    request->uri.authority = capy_uri_normalize(arena, request->uri.authority, false);
    request->uri.userinfo = capy_uri_normalize(arena, request->uri.userinfo, false);
    request->uri.host = capy_uri_normalize(arena, request->uri.host, true);
    request->uri.port = capy_uri_normalize(arena, request->uri.port, false);
    request->uri.path = capy_uri_normalize(arena, request->uri.path, false);
    request->uri.query = capy_uri_normalize(arena, request->uri.query, false);
    request->uri.fragment = (capy_string){.size = 0};

    return 0;
}

int capy_http_request_validate(capy_arena *arena, capy_http_request *request)
{
    request->content_length = 0;
    request->chunked = 0;

    capy_strkvn *host = capy_strkvmmap_get(request->headers, capy_string_literal("Host"));

    if (host == NULL || host->next != NULL)
    {
        return EINVAL;
    }

    if (request->uri.scheme.size == 0)
    {
        request->uri.scheme = capy_string_literal("http");
    }

    if (request->uri.authority.size == 0)
    {
        request->uri.authority = host->value;
        capy_uri_parse_authority(&request->uri);

        if (request->uri.userinfo.size || !capy_uri_valid(request->uri))
        {
            return EINVAL;
        }
    }

    if (request->uri.port.size == 0)
    {
        request->uri.port = capy_string_literal("80");
    }

    int err = capy_string_join(arena, &host->value, ":", 2, (capy_string[]){request->uri.host, request->uri.port});

    if (err)
    {
        return err;
    }

    capy_strkvn *transfer_encoding = capy_strkvmmap_get(request->headers, capy_string_literal("Transfer-Encoding"));

    if (transfer_encoding != NULL)
    {
        if (transfer_encoding->next || !capy_string_eq(transfer_encoding->value, capy_string_literal("chunked")))
        {
            return EINVAL;
        }

        request->chunked = true;
        request->content_length = 0;
    }

    capy_strkvn *content_length = capy_strkvmmap_get(request->headers, capy_string_literal("Content-Length"));

    if (content_length != NULL)
    {
        if (request->chunked || content_length->next || !http_string_validate(content_length->value, HTTP_DIGIT, ""))
        {
            return EINVAL;
        }

        request->content_length = strtoull(content_length->value.data, NULL, 10);
        request->chunked = 0;
    }

    return 0;
}

int capy_http_parse_field(capy_strkvmmap *fields, capy_string line)
{
    capy_string name = http_next_token(&line, ":");

    if (name.size == 0 || !http_string_validate(name, HTTP_TOKEN, NULL))
    {
        return EINVAL;
    }

    if (http_consume_chars(&line, ":") != 1)
    {
        return EINVAL;
    }

    capy_string value = capy_string_trim(line, " \t");

    if (!http_string_validate(value, HTTP_VCHAR, " \t"))
    {
        return EINVAL;
    }

    int err = http_canonical_field_name(fields->arena, name, &name);

    if (err)
    {
        return err;
    }

    err = capy_string_copy(fields->arena, &value, value);

    if (err)
    {
        return err;
    }

    return capy_strkvmmap_add(fields, name, value);
}

capy_http_router *capy_http_route_add(capy_arena *arena, capy_http_router *router, capy_string route, capy_http_handler *handler)
{
    capy_string path = route;
    capy_string method = http_next_token(&path, " ");

    if (capy_string_upper(arena, &method, method))
    {
        return NULL;
    }

    http_consume_chars(&path, " ");

    return http_route_add_(arena, router, method, path, path, handler);
}

capy_http_route *capy_http_route_get(capy_http_router *router, capy_http_method method, capy_string path)
{
    http_consume_chars(&path, "/");
    capy_string segment = http_next_token(&path, "/");

    if (segment.size == 0)
    {
        return capy_http_route_map_get(router->routes, capy_http_method_string(method));
    }

    capy_http_router *child = capy_http_router_map_get(router->segments, segment);

    if (child == NULL)
    {
        child = capy_http_router_map_get(router->segments, capy_string_literal("^"));

        if (child == NULL)
        {
            return NULL;
        }
    }

    return capy_http_route_get(child, method, path);
}

capy_strkvmmap *capy_http_parse_uriparams(capy_arena *arena, capy_string path, capy_string handler_path)
{
    capy_strkvmmap *params = NULL;

    for (;;)
    {
        http_consume_chars(&path, "/");
        capy_string path_segment = http_next_token(&path, "/");

        if (path_segment.size == 0)
        {
            break;
        }

        http_consume_chars(&handler_path, "/");
        capy_string handler_path_segment = http_next_token(&handler_path, "/");

        if (handler_path_segment.data[0] != '^')
        {
            continue;
        }

        if (params == NULL)
        {
            params = capy_strkvmmap_init(arena, 8);
        }

        handler_path_segment = capy_string_shl(handler_path_segment, 1);

        int err = capy_strkvmmap_add(params, handler_path_segment, path_segment);

        if (err)
        {
            return NULL;
        }
    }

    return params;
}

int capy_http_router_handle(capy_arena *arena, capy_http_router *router,
                            capy_http_request *request, capy_http_response *response)
{
    capy_http_route *route = capy_http_route_get(router, request->method, request->uri.path);

    if (route == NULL)
    {
        return http_404_handler(arena, request, response);
    }

    request->params = capy_http_parse_uriparams(arena, request->uri.path, route->path);
    return route->handler(arena, request, response);
}
