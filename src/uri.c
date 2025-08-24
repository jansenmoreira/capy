#include <capy/capy.h>

#define URI_GEN_DELIM 0x1
#define URI_SUB_DELIM 0x2
#define URI_UNRESERVED 0x4
#define URI_SCHEME_BEGIN 0x8
#define URI_SCHEME_CONTINUE 0x10
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
    ['+'] = URI_SUB_DELIM | URI_SCHEME_CONTINUE,
    [','] = URI_SUB_DELIM,
    [';'] = URI_SUB_DELIM,
    ['='] = URI_SUB_DELIM,

    ['-'] = URI_UNRESERVED | URI_SCHEME_CONTINUE,
    ['.'] = URI_UNRESERVED | URI_SCHEME_CONTINUE,
    ['_'] = URI_UNRESERVED,
    ['~'] = URI_UNRESERVED,

    ['0'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['1'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['2'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['3'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['4'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['5'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['6'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['7'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['8'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,
    ['9'] = URI_UNRESERVED | URI_SCHEME_CONTINUE | URI_HEXDIGIT | URI_DIGIT,

    ['a'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['b'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['c'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['d'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['e'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['f'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['g'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['h'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['i'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['j'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['k'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['l'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['m'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['n'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['o'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['p'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['q'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['r'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['s'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['t'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['u'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['v'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['w'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['x'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['y'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['z'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['A'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['B'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['C'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['D'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['E'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['F'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE | URI_HEXDIGIT,
    ['G'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['H'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['I'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['J'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['K'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['L'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['M'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['N'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['O'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['P'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['Q'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['R'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['S'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['T'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['U'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['V'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['W'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['X'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['Y'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
    ['Z'] = URI_UNRESERVED | URI_SCHEME_BEGIN | URI_SCHEME_CONTINUE,
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

    return (input.size = i, input);
}

capy_string uri_parse_dec_octet(capy_string input)
{
    switch (input.data[0])
    {
        case '0':
            return (input.size = 1, input);

        case '1':
        case '2':
        {
            if (input.size >= 2 && uri_char_categories[input.data[1]] & URI_DIGIT)
            {
                if (input.size >= 3 && uri_char_categories[input.data[2]] & URI_DIGIT)
                {
                    return (input.size = 3, input);
                }

                return (input.size = 2, input);
            }

            return (input.size = 1, input);
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
                return (input.size = 2, input);
            }

            return (input.size = 1, input);
        }

        default:
            return (input.size = 0, input);
    }
}

capy_string uri_token_advance(capy_string input)
{
    capy_assert(input.size);  // GCOVR_EXCL_LINE

    if (input.data[0] == '%')
    {
        return capy_string_shl(input, 3);
    }
    else
    {
        return capy_string_shl(input, 1);
    }
}

int uri_token_categories(capy_string input)
{
    capy_assert(input.size);  // GCOVR_EXCL_LINE

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

static inline int uri_valid_userinfo(capy_string userinfo)
{
    return uri_valid_tokens(userinfo, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, ":");
}

static inline int uri_valid_port(capy_string port)
{
    return uri_valid_tokens(port, URI_DIGIT, "");
}

static inline int uri_valid_fragment(capy_string fragment)
{
    return uri_valid_tokens(fragment, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, ":@/?");
}

static inline int uri_valid_query(capy_string query)
{
    return uri_valid_tokens(query, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, ":@/?");
}

static inline int uri_valid_reg_name(capy_string host)
{
    return uri_valid_tokens(host, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, "");
}

static inline int uri_valid_segment(capy_string segment)
{
    return uri_valid_tokens(segment, URI_UNRESERVED | URI_PCT_ENCODED | URI_SUB_DELIM, ":@");
}

static int uri_valid_ipv4(capy_string host)
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

            input = capy_string_shl(input, 1);
        }

        capy_string dec_octet = uri_parse_dec_octet(input);

        if (dec_octet.size == 0)
        {
            return 0;
        }

        input = capy_string_shl(input, dec_octet.size);
    }

    return (input.size == 0) ? host.size : 0;
}

static int uri_valid_ipv6(capy_string host)
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
                if (short_form)
                {
                    suffix += 2;
                }
                else
                {
                    prefix += 2;
                }

                input = capy_string_shl(input, input.size);
                break;
            }
        }

        capy_string hex_word = uri_parse_hex_word(input);
        input = capy_string_shl(input, hex_word.size);

        if (hex_word.size)
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

        input = capy_string_shl(input, 1);

        if (input.size && input.data[0] == ':')
        {
            if (!short_form)
            {
                short_form = 1;
                input = capy_string_shl(input, 1);
            }
            else
            {
                return 0;
            }
        }
    }

    if (!short_form && prefix != 8)
    {
        return 0;
    }

    if (input.size != 0)
    {
        return 0;
    }

    return host.size;
}

static int uri_valid_host(capy_string host)
{
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

static int uri_valid_scheme(capy_string scheme)
{
    capy_string input = scheme;

    if ((uri_token_categories(input) & URI_SCHEME_BEGIN) == 0)
    {
        return 0;
    }

    input = uri_token_advance(input);

    while (input.size)
    {
        if ((uri_token_categories(input) & URI_SCHEME_CONTINUE) == 0)
        {
            return 0;
        }

        input = uri_token_advance(input);
    }

    return scheme.size;
}

static int uri_valid_path(capy_string path)
{
    capy_string input = path;

    int segment_size = 0;

    while (segment_size < input.size)
    {
        if (input.data[segment_size] == '/')
        {
            if (segment_size && !uri_valid_segment(capy_string_slice(input, 0, segment_size)))
            {
                return 0;
            }

            input = capy_string_shl(input, segment_size + 1);
            segment_size = 0;
        }
        else
        {
            segment_size += 1;
        }
    }

    if (segment_size && !uri_valid_segment(input))
    {
        return 0;
    }

    return path.size;
}

static capy_string uri_path_merge(capy_arena *arena, capy_string base, capy_string reference)
{
    while (base.size)
    {
        if (base.data[base.size - 1] == '/')
        {
            base = capy_string_shr(base, 1);
            break;
        }

        base = capy_string_shr(base, 1);
    }

    size_t size = base.size + 1 + reference.size;
    char *buffer = capy_arena_make(char, arena, size + 1);

    strncpy(buffer, base.data, base.size);
    buffer[base.size] = '/';
    strncpy(buffer + base.size + 1, reference.data, reference.size);

    return capy_string_bytes(buffer, size);
}

static capy_string uri_path_remove_dot_segments(capy_arena *arena, capy_string path)
{
    capy_string path_self = capy_string_literal("./");
    capy_string path_parent = capy_string_literal("../");
    capy_string path_dot = capy_string_literal(".");
    capy_string path_dots = capy_string_literal("..");

    if (path.size == 0)
    {
        return path;
    }

    capy_string input = path;

    char *output = capy_arena_make(char, arena, path.size + 1);
    size_t output_size = 0;

    while (input.size)
    {
        size_t size = input.size;

        for (int i = 0; i < input.size; i++)
        {
            if (input.data[i] == '/')
            {
                size = i + 1;
                break;
            }
        }

        capy_string segment = capy_string_slice(input, 0, size);
        input = capy_string_shl(input, size);

        if (capy_string_eq(segment, path_self) || capy_string_eq(segment, path_dot))
        {
        }
        else if (capy_string_eq(segment, path_parent) || capy_string_eq(segment, path_dots))
        {
            // it's impossible to generate an output of size 1 other than "/"
            if (output_size == 1)
            {
                continue;
            }

            for (output_size -= 1; output_size > 0 && output[output_size - 1] != '/'; output_size -= 1)
            {
            }

            output[output_size] = 0;
        }
        else
        {
            memcpy(output + output_size, segment.data, segment.size);
            output_size += segment.size;
        }
    }

    capy_arena_shrink(arena, output + output_size + 1);

    return capy_string_bytes(output, output_size);
}

capy_uri capy_uri_parse(capy_string input)
{
    capy_uri uri = {NULL};

    for (int i = 0; i < input.size; i++)
    {
        char c = input.data[i];

        if (c == '/' || c == '?' || c == '#')
        {
            break;
        }
        else if (c == ':')
        {
            uri.scheme = capy_string_slice(input, 0, i);
            input = capy_string_shl(input, i + 1);

            break;
        }
    }

    if (input.size > 1 && input.data[0] == '/' && input.data[1] == '/')
    {
        input = capy_string_shl(input, 2);

        int i;

        for (i = 0; i < input.size; i++)
        {
            char c = input.data[i];

            if (c == '/' || c == '?' || c == '#')
            {
                break;
            }
        }

        uri.authority = capy_string_slice(input, 0, i);
        input = capy_string_shl(input, i);
    }

    if (input.size)
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

        uri.path = capy_string_slice(input, 0, i);
        input = capy_string_shl(input, i);
    }

    if (input.size && input.data[0] == '?')
    {
        input = capy_string_shl(input, 1);

        int i;

        for (i = 0; i < input.size && input.data[i] != '#'; i++)
        {
        }

        uri.query = capy_string_slice(input, 0, i);
        input = capy_string_shl(input, i);
    }

    if (input.size)
    {
        uri.fragment = capy_string_shl(input, 1);
        input = (capy_string){NULL};
    }

    uri.host = uri.authority;

    for (int i = 0; i < uri.host.size; i++)
    {
        if (uri.host.data[i] == '@')
        {
            uri.userinfo = capy_string_slice(uri.authority, 0, i);
            uri.host = capy_string_shl(uri.host, i + 1);
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
            uri.port = capy_string_shl(uri.host, i + 1);
            uri.host = capy_string_slice(uri.host, 0, i);
            break;
        }
    }

    return uri;
}

int capy_uri_valid(capy_uri uri)
{
    if (uri.scheme.size && !uri_valid_scheme(uri.scheme))
    {
        return false;
    }

    if (uri.userinfo.size && !uri_valid_userinfo(uri.userinfo))
    {
        return false;
    }

    if (uri.port.size && !uri_valid_port(uri.port))
    {
        return false;
    }

    if (uri.host.size && !uri_valid_host(uri.host))
    {
        return false;
    }

    if (uri.path.size && !uri_valid_path(uri.path))
    {
        return false;
    }

    if (uri.query.size && !uri_valid_query(uri.query))
    {
        return false;
    }

    if (uri.fragment.size && !uri_valid_fragment(uri.fragment))
    {
        return false;
    }

    return true;
}

capy_uri capy_uri_resolve_reference(capy_arena *arena, capy_uri base, capy_uri reference)
{
    capy_uri uri = {NULL};

    if (reference.scheme.size)
    {
        uri.scheme = reference.scheme;
        uri.authority = reference.authority;
        uri.userinfo = reference.userinfo;
        uri.host = reference.host;
        uri.port = reference.port;
        uri.path = uri_path_remove_dot_segments(arena, reference.path);
        uri.query = reference.query;
    }
    else
    {
        if (reference.authority.size)
        {
            uri.authority = reference.authority;
            uri.userinfo = reference.userinfo;
            uri.host = reference.host;
            uri.port = reference.port;
            uri.path = uri_path_remove_dot_segments(arena, reference.path);
            uri.query = reference.query;
        }
        else
        {
            if (reference.path.size == 0)
            {
                uri.path = base.path;
                uri.query = (reference.query.size) ? reference.query : base.query;
            }
            else
            {
                if (reference.path.data[0] == '/')
                {
                    uri.path = uri_path_remove_dot_segments(arena, reference.path);
                }
                else
                {
                    uri.path = uri_path_merge(arena, base.path, reference.path);
                    uri.path = uri_path_remove_dot_segments(arena, uri.path);
                }

                uri.query = reference.query;
            }

            uri.authority = base.authority;
            uri.userinfo = base.userinfo;
            uri.host = base.host;
            uri.port = base.port;
        }

        uri.scheme = base.scheme;
    }

    uri.fragment = reference.fragment;

    return uri;
}

capy_string capy_uri_string(capy_arena *arena, capy_uri uri)
{
    size_t buffer_max = uri.scheme.size +
                        uri.authority.size +
                        uri.path.size +
                        uri.query.size +
                        uri.fragment.size +
                        6;

    char *buffer = capy_arena_make(char, arena, buffer_max);
    size_t buffer_size = 0;

    if (uri.scheme.size)
    {
        memcpy(buffer + buffer_size, uri.scheme.data, uri.scheme.size);
        buffer_size += uri.scheme.size;

        buffer[buffer_size++] = ':';
    }

    if (uri.authority.size)
    {
        buffer[buffer_size++] = '/';
        buffer[buffer_size++] = '/';

        memcpy(buffer + buffer_size, uri.authority.data, uri.authority.size);
        buffer_size += uri.authority.size;
    }

    memcpy(buffer + buffer_size, uri.path.data, uri.path.size);
    buffer_size += uri.path.size;

    if (uri.query.size)
    {
        buffer[buffer_size++] = '?';

        memcpy(buffer + buffer_size, uri.query.data, uri.query.size);
        buffer_size += uri.query.size;
    }

    if (uri.fragment.size)
    {
        buffer[buffer_size++] = '#';

        memcpy(buffer + buffer_size, uri.fragment.data, uri.fragment.size);
        buffer_size += uri.fragment.size;
    }

    return capy_string_bytes(buffer, buffer_size);
}
