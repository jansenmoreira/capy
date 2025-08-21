#include <capy/capy.h>
#include <stdbool.h>

#include "capy/string.h"

#define URI_GEN_DELIM 0x1
#define URI_SUB_DELIM 0x2
#define URI_UNRESERVED 0x4
#define URI_SCHEMA_BEGIN 0x8
#define URI_SCHEMA_CONTINUE 0x10
#define URI_HEXDIGIT 0x20
#define URI_DIGIT 0x40
#define URI_PCT_ENCODED 0x80

static uint8_t uri_char_categories[256] = {
    [':'] = URI_GEN_DELIM,
    ['/'] = URI_GEN_DELIM,
    ['?'] = URI_GEN_DELIM,
    ['#'] = URI_GEN_DELIM,
    ['['] = URI_GEN_DELIM,
    [']'] = URI_GEN_DELIM,
    ['@'] = URI_GEN_DELIM,

    ['!'] = URI_SUB_DELIM,
    ['$'] = URI_SUB_DELIM,
    ['&'] = URI_SUB_DELIM,
    ['\''] = URI_SUB_DELIM,
    ['('] = URI_SUB_DELIM,
    [')'] = URI_SUB_DELIM,
    ['*'] = URI_SUB_DELIM,
    ['+'] = URI_SUB_DELIM | URI_SCHEMA_CONTINUE,
    [','] = URI_SUB_DELIM,
    [';'] = URI_SUB_DELIM,
    ['='] = URI_SUB_DELIM,

    ['-'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE,
    ['.'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE,
    ['_'] = URI_UNRESERVED,
    ['~'] = URI_UNRESERVED,

    ['0'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['1'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['2'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['3'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['4'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['5'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['6'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['7'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['8'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['9'] = URI_UNRESERVED | URI_SCHEMA_CONTINUE | URI_HEXDIGIT | URI_DIGIT,

    ['a'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['b'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['c'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['d'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['e'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['f'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['g'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['h'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['i'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['j'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['k'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['l'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['m'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['n'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['o'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['p'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['q'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['r'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['s'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['t'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['u'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['v'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['w'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['x'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['y'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['z'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['A'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['B'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['C'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['D'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['E'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['F'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE | URI_HEXDIGIT,
    ['G'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['H'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['I'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['J'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['K'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['L'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['M'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['N'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['O'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['P'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['Q'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['R'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['S'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['T'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['U'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['V'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['W'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['X'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['Y'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
    ['Z'] = URI_UNRESERVED | URI_SCHEMA_BEGIN | URI_SCHEMA_CONTINUE,
};

capy_string uri_parse_hex_word(capy_string input)
{
    int i;

    for (i = 0; i < 4 && i < input.size; i++)
    {
        if ((uri_char_categories[input.data[i]] & URI_HEXDIGIT) == 0)
        {
            break;
        }
    }

    return capy_string_resize(input, i);
}

capy_string uri_parse_dec_octet(capy_string input)
{
    switch (input.data[0])
    {
        case '0':
        {
            return capy_string_resize(input, 1);
        }

        case '1':
        case '2':
        {
            if (input.size >= 2 && uri_char_categories[input.data[1]] & URI_DIGIT)
            {
                if (input.size >= 3 && uri_char_categories[input.data[2]] & URI_DIGIT)
                {
                    return capy_string_resize(input, 3);
                }

                return capy_string_resize(input, 2);
            }

            return capy_string_resize(input, 1);
        }

        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            if (input.size >= 2 && uri_char_categories[input.data[1]] & URI_DIGIT)
            {
                return capy_string_resize(input, 2);
            }

            return capy_string_resize(input, 1);
        }

        default:
        {
            return capy_string_resize(input, 0);
        }
    }
}

capy_string uri_token_advance(capy_string input)
{
    if (input.size == 0)
    {
        return input;
    }
    else if (input.data[0] == '%')
    {
        return capy_string_shrinkl(input, 3);
    }
    else
    {
        return capy_string_shrinkl(input, 1);
    }
}

int uri_token_categories(capy_string input)
{
    if (!input.size)
    {
        return 0;
    }

    if (input.data[0] != '%')
    {
        return uri_char_categories[input.data[0]];
    }

    if (input.size < 3 ||
        !(uri_char_categories[input.data[1]] & URI_HEXDIGIT) ||
        !(uri_char_categories[input.data[2]] & URI_HEXDIGIT))
    {
        return 0;
    }

    return URI_PCT_ENCODED;
}

int uri_valid_tokens(capy_string s, int categories, const char *chars)
{
    capy_string input = s;

    while (input.size)
    {
        if ((uri_token_categories(input) & categories) == 0)
        {
            for (int i = 0; chars[i] != input.data[0]; i++)
            {
                if (chars[i] == 0)
                {
                    return 0;
                }
            }
        }

        input = uri_token_advance(input);
    }

    return s.size;
}

int uri_valid_userinfo(capy_string userinfo)
{
    return uri_valid_tokens(userinfo, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, ":");
}

int uri_valid_port(capy_string port)
{
    return uri_valid_tokens(port, URI_DIGIT, "");
}

int uri_valid_fragment(capy_string fragment)
{
    return uri_valid_tokens(fragment, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, ":@/?");
}

int uri_valid_query(capy_string query)
{
    return uri_valid_tokens(query, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, ":@/?");
}

int uri_valid_reg_name(capy_string host)
{
    return uri_valid_tokens(host, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, "");
}

int uri_valid_ipv4(capy_string host)
{
    capy_string input = host;

    for (int i = 0; i < 4; i++)
    {
        if (i != 0)
        {
            if (input.data[0] != '.')
            {
                return 0;
            }

            input = capy_string_shrinkl(input, 1);
        }

        capy_string dec_octet = uri_parse_dec_octet(input);

        if (dec_octet.size == 0)
        {
            return 0;
        }

        input = capy_string_shrinkl(input, dec_octet.size);
    }

    return (input.size == 0) ? host.size : 0;
}

int uri_valid_ipv6(capy_string host)
{
    capy_string input = host;

    int prefix = 0;
    int suffix = 0;
    int short_form = 0;

    for (int i = 0; prefix + suffix + short_form < 8 && input.size; i++)
    {
        if ((!short_form && prefix == 6) || (short_form && suffix < 6))
        {
            if (uri_valid_ipv4(input))
            {
                input = capy_string_shrinkl(input, input.size);
                break;
            }
        }

        capy_string hex_word = uri_parse_hex_word(input);
        input = capy_string_shrinkl(input, hex_word.size);

        if (hex_word.size > 0)
        {
            if (short_form)
            {
                suffix += 1;
            }
            else
            {
                prefix += 1;
            }
        }

        if ((input.size == 0) || (input.data[0] != ':'))
        {
            break;
        }

        input = capy_string_shrinkl(input, 1);

        if (input.size && input.data[0] == ':')
        {
            if (!short_form)
            {
                short_form = 1;
                input = capy_string_shrinkl(input, 1);
            }
            else
            {
                return 0;
            }
        }
    }

    return (input.size == 0) ? host.size : 0;
}

int uri_valid_host(capy_string host)
{
    if (host.size == 0)
    {
        return 0;
    }

    if (host.data[0] == '[' && host.data[host.size - 1] == ']')
    {
        capy_string input = {.data = host.data + 1, .size = host.size - 2};

        if (uri_valid_ipv6(input))
        {
            return host.size;
        }

        return 0;
    }

    if (uri_valid_ipv4(host))
    {
        return host.size;
    }

    return uri_valid_reg_name(host);
}

int uri_valid_schema(capy_string schema)
{
    capy_string input = schema;

    if (input.size > 0 && (uri_token_categories(input) & URI_SCHEMA_BEGIN) == 0)
    {
        return 0;
    }

    input = uri_token_advance(input);

    while (input.size)
    {
        if (input.size > 0 && (uri_token_categories(input) & URI_SCHEMA_CONTINUE) == 0)
        {
            return 0;
        }

        input = uri_token_advance(input);
    }

    return schema.size;
}

static inline capy_URI uri_split_components(capy_string input)
{
    capy_URI uri = {NULL};

    for (int i = 0; i < input.size; i++)
    {
        char c = input.data[i];

        if (c == '/' || c == '?' || c == '#')
        {
            break;
        }
        else if (c == ':')
        {
            uri.schema = capy_string_resize(input, i);
            input = capy_string_shrinkl(input, i + 1);

            break;
        }
    }

    if (input.size > 1 && input.data[0] == '/' && input.data[1] == '/')
    {
        input = capy_string_shrinkl(input, 2);

        int i;

        for (i = 0; i < input.size; i++)
        {
            char c = input.data[i];

            if (c == '/' || c == '?' || c == '#')
            {
                break;
            }
        }

        uri.authority = capy_string_resize(input, i);
        input = capy_string_shrinkl(input, i);
    }

    if (input.size > 0)
    {
        int i;

        for (i = 0; i < input.size; i++)
        {
            char c = input.data[i];

            if (c == '?' || c == '#')
            {
                break;
            }
        }

        uri.path = capy_string_resize(input, i);
        input = capy_string_shrinkl(input, i);
    }

    if (input.size > 0 && input.data[0] == '?')
    {
        input = capy_string_shrinkl(input, 1);

        int i;

        for (i = 0; i < input.size && input.data[i] != '#'; i++)
        {
        }

        uri.query = capy_string_resize(input, i);
        input = capy_string_shrinkl(input, i);
    }

    if (input.size > 0 && input.data[0] == '#')
    {
        uri.fragment = capy_string_shrinkl(input, 1);
        input = (capy_string){NULL};
    }

    uri.host = uri.authority;

    for (int i = 0; i < uri.host.size; i++)
    {
        if (uri.host.data[i] == '@')
        {
            uri.userinfo = capy_string_resize(uri.authority, i);
            uri.host = capy_string_shrinkl(uri.host, i + 1);
            break;
        }
    }

    for (int i = uri.host.size - 1; i >= 0; i--)
    {
        char c = uri.host.data[i];

        if (c == ']')
        {
            break;
        }
        else if (c == ':')
        {
            uri.port = capy_string_shrinkl(uri.host, i + 1);
            uri.host = capy_string_resize(uri.host, i);
            break;
        }
    }

    return uri;
}

int uri_valid(capy_URI *uri)
{
    if (uri->schema.size > 0 && !uri_valid_schema(uri->schema))
    {
        return false;
    }

    if (uri->userinfo.size > 0 && !uri_valid_userinfo(uri->userinfo))
    {
        return false;
    }

    if (uri->port.size > 0 && !uri_valid_port(uri->port))
    {
        return false;
    }

    if (uri->host.size > 0 && !uri_valid_host(uri->host))
    {
        return false;
    }

    if (uri->query.size > 0 && !uri_valid_query(uri->query))
    {
        return false;
    }

    if (uri->fragment.size > 0 && !uri_valid_fragment(uri->fragment))
    {
        return false;
    }

    return true;
}

capy_URI *capy_URI_decode(capy_arena *arena, capy_string input)
{
    capy_URI tmp = uri_split_components(input);

    if (!uri_valid(&tmp))
    {
        return NULL;
    }

    tmp.schema = capy_string_copy(arena, tmp.schema);
    tmp.authority = capy_string_copy(arena, tmp.authority);
    tmp.userinfo = capy_string_copy(arena, tmp.userinfo);
    tmp.host = capy_string_copy(arena, tmp.host);
    tmp.port = capy_string_copy(arena, tmp.port);
    tmp.path = capy_string_copy(arena, tmp.path);
    tmp.query = capy_string_copy(arena, tmp.query);
    tmp.fragment = capy_string_copy(arena, tmp.fragment);

    capy_URI *uri = capy_arena_make(capy_URI, arena, 1);

    uri[0] = tmp;

    return uri;
}
