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

static int http_valid_characters(capy_string s, int categories, const char *chars)
{
    capy_string input = s;

    while (input.size)
    {
        if ((http_char_categories[(uint8_t)(input.data[0])] & categories) == 0)
        {
            if (chars == NULL)
            {
                return false;
            }

            for (int i = 0; chars[i] != input.data[0]; i++)
            {
                if (chars[i] == 0)
                {
                    return false;
                }
            }
        }

        input = capy_string_shl(input, 1);
    }

    return true;
}

static capy_http_method capy_http_parse_method(capy_string input)
{
    static const capy_string ONNECT = capy_string_literal("ONNECT");
    static const capy_string ELETE = capy_string_literal("ELETE");
    static const capy_string ET = capy_string_literal("ET");
    static const capy_string EAD = capy_string_literal("EAD");
    static const capy_string PTIONS = capy_string_literal("PTIONS");
    static const capy_string OST = capy_string_literal("OST");
    static const capy_string RACE = capy_string_literal("TRACE");

    if (input.size < 3 || input.size > 7)
    {
        return CAPY_HTTP_METHOD_UNK;
    }

    switch (input.data[0])
    {
        case 'C':
        {
            if (capy_string_eq(capy_string_shl(input, 1), ONNECT))
            {
                return CAPY_HTTP_CONNECT;
            }
        }
        break;

        case 'D':
        {
            if (capy_string_eq(capy_string_shl(input, 1), ELETE))
            {
                return CAPY_HTTP_DELETE;
            }
        }
        break;

        case 'G':
        {
            if (capy_string_eq(capy_string_shl(input, 1), ET))
            {
                return CAPY_HTTP_GET;
            }
        }
        break;

        case 'H':
        {
            if (capy_string_eq(capy_string_shl(input, 1), EAD))
            {
                return CAPY_HTTP_HEAD;
            }
        }
        break;

        case 'O':
        {
            if (capy_string_eq(capy_string_shl(input, 1), PTIONS))
            {
                return CAPY_HTTP_OPTIONS;
            }
        }
        break;

        case 'P':
        {
            switch (input.data[1])
            {
                case 'U':
                {
                    if (input.data[2] == 'T')
                    {
                        return CAPY_HTTP_PUT;
                    }
                }
                break;

                default:
                {
                    if (capy_string_eq(capy_string_shl(input, 1), OST))
                    {
                        return CAPY_HTTP_POST;
                    }
                }
                break;
            }
        }
        break;

        case 'T':
        {
            if (capy_string_eq(capy_string_shl(input, 1), RACE))
            {
                return CAPY_HTTP_TRACE;
            }
        }
        break;
    }

    return CAPY_HTTP_METHOD_UNK;
}

static capy_http_version capy_http_parse_version(capy_string input)
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

static capy_string http_message_get_token(capy_string *buffer, const char *delimiters, int ows)
{
    capy_string input = *buffer;

    size_t i, token_size = -1;

    for (i = 0; token_size == (size_t)(-1) && i < input.size; i++)
    {
        for (const char *it = delimiters; it[0] != 0; it++)
        {
            if (input.data[i] == it[0])
            {
                token_size = i;
                break;
            }
        }
    }

    if (token_size == (size_t)(-1))
    {
        token_size = i;
    }

    for (; ows && i < input.size && input.data[i] == ' '; i++)
    {
    }

    *buffer = capy_string_shl(input, i);

    return capy_string_slice(input, 0, token_size);
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

    response_size = snprintf(response_buffer,
                             response_size,
                             format,
                             response->status,
                             response->content.size,
                             (int)response->content.size,
                             response->content.data);

    return capy_string_bytes(response_buffer, response_size);
}

int capy_http_parse_request_line(capy_arena *arena, capy_string line, capy_http_request *request)
{
    capy_string method = http_message_get_token(&line, " ", false);

    if (method.size == 0)
    {
        return EINVAL;
    }

    capy_string uri = http_message_get_token(&line, " ", false);

    if (uri.size == 0)
    {
        return EINVAL;
    }

    capy_string version = http_message_get_token(&line, " ", false);

    if (version.size == 0)
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

    request->uri.scheme = capy_string_copy(arena, request->uri.scheme);
    request->uri.authority = capy_string_copy(arena, request->uri.authority);
    request->uri.userinfo = capy_string_copy(arena, request->uri.userinfo);
    request->uri.host = capy_string_copy(arena, request->uri.host);
    request->uri.port = capy_string_copy(arena, request->uri.port);
    request->uri.path = capy_string_copy(arena, request->uri.path);
    request->uri.query = capy_string_copy(arena, request->uri.query);
    request->uri.fragment = capy_string_copy(arena, request->uri.fragment);

    return 0;
}

int capy_http_content_length(capy_http_request *request)
{
    capy_http_field *content_length = capy_smap_get(request->headers, capy_string_literal("content-length"));

    if (content_length != NULL)
    {
        if (!http_valid_characters(content_length->value, HTTP_DIGIT, ""))
        {
            return EINVAL;
        }

        request->content_length = atoi(content_length->value.data);
    }

    capy_http_field *transfer_encoding = capy_smap_get(request->headers, capy_string_literal("transfer-encoding"));

    if (transfer_encoding != NULL)
    {
        capy_string input = transfer_encoding->value;

        while (input.size)
        {
            capy_string token = http_message_get_token(&input, ",", true);

            token = capy_string_trim(token);

            if (capy_string_eq(token, capy_string_literal("chunked")))
            {
                request->chunked = true;
                request->content_length = 0;
            }
        }
    }

    return 0;
}

int capy_http_parse_field(capy_arena *arena, capy_string line, capy_http_field **fields)
{
    capy_http_field field;

    field.name = http_message_get_token(&line, ":", false);

    if (field.name.size == 0)
    {
        return EINVAL;
    }

    if (!http_valid_characters(field.name, HTTP_TOKEN, NULL))
    {
        return EINVAL;
    }

    field.value = capy_string_trim(line);

    if (!http_valid_characters(field.value, HTTP_VCHAR, " \t"))
    {
        return EINVAL;
    }

    field.name = capy_string_tolower(arena, field.name);
    field.value = capy_string_copy(arena, field.value);

    capy_http_field *existing = capy_smap_get(*fields, field.name);

    if (existing != NULL && existing->value.size)
    {
        field.value = capy_string_join(arena, capy_string_literal(", "), 2, (capy_string[]){existing->value, field.value});
    }

    *fields = capy_smap_set(*fields, &field.name);

    return 0;
}
