#include "capy/http.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <capy/capy.h>
#include <errno.h>

#include "capy/string.h"
#include "capy/uri.h"

#define HTTP_VCHAR 0x1
#define HTTP_TOKEN 0x2

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
    ['0'] = HTTP_VCHAR | HTTP_TOKEN,
    ['1'] = HTTP_VCHAR | HTTP_TOKEN,
    ['2'] = HTTP_VCHAR | HTTP_TOKEN,
    ['3'] = HTTP_VCHAR | HTTP_TOKEN,
    ['4'] = HTTP_VCHAR | HTTP_TOKEN,
    ['5'] = HTTP_VCHAR | HTTP_TOKEN,
    ['6'] = HTTP_VCHAR | HTTP_TOKEN,
    ['7'] = HTTP_VCHAR | HTTP_TOKEN,
    ['8'] = HTTP_VCHAR | HTTP_TOKEN,
    ['9'] = HTTP_VCHAR | HTTP_TOKEN,
    [':'] = HTTP_VCHAR,
    [';'] = HTTP_VCHAR,
    ['<'] = HTTP_VCHAR,
    ['='] = HTTP_VCHAR,
    ['>'] = HTTP_VCHAR,
    ['?'] = HTTP_VCHAR,
    ['@'] = HTTP_VCHAR,
    ['A'] = HTTP_VCHAR | HTTP_TOKEN,
    ['B'] = HTTP_VCHAR | HTTP_TOKEN,
    ['C'] = HTTP_VCHAR | HTTP_TOKEN,
    ['D'] = HTTP_VCHAR | HTTP_TOKEN,
    ['E'] = HTTP_VCHAR | HTTP_TOKEN,
    ['F'] = HTTP_VCHAR | HTTP_TOKEN,
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
    ['a'] = HTTP_VCHAR | HTTP_TOKEN,
    ['b'] = HTTP_VCHAR | HTTP_TOKEN,
    ['c'] = HTTP_VCHAR | HTTP_TOKEN,
    ['d'] = HTTP_VCHAR | HTTP_TOKEN,
    ['e'] = HTTP_VCHAR | HTTP_TOKEN,
    ['f'] = HTTP_VCHAR | HTTP_TOKEN,
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
    static const capy_string OST = capy_string_literal("ST");
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
                return CAPY_HTTP_METHOD_CONNECT;
            }
        }
        break;

        case 'D':
        {
            if (capy_string_eq(capy_string_shl(input, 1), ELETE))
            {
                return CAPY_HTTP_METHOD_DELETE;
            }
        }
        break;

        case 'G':
        {
            if (capy_string_eq(capy_string_shl(input, 1), ET))
            {
                return CAPY_HTTP_METHOD_GET;
            }
        }
        break;

        case 'H':
        {
            if (capy_string_eq(capy_string_shl(input, 1), EAD))
            {
                return CAPY_HTTP_METHOD_HEAD;
            }
        }
        break;

        case 'O':
        {
            if (capy_string_eq(capy_string_shl(input, 1), PTIONS))
            {
                return CAPY_HTTP_METHOD_OPTIONS;
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
                        return CAPY_HTTP_METHOD_PUT;
                    }
                }
                break;

                default:
                {
                    if (capy_string_eq(capy_string_shl(input, 3), OST))
                    {
                        return CAPY_HTTP_METHOD_POST;
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
                return CAPY_HTTP_METHOD_TRACE;
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
                return CAPY_HTTP_VERSION_09;
            }
        }
        break;

        case '1':
        {
            switch (input.data[7])
            {
                case '0':
                    return CAPY_HTTP_VERSION_10;
                case '1':
                    return CAPY_HTTP_VERSION_11;
            }
        }
        break;

        case '2':
        {
            if (input.data[7] == '0')
            {
                return CAPY_HTTP_VERSION_20;
            }
        }
        break;

        case '3':
        {
            if (input.data[7] == '0')
            {
                return CAPY_HTTP_VERSION_30;
            }
        }
        break;
    }

    return CAPY_HTTP_VERSION_UNK;
}

static int http_message_read_line(capy_string *buffer, capy_string *line, size_t limit)
{
    size_t i = 0;
    capy_string input = *buffer;

    for (i = 0; input.size && input.data[i] != '\n'; i++)
    {
        if (input.size == limit)
        {
            return EINVAL;
        }
    }

    if (input.data[i - 1] == '\r')
    {
        *line = capy_string_slice(input, 0, i - 1ULL);
    }
    else
    {
        *line = capy_string_slice(input, 0, i);
    }

    *buffer = capy_string_shl(input, i + 1);

    return 0;
}

static int http_message_get_token(capy_string *buffer, capy_string *token, const char *delimiters, int ows)
{
    capy_string input = *buffer;

    size_t i, token_size = 0;

    for (i = 0; !token_size && i < input.size; i++)
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

    if (token_size == 0)
    {
        token_size = i;
    }

    for (; ows && i < input.size && input.data[i] == ' '; i++)
    {
    }

    *token = capy_string_slice(input, 0, token_size);
    *buffer = capy_string_shl(input, i);
    return 0;
}

capy_http_request *capy_http_parse_header(capy_arena *arena, capy_string input, size_t line_limit)
{
    capy_string line = {NULL};
    capy_string method = {NULL};
    capy_string uri = {NULL};
    capy_string version = {NULL};

    if (http_message_read_line(&input, &line, line_limit))
    {
        return NULL;
    }

    if (http_message_get_token(&line, &method, " ", false))
    {
        return NULL;
    }

    if (http_message_get_token(&line, &uri, " ", false))
    {
        return NULL;
    }

    if (http_message_get_token(&line, &version, " ", false))
    {
        return NULL;
    }

    capy_http_request *request = capy_arena_make(capy_http_request, arena, 1);
    request->method = capy_http_parse_method(method);
    request->uri = capy_uri_parse(uri);
    request->version = capy_http_parse_version(version);
    request->headers = capy_smap_of(capy_http_field, arena, 16);

    for (;;)
    {
        if (http_message_read_line(&input, &line, line_limit))
        {
            return NULL;
        }

        if (line.size == 0)
        {
            break;
        }

        capy_http_field field;

        if (http_message_get_token(&line, &field.name, ":", false))
        {
            return NULL;
        }

        if (!http_valid_characters(field.name, HTTP_TOKEN, NULL))
        {
            return NULL;
        }

        field.value = capy_string_trim(line);

        if (!http_valid_characters(field.value, HTTP_VCHAR, " \t"))
        {
            return NULL;
        }

        field.name = capy_string_tolower(arena, field.name);
        field.value = capy_string_copy(arena, field.value);

        capy_http_field *header = capy_smap_get(request->headers, field.name);

        if (header != NULL && header->value.size)
        {
            field.value = capy_string_join(arena, capy_string_literal(", "), 2, (capy_string[]){header->value, field.value});
        }

        capy_smap_set(request->headers, &field.name);
    }

    return request;
}
