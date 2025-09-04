#include <capy/capy.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define HTTP_VCHAR 0x1
#define HTTP_TOKEN 0x2
#define HTTP_DIGIT 0x4
#define HTTP_HEXDIGIT 0x8

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

void capy_http_write_headers(capy_strbuf *strbuf, capy_http_response *response)
{
    capy_strbuf_format(strbuf, 0,
                       "HTTP/1.1 %d\r\nContent-Length: %llu\r\n",
                       response->status,
                       response->content[0].size);

    for (size_t i = 0; i < response->headers->capacity; i++)
    {
        capy_http_field *header = &response->headers->data[i];

        while (header != NULL)
        {
            if (header->name.size != 0)
            {
                capy_strbuf_format(strbuf, 0,
                                   "%s: %s\r\n",
                                   header->name.data, header->value.data);
            }

            header = header->next;
        }
    }

    capy_strbuf_write_cstr(strbuf, "\r\n");
}

int capy_http_parse_request_line(capy_arena *arena, capy_http_request *request, capy_string line)
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

    capy_http_field *host = capy_http_fields_get(request->headers, capy_string_literal("Host"));

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

    host->value = capy_string_join(arena, ":", 2, (capy_string[]){request->uri.host, request->uri.port});

    capy_http_field *transfer_encoding = capy_http_fields_get(request->headers, capy_string_literal("Transfer-Encoding"));

    if (transfer_encoding != NULL)
    {
        if (transfer_encoding->next || !capy_string_eq(transfer_encoding->value, capy_string_literal("chunked")))
        {
            return EINVAL;
        }

        request->chunked = true;
        request->content_length = 0;
    }

    capy_http_field *content_length = capy_http_fields_get(request->headers, capy_string_literal("Content-Length"));

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

static capy_string http_canonical_field_name(capy_arena *arena, capy_string header)
{
    int upper = 1;

    char *data = capy_arena_make(char, arena, header.size + 1);

    for (size_t i = 0; i < header.size; i++)
    {
        char c = header.data[i];

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

    return capy_string_bytes(header.size, data);
}

int capy_http_parse_field(capy_arena *arena, capy_http_fields *fields, capy_string line)
{
    capy_http_field field = {.next = NULL};

    field.name = http_next_token(&line, ":");

    if (field.name.size == 0 || !http_string_validate(field.name, HTTP_TOKEN, NULL))
    {
        return EINVAL;
    }

    if (http_consume_chars(&line, ":") != 1)
    {
        return EINVAL;
    }

    field.value = capy_string_trim(line, " \t");

    if (!http_string_validate(field.value, HTTP_VCHAR, " \t"))
    {
        return EINVAL;
    }

    field.name = http_canonical_field_name(arena, field.name);
    field.value = capy_string_copy(arena, field.value);

    capy_http_fields_add(fields, field.name, field.value);

    return 0;
}

capy_http_fields *capy_http_fields_init(capy_arena *arena, size_t capacity)
{
    return (capy_http_fields *)capy_smap_init(arena, sizeof(capy_http_field), capacity);
}

capy_http_field *capy_http_fields_get(capy_http_fields *headers, capy_string key)
{
    return capy_smap_get((capy_smap *)headers, key);
}

void capy_http_fields_set(capy_http_fields *headers, capy_string name, capy_string value)
{
    capy_http_field field = {
        .name = http_canonical_field_name(headers->arena, name),
        .value = capy_string_copy(headers->arena, value),
    };

    capy_smap_set((capy_smap *)headers, &field.name);
}

void capy_http_fields_add(capy_http_fields *headers, capy_string name, capy_string value)
{
    capy_http_field field = {
        .name = http_canonical_field_name(headers->arena, name),
        .value = capy_string_copy(headers->arena, value),
    };

    capy_http_field *another = capy_smap_get((capy_smap *)headers, name);

    if (another == NULL)
    {
        capy_smap_set((capy_smap *)headers, &field.name);
        return;
    }

    while (another->next)
    {
        another = another->next;
    }

    another->next = capy_arena_make(capy_http_field, headers->arena, 1);
    *another->next = field;
}

void capy_http_fields_clear(capy_http_fields *headers)
{
    capy_smap_clear((capy_smap *)headers);
}
