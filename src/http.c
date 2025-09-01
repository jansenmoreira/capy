#include <capy/capy.h>
#include <errno.h>

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
        if ((http_char_categories[(uint8_t)(input.data[0])] & categories) == 0)
        {
            if (!capy_char_is(input.data[0], chars))
            {
                return false;
            }
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

capy_http_method capy_http_parse_method(capy_string input)
{
    static const capy_string ONNECT = capy_string_literal("ONNECT");
    static const capy_string ELETE = capy_string_literal("ELETE");
    static const capy_string ET = capy_string_literal("ET");
    static const capy_string EAD = capy_string_literal("EAD");
    static const capy_string PTIONS = capy_string_literal("PTIONS");
    static const capy_string UT = capy_string_literal("UT");
    static const capy_string OST = capy_string_literal("OST");
    static const capy_string RACE = capy_string_literal("RACE");

    if (input.size < 3 || input.size > 7)
    {
        return CAPY_HTTP_METHOD_UNK;
    }

    capy_string suffix = capy_string_shl(input, 1);

    switch (input.data[0])
    {
        case 'C':
        {
            if (capy_string_eq(suffix, ONNECT))
            {
                return CAPY_HTTP_CONNECT;
            }
        }
        break;

        case 'D':
        {
            if (capy_string_eq(suffix, ELETE))
            {
                return CAPY_HTTP_DELETE;
            }
        }
        break;

        case 'G':
        {
            if (capy_string_eq(suffix, ET))
            {
                return CAPY_HTTP_GET;
            }
        }
        break;

        case 'H':
        {
            if (capy_string_eq(suffix, EAD))
            {
                return CAPY_HTTP_HEAD;
            }
        }
        break;

        case 'O':
        {
            if (capy_string_eq(suffix, PTIONS))
            {
                return CAPY_HTTP_OPTIONS;
            }
        }
        break;

        case 'P':
        {
            if (capy_string_eq(suffix, UT))
            {
                return CAPY_HTTP_PUT;
            }
            else if (capy_string_eq(suffix, OST))
            {
                return CAPY_HTTP_POST;
            }
        }
        break;

        case 'T':
        {
            if (capy_string_eq(suffix, RACE))
            {
                return CAPY_HTTP_TRACE;
            }
        }
        break;

        default:
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

    if (strncmp(input.data, "HTTP/", 5) != 0)
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
                default:
                    break;
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

        default:
            break;
    }

    return CAPY_HTTP_VERSION_UNK;
}

capy_string capy_http_write_response(capy_arena *arena, capy_http_response *response)
{
    size_t response_size = 8 * 1024;
    char *response_buffer = capy_arena_make(char, arena, 8 * 1024);

    const char *format =
        "HTTP/1.1 %d\r\n"
        "Content-Length: %llu\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "%.*s";

    response_size = (size_t)(snprintf(response_buffer,
                                      response_size,
                                      format,
                                      response->status,
                                      response->content.size,
                                      (int)response->content.size,
                                      response->content.data));

    return capy_string_bytes(response_buffer, response_size);
}

int capy_http_parse_request_line(capy_arena *arena, capy_string line, capy_http_request *request)
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

    request->uri = capy_uri_parse(uri);

    if (!capy_uri_valid(request->uri))
    {
        return EINVAL;
    }

    int err = 0;

    err |= capy_string_copy(arena, request->uri.scheme, &request->uri.scheme);
    err |= capy_string_copy(arena, request->uri.authority, &request->uri.authority);
    err |= capy_string_copy(arena, request->uri.userinfo, &request->uri.userinfo);
    err |= capy_string_copy(arena, request->uri.host, &request->uri.host);
    err |= capy_string_copy(arena, request->uri.port, &request->uri.port);
    err |= capy_string_copy(arena, request->uri.path, &request->uri.path);
    err |= capy_string_copy(arena, request->uri.query, &request->uri.query);
    err |= capy_string_copy(arena, request->uri.fragment, &request->uri.fragment);

    if (err)
    {
        return ENOMEM;
    }

    return 0;
}

int capy_http_content_attributes(capy_http_request *request)
{
    request->content_length = 0;
    request->chunked = 0;

    capy_http_field *transfer_encoding = capy_smap_get(request->headers, capy_string_literal("transfer-encoding"));

    if (transfer_encoding != NULL)
    {
        capy_string input = transfer_encoding->value;

        while (input.size)
        {
            capy_string token = http_next_token(&input, ",");

            token = capy_string_trim(token, " \t");

            if (capy_string_eq(token, capy_string_literal("chunked")))
            {
                request->chunked = true;
                request->content_length = 0;
            }

            http_consume_chars(&input, ",");
        }
    }

    capy_http_field *content_length = capy_smap_get(request->headers, capy_string_literal("content-length"));

    if (content_length != NULL)
    {
        if (request->chunked || !http_string_validate(content_length->value, HTTP_DIGIT, ""))
        {
            return EINVAL;
        }

        request->content_length = strtoull(content_length->value.data, NULL, 10);
        request->chunked = 0;
    }

    return 0;
}

int capy_http_parse_field(capy_arena *arena, capy_string line, capy_http_field **fields)
{
    capy_http_field field;

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

    if (!http_string_validate(field.value, HTTP_VCHAR, NULL))
    {
        return EINVAL;
    }

    int err = 0;

    err |= capy_string_tolower(arena, field.name, &field.name);
    err |= capy_string_copy(arena, field.value, &field.value);

    capy_http_field *other = capy_smap_get(*fields, field.name);

    if (other)
    {
        err |= capy_string_join(arena,
                                capy_string_literal(", "),
                                2, (capy_string[]){other->value, field.value},
                                &field.value);
    }

    if (err)
    {
        return ENOMEM;
    }

    capy_http_field *tmp = capy_smap_set(*fields, &field.name);

    if (tmp == NULL)
    {
        return ENOMEM;
    }

    *fields = tmp;

    return 0;
}
