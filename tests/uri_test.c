#include <capy/test.h>

static int test_uri_parse(void)
{
    struct table_uri
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
    };

    struct table_uri uris[] = {
        {
            .raw = Str("http://foo@localhost:8080/post?id=1#title"),
            .scheme = Str("http"),
            .authority = Str("foo@localhost:8080"),
            .userinfo = Str("foo"),
            .host = Str("localhost"),
            .port = Str("8080"),
            .path = Str("/post"),
            .query = Str("id=1"),
            .fragment = Str("title"),
            .is_valid = true,
        },
        {
            .raw = Str("data:text/plain;charset=utf-8,foo%20bar"),
            .scheme = Str("data"),
            .path = Str("text/plain;charset=utf-8,foo%20bar"),
            .is_valid = true,
        },
        {
            .raw = Str("foo bar://localhost"),
            .scheme = Str("foo bar"),
            .authority = Str("localhost"),
            .host = Str("localhost"),
            .is_valid = false,
        },
        {
            .raw = Str(" http://localhost"),
            .scheme = Str(" http"),
            .authority = Str("localhost"),
            .host = Str("localhost"),
            .is_valid = false,
        },
        {
            .raw = Str(":"),
            .scheme = Str(""),
            .is_valid = false,
        },
        {
            .raw = Str("h:"),
            .scheme = Str("h"),
            .is_valid = true,
        },
        {
            .raw = Str("//127.0.0.1"),
            .authority = Str("127.0.0.1"),
            .host = Str("127.0.0.1"),
            .is_valid = true,
        },
        {
            .raw = Str("//10.20.30.5"),
            .authority = Str("10.20.30.5"),
            .host = Str("10.20.30.5"),
            .is_valid = true,
        },
        {
            .raw = Str("//1O.0.0.1"),
            .authority = Str("1O.0.0.1"),
            .host = Str("1O.0.0.1"),
            .is_valid = true,
        },
        {
            .raw = Str("//10"),
            .authority = Str("10"),
            .host = Str("10"),
            .is_valid = true,
        },
        {
            .raw = Str("//30.40.50.60"),
            .authority = Str("30.40.50.60"),
            .host = Str("30.40.50.60"),
            .is_valid = true,
        },
        {
            .raw = Str("//70.80.90.9O"),
            .authority = Str("70.80.90.9O"),
            .host = Str("70.80.90.9O"),
            .is_valid = true,
        },
        {
            .raw = Str("//foo%2"),
            .authority = Str("foo%2"),
            .host = Str("foo%2"),
            .is_valid = false,
        },
        {
            .raw = Str("//foo%20bar"),
            .authority = Str("foo%20bar"),
            .host = Str("foo%20bar"),
            .is_valid = true,
        },
        {
            .raw = Str("//foo bar"),
            .authority = Str("foo bar"),
            .host = Str("foo bar"),
            .is_valid = false,
        },
        {
            .raw = Str("//foo%0gbar"),
            .authority = Str("foo%0gbar"),
            .host = Str("foo%0gbar"),
            .is_valid = false,
        },
        {
            .raw = Str("//foo%g0bar"),
            .authority = Str("foo%g0bar"),
            .host = Str("foo%g0bar"),
            .is_valid = false,
        },
        {
            .raw = Str("/foo@bar"),
            .path = Str("/foo@bar"),
            .is_valid = true,
        },
        {
            .raw = Str("/foo/^bar"),
            .path = Str("/foo/^bar"),
            .is_valid = false,
        },
        {
            .raw = Str("/foo^bar/"),
            .path = Str("/foo^bar/"),
            .is_valid = false,
        },
        {
            .raw = Str("foo/"),
            .path = Str("foo/"),
            .is_valid = true,
        },
        {
            .raw = Str("/foo"),
            .path = Str("/foo"),
            .is_valid = true,
        },
        {
            .raw = Str("//foo@[::1]:8080"),
            .authority = Str("foo@[::1]:8080"),
            .userinfo = Str("foo"),
            .host = Str("[::1]"),
            .port = Str("8080"),
            .is_valid = true,
        },
        {
            .raw = Str("//[::12345]"),
            .authority = Str("[::12345]"),
            .host = Str("[::12345]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[::127.0.0.1]"),
            .authority = Str("[::127.0.0.1]"),
            .host = Str("[::127.0.0.1]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[::1:2:3:4:5:6:7]"),
            .authority = Str("[::1:2:3:4:5:6:7]"),
            .host = Str("[::1:2:3:4:5:6:7]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[::1:2:3:4:5:6:7:8]"),
            .authority = Str("[::1:2:3:4:5:6:7:8]"),
            .host = Str("[::1:2:3:4:5:6:7:8]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[1::2:3:4:5:6:7]"),
            .authority = Str("[1::2:3:4:5:6:7]"),
            .host = Str("[1::2:3:4:5:6:7]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[1::2:3:4:5:6:7:8]"),
            .authority = Str("[1::2:3:4:5:6:7:8]"),
            .host = Str("[1::2:3:4:5:6:7:8]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[]"),
            .authority = Str("[]"),
            .host = Str("[]"),
            .is_valid = false,
        },
        {
            .raw = Str("//["),
            .authority = Str("["),
            .host = Str("["),
            .is_valid = false,
        },
        {
            .raw = Str("//[1:]"),
            .authority = Str("[1:]"),
            .host = Str("[1:]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[1:2:3:4:5:6:7:8]"),
            .authority = Str("[1:2:3:4:5:6:7:8]"),
            .host = Str("[1:2:3:4:5:6:7:8]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[0:0:0:0:0:0:127.0.0.1]"),
            .authority = Str("[0:0:0:0:0:0:127.0.0.1]"),
            .host = Str("[0:0:0:0:0:0:127.0.0.1]"),
            .is_valid = true,
        },
        {
            .raw = Str("//[0:0:0:0:0:0:0:127.0.0.1]"),
            .authority = Str("[0:0:0:0:0:0:0:127.0.0.1]"),
            .host = Str("[0:0:0:0:0:0:0:127.0.0.1]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[::0:0:0:0:0:0:127.0.0.1]"),
            .authority = Str("[::0:0:0:0:0:0:127.0.0.1]"),
            .host = Str("[::0:0:0:0:0:0:127.0.0.1]"),
            .is_valid = false,
        },
        {
            .raw = Str("//[::0::1]"),
            .authority = Str("[::0::1]"),
            .host = Str("[::0::1]"),
            .is_valid = false,
        },
        {
            .raw = Str("//foo bar@localhost"),
            .authority = Str("foo bar@localhost"),
            .userinfo = Str("foo bar"),
            .host = Str("localhost"),
            .is_valid = false,
        },
        {
            .raw = Str("//localhost: 8080"),
            .authority = Str("localhost: 8080"),
            .host = Str("localhost"),
            .port = Str(" 8080"),
            .is_valid = false,
        },
        {
            .raw = Str("index.html?id=foo bar"),
            .path = Str("index.html"),
            .query = Str("id=foo bar"),
            .is_valid = false,
        },
        {
            .raw = Str("index.html#id=foo bar"),
            .path = Str("index.html"),
            .fragment = Str("id=foo bar"),
            .is_valid = false,
        },
        {
            .raw = Str("//localhost?id=1"),
            .authority = Str("localhost"),
            .host = Str("localhost"),
            .query = Str("id=1"),
            .is_valid = true,
        },
        {
            .raw = Str("//localhost#title"),
            .authority = Str("localhost"),
            .host = Str("localhost"),
            .fragment = Str("title"),
            .is_valid = true,
        },
    };

    for (size_t i = 0; i < ArrLen(uris); i++)
    {
        capy_uri uri = capy_uri_parse(uris[i].raw);

        ExpectEqS(capy_uri_valid(uri), uris[i].is_valid);
        ExpectEqStr(uri.scheme, uris[i].scheme);
        ExpectEqStr(uri.authority, uris[i].authority);
        ExpectEqStr(uri.userinfo, uris[i].userinfo);
        ExpectEqStr(uri.host, uris[i].host);
        ExpectEqStr(uri.port, uris[i].port);
        ExpectEqStr(uri.path, uris[i].path);
        ExpectEqStr(uri.query, uris[i].query);
        ExpectEqStr(uri.fragment, uris[i].fragment);
    }

    return true;
}

