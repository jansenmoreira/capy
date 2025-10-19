#include "capy.h"

enum
{
    URI_GEN_DELIM = (1 << 0),
    URI_SUB_DELIM = (1 << 1),
    URI_UNRESERVED = (1 << 2),
    URI_SCHEME_BEGIN = (1 << 3),
    URI_SCHEME_CONTINUE = (1 << 4),
    URI_HEXDIGIT = (1 << 5),
    URI_DIGIT = (1 << 6),
    URI_PCT_ENCODED = (1 << 7),
};

// INTERNAL VARIABLES

static uint8_t uri_char_categories_[256] = {
    ['#'] = URI_GEN_DELIM,
    ['/'] = URI_GEN_DELIM,
    [':'] = URI_GEN_DELIM,
    ['?'] = URI_GEN_DELIM,
    ['@'] = URI_GEN_DELIM,
    ['['] = URI_GEN_DELIM,
    [']'] = URI_GEN_DELIM,

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

    ['"'] = 0,
    ['%'] = 0,
    ['<'] = 0,
    ['>'] = 0,
    ['\\'] = 0,
    ['^'] = 0,
    ['`'] = 0,
    ['{'] = 0,
    ['|'] = 0,
    ['}'] = 0,

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
};

// INTERNAL DEFINITIONS

uint8_t capy_uri_char_categories_(char c)
{
    return uri_char_categories_[(uint8_t)(c)];
}

capy_string capy_uri_pctenc_shl_(capy_string input)
{
    capy_assert(input.size);

    if (input.data[0] == '%')
    {
        return capy_string_shl(input, 3);
    }
    else
    {
        return capy_string_shl(input, 1);
    }
}

int capy_uri_pctenc_validate_(capy_string input, int categories, const char *chars)
{
    capy_assert(input.size);

    if (input.data[0] != '%')
    {
        return ((capy_uri_char_categories_(input.data[0]) & categories) ||
                capy_char_is(input.data[0], chars));
    }

    if (input.size >= 3 &&
        (capy_uri_char_categories_(input.data[1]) & URI_HEXDIGIT) &&
        (capy_uri_char_categories_(input.data[2]) & URI_HEXDIGIT))
    {
        return URI_PCT_ENCODED & categories;
    }

    return 0;
}

size_t capy_uri_string_validate_(capy_string s, int categories, const char *chars)
{
    capy_string input = s;

    while (input.size)
    {
        if (!capy_uri_pctenc_validate_(input, categories, chars))
        {
            return 0;
        }

        input = capy_uri_pctenc_shl_(input);
    }

    return s.size;
}

capy_string capy_uri_parse_hexword_(capy_string input)
{
    size_t i;

    for (i = 0; i < 4 && i < input.size; i++)
    {
        if ((capy_uri_char_categories_(input.data[i]) & URI_HEXDIGIT) == 0)
        {
            break;
        }
    }

    input.size = i;

    return input;
}

capy_string capy_uri_parse_decoctet_(capy_string input)
{
    switch (input.data[0])
    {
        case '0':
        {
            input.size = 1;
            return input;
        }
        break;

        case '1':
        case '2':
        {
            if (input.size >= 2 && capy_uri_char_categories_(input.data[1]) & URI_DIGIT)
            {
                if (input.size >= 3 && capy_uri_char_categories_(input.data[2]) & URI_DIGIT)
                {
                    input.size = 3;
                }
                else
                {
                    input.size = 2;
                }
            }
            else
            {
                input.size = 1;
            }

            return input;
        }
        break;

        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            if (input.size >= 2 && capy_uri_char_categories_(input.data[1]) & URI_DIGIT)
            {
                input.size = 2;
            }
            else
            {
                input.size = 1;
            }

            return input;
        }
        break;

        default:
            break;
    }

    input.size = 0;

    return input;
}

size_t capy_uri_valid_userinfo_(capy_string userinfo)
{
    return capy_uri_string_validate_(userinfo, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, ":");
}

size_t capy_uri_valid_port_(capy_string port)
{
    return capy_uri_string_validate_(port, URI_DIGIT, NULL);
}

size_t capy_uri_valid_fragment_(capy_string fragment)
{
    return capy_uri_string_validate_(fragment, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, ":@/?");
}

size_t capy_uri_valid_query_(capy_string query)
{
    return capy_uri_string_validate_(query, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, ":@/?");
}

size_t capy_uri_valid_reg_name_(capy_string host)
{
    return capy_uri_string_validate_(host, URI_UNRESERVED | URI_SUB_DELIM | URI_PCT_ENCODED, NULL);
}

size_t capy_uri_valid_segment_(capy_string segment)
{
    return capy_uri_string_validate_(segment, URI_UNRESERVED | URI_PCT_ENCODED | URI_SUB_DELIM, ":@");
}

