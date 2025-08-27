#include "test.h"

static struct
{
    capy_string raw;
    capy_string scheme;
    capy_string authority;
    capy_string userinfo;
    capy_string host;
    capy_string port;
    capy_string path;
    capy_string query;
    capy_string fragment;
    int64_t is_valid;
} uris[] = {
    {
        .raw = str("http://foo@localhost:8080/post?id=1#title"),
        .scheme = str("http"),
        .authority = str("foo@localhost:8080"),
        .userinfo = str("foo"),
        .host = str("localhost"),
        .port = str("8080"),
        .path = str("/post"),
        .query = str("id=1"),
        .fragment = str("title"),
        .is_valid = true,
    },
    {
        .raw = str("data:text/plain;charset=utf-8,foo%20bar"),
        .scheme = str("data"),
        .path = str("text/plain;charset=utf-8,foo%20bar"),
        .is_valid = true,
    },
    {
        .raw = str("foo bar://localhost"),
        .scheme = str("foo bar"),
        .authority = str("localhost"),
        .host = str("localhost"),
        .is_valid = false,
    },
    {
        .raw = str(" http://localhost"),
        .scheme = str(" http"),
        .authority = str("localhost"),
        .host = str("localhost"),
        .is_valid = false,
    },
    {
        .raw = str(":"),
        .scheme = str(""),
        .is_valid = true,
    },
    {
        .raw = str("h:"),
        .scheme = str("h"),
        .is_valid = true,
    },
    {
        .raw = str("//127.0.0.1"),
        .authority = str("127.0.0.1"),
        .host = str("127.0.0.1"),
        .is_valid = true,
    },
    {
        .raw = str("//10.20.30.5"),
        .authority = str("10.20.30.5"),
        .host = str("10.20.30.5"),
        .is_valid = true,
    },
    {
        .raw = str("//1O.0.0.1"),
        .authority = str("1O.0.0.1"),
        .host = str("1O.0.0.1"),
        .is_valid = true,
    },
    {
        .raw = str("//10"),
        .authority = str("10"),
        .host = str("10"),
        .is_valid = true,
    },
    {
        .raw = str("//30.40.50.60"),
        .authority = str("30.40.50.60"),
        .host = str("30.40.50.60"),
        .is_valid = true,
    },
    {
        .raw = str("//70.80.90.9O"),
        .authority = str("70.80.90.9O"),
        .host = str("70.80.90.9O"),
        .is_valid = true,
    },
    {
        .raw = str("//foo%2"),
        .authority = str("foo%2"),
        .host = str("foo%2"),
        .is_valid = false,
    },
    {
        .raw = str("//foo%20bar"),
        .authority = str("foo%20bar"),
        .host = str("foo%20bar"),
        .is_valid = true,
    },
    {
        .raw = str("//foo bar"),
        .authority = str("foo bar"),
        .host = str("foo bar"),
        .is_valid = false,
    },
    {
        .raw = str("//foo%0gbar"),
        .authority = str("foo%0gbar"),
        .host = str("foo%0gbar"),
        .is_valid = false,
    },
    {
        .raw = str("//foo%g0bar"),
        .authority = str("foo%g0bar"),
        .host = str("foo%g0bar"),
        .is_valid = false,
    },
    {
        .raw = str("/foo@bar"),
        .path = str("/foo@bar"),
        .is_valid = true,
    },
    {
        .raw = str("/foo/^bar"),
        .path = str("/foo/^bar"),
        .is_valid = false,
    },
    {
        .raw = str("/foo^bar/"),
        .path = str("/foo^bar/"),
        .is_valid = false,
    },
    {
        .raw = str("foo/"),
        .path = str("foo/"),
        .is_valid = true,
    },
    {
        .raw = str("/foo"),
        .path = str("/foo"),
        .is_valid = true,
    },
    {
        .raw = str("//foo@[::1]:8080"),
        .authority = str("foo@[::1]:8080"),
        .userinfo = str("foo"),
        .host = str("[::1]"),
        .port = str("8080"),
        .is_valid = true,
    },
    {
        .raw = str("//[::12345]"),
        .authority = str("[::12345]"),
        .host = str("[::12345]"),
        .is_valid = false,
    },
    {
        .raw = str("//[::127.0.0.1]"),
        .authority = str("[::127.0.0.1]"),
        .host = str("[::127.0.0.1]"),
        .is_valid = true,
    },
    {
        .raw = str("//[::1:2:3:4:5:6:7]"),
        .authority = str("[::1:2:3:4:5:6:7]"),
        .host = str("[::1:2:3:4:5:6:7]"),
        .is_valid = true,
    },
    {
        .raw = str("//[::1:2:3:4:5:6:7:8]"),
        .authority = str("[::1:2:3:4:5:6:7:8]"),
        .host = str("[::1:2:3:4:5:6:7:8]"),
        .is_valid = false,
    },
    {
        .raw = str("//[1::2:3:4:5:6:7]"),
        .authority = str("[1::2:3:4:5:6:7]"),
        .host = str("[1::2:3:4:5:6:7]"),
        .is_valid = true,
    },
    {
        .raw = str("//[1::2:3:4:5:6:7:8]"),
        .authority = str("[1::2:3:4:5:6:7:8]"),
        .host = str("[1::2:3:4:5:6:7:8]"),
        .is_valid = false,
    },
    {
        .raw = str("//[]"),
        .authority = str("[]"),
        .host = str("[]"),
        .is_valid = false,
    },
    {
        .raw = str("//["),
        .authority = str("["),
        .host = str("["),
        .is_valid = false,
    },
    {
        .raw = str("//[1:]"),
        .authority = str("[1:]"),
        .host = str("[1:]"),
        .is_valid = false,
    },
    {
        .raw = str("//[1:2:3:4:5:6:7:8]"),
        .authority = str("[1:2:3:4:5:6:7:8]"),
        .host = str("[1:2:3:4:5:6:7:8]"),
        .is_valid = true,
    },
    {
        .raw = str("//[0:0:0:0:0:0:127.0.0.1]"),
        .authority = str("[0:0:0:0:0:0:127.0.0.1]"),
        .host = str("[0:0:0:0:0:0:127.0.0.1]"),
        .is_valid = true,
    },
    {
        .raw = str("//[0:0:0:0:0:0:0:127.0.0.1]"),
        .authority = str("[0:0:0:0:0:0:0:127.0.0.1]"),
        .host = str("[0:0:0:0:0:0:0:127.0.0.1]"),
        .is_valid = false,
    },
    {
        .raw = str("//[::0:0:0:0:0:0:127.0.0.1]"),
        .authority = str("[::0:0:0:0:0:0:127.0.0.1]"),
        .host = str("[::0:0:0:0:0:0:127.0.0.1]"),
        .is_valid = false,
    },
    {
        .raw = str("//[::0::1]"),
        .authority = str("[::0::1]"),
        .host = str("[::0::1]"),
        .is_valid = false,
    },
    {
        .raw = str("//foo bar@localhost"),
        .authority = str("foo bar@localhost"),
        .userinfo = str("foo bar"),
        .host = str("localhost"),
        .is_valid = false,
    },
    {
        .raw = str("//localhost: 8080"),
        .authority = str("localhost: 8080"),
        .host = str("localhost"),
        .port = str(" 8080"),
        .is_valid = false,
    },
    {
        .raw = str("index.html?id=foo bar"),
        .path = str("index.html"),
        .query = str("id=foo bar"),
        .is_valid = false,
    },
    {
        .raw = str("index.html#id=foo bar"),
        .path = str("index.html"),
        .fragment = str("id=foo bar"),
        .is_valid = false,
    },
    {
        .raw = str("//localhost?id=1"),
        .authority = str("localhost"),
        .host = str("localhost"),
        .query = str("id=1"),
        .is_valid = true,
    },
    {
        .raw = str("//localhost#title"),
        .authority = str("localhost"),
        .host = str("localhost"),
        .fragment = str("title"),
        .is_valid = true,
    },
};