static int test_uri_resolve_reference(void)
{
    struct table_resolve
    {
        capy_string base;
        capy_string reference;
        capy_string expected;
    };

    struct table_resolve uris_reference[] = {
        {Str("//localhost"), Str(""), Str("//localhost")},
        {Str("//localhost"), Str("index.html"), Str("//localhost/index.html")},
        {Str("//localhost"), Str("https:a/.."), Str("https:")},

        // RFC 3986: https://datatracker.ietf.org/doc/html/rfc3986#section-5.4.1
        {Str("http://a/b/c/d;p?q"), Str("g:h"), Str("g:h")},
        {Str("http://a/b/c/d;p?q"), Str("g"), Str("http://a/b/c/g")},
        {Str("http://a/b/c/d;p?q"), Str("./g"), Str("http://a/b/c/g")},
        {Str("http://a/b/c/d;p?q"), Str("g/"), Str("http://a/b/c/g/")},
        {Str("http://a/b/c/d;p?q"), Str("/g"), Str("http://a/g")},
        {Str("http://a/b/c/d;p?q"), Str("//g"), Str("http://g")},
        {Str("http://a/b/c/d;p?q"), Str("?y"), Str("http://a/b/c/d;p?y")},
        {Str("http://a/b/c/d;p?q"), Str("g?y"), Str("http://a/b/c/g?y")},
        {Str("http://a/b/c/d;p?q"), Str("#s"), Str("http://a/b/c/d;p?q#s")},
        {Str("http://a/b/c/d;p?q"), Str("g#s"), Str("http://a/b/c/g#s")},
        {Str("http://a/b/c/d;p?q"), Str("g?y#s"), Str("http://a/b/c/g?y#s")},
        {Str("http://a/b/c/d;p?q"), Str(";x"), Str("http://a/b/c/;x")},
        {Str("http://a/b/c/d;p?q"), Str("g;x"), Str("http://a/b/c/g;x")},
        {Str("http://a/b/c/d;p?q"), Str("g;x?y#s"), Str("http://a/b/c/g;x?y#s")},
        {Str("http://a/b/c/d;p?q"), Str(""), Str("http://a/b/c/d;p?q")},
        {Str("http://a/b/c/d;p?q"), Str("."), Str("http://a/b/c/")},
        {Str("http://a/b/c/d;p?q"), Str("./"), Str("http://a/b/c/")},
        {Str("http://a/b/c/d;p?q"), Str(".."), Str("http://a/b/")},
        {Str("http://a/b/c/d;p?q"), Str("../"), Str("http://a/b/")},
        {Str("http://a/b/c/d;p?q"), Str("../g"), Str("http://a/b/g")},
        {Str("http://a/b/c/d;p?q"), Str("../.."), Str("http://a/")},
        {Str("http://a/b/c/d;p?q"), Str("../../"), Str("http://a/")},
        {Str("http://a/b/c/d;p?q"), Str("../../g"), Str("http://a/g")},
        {Str("http://a/b/c/d;p?q"), Str("/./g"), Str("http://a/g")},
        {Str("http://a/b/c/d;p?q"), Str("/../g"), Str("http://a/g")},
        {Str("http://a/b/c/d;p?q"), Str("g."), Str("http://a/b/c/g.")},
        {Str("http://a/b/c/d;p?q"), Str(".g"), Str("http://a/b/c/.g")},
        {Str("http://a/b/c/d;p?q"), Str("g.."), Str("http://a/b/c/g..")},
        {Str("http://a/b/c/d;p?q"), Str("..g"), Str("http://a/b/c/..g")},
        {Str("http://a/b/c/d;p?q"), Str("./../g"), Str("http://a/b/g")},
        {Str("http://a/b/c/d;p?q"), Str("./g/."), Str("http://a/b/c/g/")},
        {Str("http://a/b/c/d;p?q"), Str("g/./h"), Str("http://a/b/c/g/h")},
        {Str("http://a/b/c/d;p?q"), Str("g/../h"), Str("http://a/b/c/h")},
        {Str("http://a/b/c/d;p?q"), Str("g;x=1/./y"), Str("http://a/b/c/g;x=1/y")},
        {Str("http://a/b/c/d;p?q"), Str("g;x=1/../y"), Str("http://a/b/c/y")},
        {Str("http://a/b/c/d;p?q"), Str("g?y/./x"), Str("http://a/b/c/g?y/./x")},
        {Str("http://a/b/c/d;p?q"), Str("g?y/../x"), Str("http://a/b/c/g?y/../x")},
        {Str("http://a/b/c/d;p?q"), Str("g#s/./x"), Str("http://a/b/c/g#s/./x")},
        {Str("http://a/b/c/d;p?q"), Str("g#s/../x"), Str("http://a/b/c/g#s/../x")},
        {Str("http://a/b/c/d;p?q"), Str("http:g"), Str("http:g")},
    };

    capy_arena *arena = capy_arena_init(0, KiB(4));

    for (size_t i = 0; i < ArrLen(uris_reference); i++)
    {
        capy_uri base = capy_uri_parse(uris_reference[i].base);
        capy_uri reference = capy_uri_parse(uris_reference[i].reference);
        capy_uri uri = capy_uri_resolve_reference(arena, base, reference);
        capy_string result = capy_uri_string(arena, uri);

        ExpectEqStr(result, uris_reference[i].expected);
    }

    capy_arena_destroy(arena);

    return true;
}

static void test_uri(testbench *t)
{
    runtest(t, test_uri_parse, "capy_uri_parse");
    runtest(t, test_uri_resolve_reference, "capy_uri_resolve_reference");
}