size_t capy_uri_valid_ipv4_(capy_string host)
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

        capy_string dec_octet = capy_uri_parse_decoctet_(input);

        if (dec_octet.size == 0)
        {
            return 0;
        }

        input = capy_string_shl(input, dec_octet.size);
    }

    return (input.size == 0) ? host.size : 0;
}

size_t capy_uri_valid_ipv6_(capy_string host)
{
    capy_string input = host;

    size_t prefix = 0;
    size_t suffix = 0;
    size_t short_form = 0;

    while (prefix + suffix + short_form < 8 && input.size)
    {
        if ((!short_form && prefix == 6) || (short_form && suffix < 6))
        {
            if (capy_uri_valid_ipv4_(input))
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

        capy_string hex_word = capy_uri_parse_hexword_(input);
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

size_t capy_uri_valid_host_(capy_string host)
{
    if (host.data[0] == '[' && host.data[host.size - 1] == ']')
    {
        capy_string input = {.data = host.data + 1, .size = host.size - 2};

        if (capy_uri_valid_ipv6_(input))
        {
            return host.size;
        }

        return 0;
    }

    if (capy_uri_valid_ipv4_(host))
    {
        return host.size;
    }

    return capy_uri_valid_reg_name_(host);
}

size_t capy_uri_valid_scheme_(capy_string scheme)
{
    if (scheme.size == 0)
    {
        return 0;
    }

    capy_string input = scheme;

    if (!capy_uri_pctenc_validate_(input, URI_SCHEME_BEGIN, NULL))
    {
        return 0;
    }

    input = capy_uri_pctenc_shl_(input);

    while (input.size)
    {
        if (!capy_uri_pctenc_validate_(input, URI_SCHEME_CONTINUE, NULL))
        {
            return 0;
        }

        input = capy_uri_pctenc_shl_(input);
    }

    return scheme.size;
}

size_t capy_uri_valid_path_(capy_string path)
{
    capy_string input = path;

    size_t segment_size = 0;

    while (segment_size < input.size)
    {
        if (input.data[segment_size] == '/')
        {
            if (segment_size && !capy_uri_valid_segment_(capy_string_slice(input, 0, segment_size)))
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

    if (segment_size && !capy_uri_valid_segment_(input))
    {
        return 0;
    }

    return path.size;
}

capy_string capy_uri_path_merge_(capy_arena *arena, capy_string base, capy_string reference)
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
    char *buffer = Make(arena, char, size + 1);

    strncpy(buffer, base.data, base.size);
    buffer[base.size] = '/';
    strncpy(buffer + base.size + 1, reference.data, reference.size);

    return (capy_string){.data = buffer, .size = size};
}

// PUBLIC DEFINITIONS

capy_err capy_uri_normalize(capy_arena *arena, capy_string *output, capy_string input, int lowercase)
{
    if (input.size == 0)
    {
        *output = (capy_string){.size = 0};
        return Ok;
    }

    char *buffer = Make(arena, char, input.size + 1);

    if (buffer == NULL)
    {
        return ErrStd(ENOMEM);
    }

    size_t size = 0;

    while (input.size)
    {
        if (input.data[0] == '%')
        {
            uint64_t value;
            capy_string_parse_hexdigits(&value, capy_string_slice(input, 1, 3));
            char c = (char)(value);

            if (capy_uri_char_categories_(c) & URI_UNRESERVED)
            {
                buffer[size] = c;
                size += 1;
            }
            else
            {
                buffer[size] = '%';
                buffer[size + 1] = capy_char_lowercase(input.data[1]);
                buffer[size + 2] = capy_char_lowercase(input.data[2]);
                size += 3;
            }

            input = capy_string_shl(input, 3);
        }
        else
        {
            buffer[size] = (lowercase) ? capy_char_lowercase(input.data[0]) : input.data[0];
            size += 1;
            input = capy_string_shl(input, 1);
        }
    }

    *output = (capy_string){.data = buffer, .size = size};
    return Ok;
}

capy_string capy_uri_path_removedots(capy_arena *arena, capy_string path)
{
    capy_string path_self = Str("./");
    capy_string path_parent = Str("../");
    capy_string path_dot = Str(".");
    capy_string path_dots = Str("..");

    if (path.size == 0)
    {
        return (capy_string){.size = 0};
    }

    capy_string input = path;

    char *output = Make(arena, char, path.size + 1);
    size_t output_size = 0;

    while (input.size)
    {
        size_t size = input.size;

        for (size_t i = 0; i < input.size; i++)
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
            memmove(output + output_size, segment.data, segment.size);
            output_size += segment.size;
        }
    }

    output[output_size] = 0;

    return capy_string_bytes(output_size, output);
}

capy_uri capy_uri_parse_authority(capy_uri uri)
{
    uri.host = uri.authority;

    for (size_t i = 0; i < uri.host.size; i++)
    {
        if (uri.host.data[i] == '@')
        {
            uri.userinfo = capy_string_slice(uri.authority, 0, i);
            uri.host = capy_string_shl(uri.host, i + 1);
            break;
        }
    }

    for (size_t i = uri.host.size; i > 0; i--)
    {
        char c = uri.host.data[i - 1];

        if (c == ']')
        {
            break;
        }
        else if (c == ':')
        {
            uri.port = capy_string_shl(uri.host, i);
            uri.host = capy_string_slice(uri.host, 0, i - 1);
            break;
        }
    }

    return uri;
}

capy_uri capy_uri_parse(capy_string input)
{
    capy_uri uri = {{NULL}};

    for (size_t i = 0; i < input.size; i++)
    {
        char c = input.data[i];

        if (c == '/' || c == '?' || c == '#')
        {
            break;
        }
        else if (c == ':')
        {
            uri.scheme = capy_string_slice(input, 0, i);
            uri.flags |= CAPY_URI_SCHEME;
            input = capy_string_shl(input, i + 1);

            break;
        }
    }

    if (input.size > 1 && input.data[0] == '/' && input.data[1] == '/')
    {
        input = capy_string_shl(input, 2);

        size_t i;

        for (i = 0; i < input.size; i++)
        {
            char c = input.data[i];

            if (c == '/' || c == '?' || c == '#')
            {
                break;
            }
        }

        uri.authority = capy_string_slice(input, 0, i);
        uri.flags |= CAPY_URI_AUTHORITY;
        input = capy_string_shl(input, i);
    }

    if (input.size)
    {
        size_t i;

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

        size_t i;

        for (i = 0; i < input.size && input.data[i] != '#'; i++)
        {
        }

        uri.query = capy_string_slice(input, 0, i);
        uri.flags |= CAPY_URI_QUERY;
        input = capy_string_shl(input, i);
    }

    if (input.size)
    {
        uri.fragment = capy_string_shl(input, 1);
        uri.flags |= CAPY_URI_FRAGMENT;
        input = (capy_string){NULL};
    }

    return capy_uri_parse_authority(uri);
}

int capy_uri_valid(capy_uri uri)
{
    if ((uri.flags & CAPY_URI_SCHEME) && !capy_uri_valid_scheme_(uri.scheme))
    {
        return false;
    }

    if (uri.userinfo.size && !capy_uri_valid_userinfo_(uri.userinfo))
    {
        return false;
    }

    if (uri.port.size && !capy_uri_valid_port_(uri.port))
    {
        return false;
    }

    if (uri.host.size && !capy_uri_valid_host_(uri.host))
    {
        return false;
    }

    if (uri.path.size && !capy_uri_valid_path_(uri.path))
    {
        return false;
    }

    if (uri.query.size && !capy_uri_valid_query_(uri.query))
    {
        return false;
    }

    if (uri.fragment.size && !capy_uri_valid_fragment_(uri.fragment))
    {
        return false;
    }

    return true;
}

// @todo: return error
capy_uri capy_uri_resolve_reference(capy_arena *arena, capy_uri base, capy_uri reference)
{
    capy_uri uri = reference;

    if (reference.scheme.size)
    {
        uri.path = capy_uri_path_removedots(arena, reference.path);
        return uri;
    }

    if (reference.authority.size)
    {
        uri.scheme = base.scheme;
        uri.path = capy_uri_path_removedots(arena, reference.path);
        uri.flags &= ~CAPY_URI_SCHEME;
        uri.flags |= base.flags & CAPY_URI_SCHEME;
        return uri;
    }

    uri.scheme = base.scheme;
    uri.authority = base.authority;
    uri.userinfo = base.userinfo;
    uri.host = base.host;
    uri.port = base.port;
    uri.flags &= ~(CAPY_URI_SCHEME | CAPY_URI_AUTHORITY);
    uri.flags |= base.flags & (CAPY_URI_SCHEME | CAPY_URI_AUTHORITY);

    if (reference.path.size)
    {
        if (reference.path.data[0] == '/')
        {
            uri.path = capy_uri_path_removedots(arena, reference.path);
        }
        else
        {
            uri.path = capy_uri_path_merge_(arena, base.path, reference.path);
            uri.path = capy_uri_path_removedots(arena, uri.path);
        }
    }
    else
    {
        uri.path = base.path;

        if (reference.query.size == 0)
        {
            uri.query = base.query;
            uri.flags &= ~CAPY_URI_QUERY;
            uri.flags |= base.flags & CAPY_URI_QUERY;
        }
    }

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

    char *buffer = Make(arena, char, buffer_max);
    size_t buffer_size = 0;

    if (uri.flags & CAPY_URI_SCHEME)
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

    return capy_string_bytes(buffer_size, buffer);
}