static struct
{
    capy_string base;
    capy_string reference;
    capy_string expected;
} uris_reference[] = {
    {str("//localhost"), str(""), str("//localhost")},
    {str("//localhost"), str("index.html"), str("//localhost/index.html")},
    {str("//localhost"), str("https:a/.."), str("https:")},

    // RFC 3986: https://datatracker.ietf.org/doc/html/rfc3986#section-5.4.1
    {str("http://a/b/c/d;p?q"), str("g:h"), str("g:h")},
    {str("http://a/b/c/d;p?q"), str("g"), str("http://a/b/c/g")},
    {str("http://a/b/c/d;p?q"), str("./g"), str("http://a/b/c/g")},
    {str("http://a/b/c/d;p?q"), str("g/"), str("http://a/b/c/g/")},
    {str("http://a/b/c/d;p?q"), str("/g"), str("http://a/g")},
    {str("http://a/b/c/d;p?q"), str("//g"), str("http://g")},
    {str("http://a/b/c/d;p?q"), str("?y"), str("http://a/b/c/d;p?y")},
    {str("http://a/b/c/d;p?q"), str("g?y"), str("http://a/b/c/g?y")},
    {str("http://a/b/c/d;p?q"), str("#s"), str("http://a/b/c/d;p?q#s")},
    {str("http://a/b/c/d;p?q"), str("g#s"), str("http://a/b/c/g#s")},
    {str("http://a/b/c/d;p?q"), str("g?y#s"), str("http://a/b/c/g?y#s")},
    {str("http://a/b/c/d;p?q"), str(";x"), str("http://a/b/c/;x")},
    {str("http://a/b/c/d;p?q"), str("g;x"), str("http://a/b/c/g;x")},
    {str("http://a/b/c/d;p?q"), str("g;x?y#s"), str("http://a/b/c/g;x?y#s")},
    {str("http://a/b/c/d;p?q"), str(""), str("http://a/b/c/d;p?q")},
    {str("http://a/b/c/d;p?q"), str("."), str("http://a/b/c/")},
    {str("http://a/b/c/d;p?q"), str("./"), str("http://a/b/c/")},
    {str("http://a/b/c/d;p?q"), str(".."), str("http://a/b/")},
    {str("http://a/b/c/d;p?q"), str("../"), str("http://a/b/")},
    {str("http://a/b/c/d;p?q"), str("../g"), str("http://a/b/g")},
    {str("http://a/b/c/d;p?q"), str("../.."), str("http://a/")},
    {str("http://a/b/c/d;p?q"), str("../../"), str("http://a/")},
    {str("http://a/b/c/d;p?q"), str("../../g"), str("http://a/g")},
    {str("http://a/b/c/d;p?q"), str("/./g"), str("http://a/g")},
    {str("http://a/b/c/d;p?q"), str("/../g"), str("http://a/g")},
    {str("http://a/b/c/d;p?q"), str("g."), str("http://a/b/c/g.")},
    {str("http://a/b/c/d;p?q"), str(".g"), str("http://a/b/c/.g")},
    {str("http://a/b/c/d;p?q"), str("g.."), str("http://a/b/c/g..")},
    {str("http://a/b/c/d;p?q"), str("..g"), str("http://a/b/c/..g")},
    {str("http://a/b/c/d;p?q"), str("./../g"), str("http://a/b/g")},
    {str("http://a/b/c/d;p?q"), str("./g/."), str("http://a/b/c/g/")},
    {str("http://a/b/c/d;p?q"), str("g/./h"), str("http://a/b/c/g/h")},
    {str("http://a/b/c/d;p?q"), str("g/../h"), str("http://a/b/c/h")},
    {str("http://a/b/c/d;p?q"), str("g;x=1/./y"), str("http://a/b/c/g;x=1/y")},
    {str("http://a/b/c/d;p?q"), str("g;x=1/../y"), str("http://a/b/c/y")},
    {str("http://a/b/c/d;p?q"), str("g?y/./x"), str("http://a/b/c/g?y/./x")},
    {str("http://a/b/c/d;p?q"), str("g?y/../x"), str("http://a/b/c/g?y/../x")},
    {str("http://a/b/c/d;p?q"), str("g#s/./x"), str("http://a/b/c/g#s/./x")},
    {str("http://a/b/c/d;p?q"), str("g#s/../x"), str("http://a/b/c/g#s/../x")},
    {str("http://a/b/c/d;p?q"), str("http:g"), str("http:g")},
};

static int test_uri(void)
{
    capy_arena *arena = capy_arena_init(GiB(8ULL));

    for (size_t i = 0; i < arrlen(uris); i++)
    {
        capy_uri uri = capy_uri_parse(uris[i].raw);

        expect_s_eq(capy_uri_valid(uri), uris[i].is_valid);
        expect_str_eq(uri.scheme, uris[i].scheme);
        expect_str_eq(uri.authority, uris[i].authority);
        expect_str_eq(uri.userinfo, uris[i].userinfo);
        expect_str_eq(uri.host, uris[i].host);
        expect_str_eq(uri.port, uris[i].port);
        expect_str_eq(uri.path, uris[i].path);
        expect_str_eq(uri.query, uris[i].query);
        expect_str_eq(uri.fragment, uris[i].fragment);
    }

    for (size_t i = 0; i < arrlen(uris_reference); i++)
    {
        capy_uri base = capy_uri_parse(uris_reference[i].base);
        capy_uri reference = capy_uri_parse(uris_reference[i].reference);
        capy_uri uri = capy_uri_resolve_reference(arena, base, reference);
        capy_string result = capy_uri_string(arena, uri);

        expect_str_eq(result, uris_reference[i].expected);
    }

    return 0;
}
