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
            .raw = strl("http://foo@localhost:8080/post?id=1#title"),
            .scheme = strl("http"),
            .authority = strl("foo@localhost:8080"),
            .userinfo = strl("foo"),
            .host = strl("localhost"),
            .port = strl("8080"),
            .path = strl("/post"),
            .query = strl("id=1"),
            .fragment = strl("title"),
            .is_valid = true,
        },
        {
            .raw = strl("data:text/plain;charset=utf-8,foo%20bar"),
            .scheme = strl("data"),
            .path = strl("text/plain;charset=utf-8,foo%20bar"),
            .is_valid = true,
        },
        {
            .raw = strl("foo bar://localhost"),
            .scheme = strl("foo bar"),
            .authority = strl("localhost"),
            .host = strl("localhost"),
            .is_valid = false,
        },
        {
            .raw = strl(" http://localhost"),
            .scheme = strl(" http"),
            .authority = strl("localhost"),
            .host = strl("localhost"),
            .is_valid = false,
        },
        {
            .raw = strl(":"),
            .scheme = strl(""),
            .is_valid = false,
        },
        {
            .raw = strl("h:"),
            .scheme = strl("h"),
            .is_valid = true,
        },
        {
            .raw = strl("//127.0.0.1"),
            .authority = strl("127.0.0.1"),
            .host = strl("127.0.0.1"),
            .is_valid = true,
        },
        {
            .raw = strl("//10.20.30.5"),
            .authority = strl("10.20.30.5"),
            .host = strl("10.20.30.5"),
            .is_valid = true,
        },
        {
            .raw = strl("//1O.0.0.1"),
            .authority = strl("1O.0.0.1"),
            .host = strl("1O.0.0.1"),
            .is_valid = true,
        },
        {
            .raw = strl("//10"),
            .authority = strl("10"),
            .host = strl("10"),
            .is_valid = true,
        },
        {
            .raw = strl("//30.40.50.60"),
            .authority = strl("30.40.50.60"),
            .host = strl("30.40.50.60"),
            .is_valid = true,
        },
        {
            .raw = strl("//70.80.90.9O"),
            .authority = strl("70.80.90.9O"),
            .host = strl("70.80.90.9O"),
            .is_valid = true,
        },
        {
            .raw = strl("//foo%2"),
            .authority = strl("foo%2"),
            .host = strl("foo%2"),
            .is_valid = false,
        },
        {
            .raw = strl("//foo%20bar"),
            .authority = strl("foo%20bar"),
            .host = strl("foo%20bar"),
            .is_valid = true,
        },
        {
            .raw = strl("//foo bar"),
            .authority = strl("foo bar"),
            .host = strl("foo bar"),
            .is_valid = false,
        },
        {
            .raw = strl("//foo%0gbar"),
            .authority = strl("foo%0gbar"),
            .host = strl("foo%0gbar"),
            .is_valid = false,
        },
        {
            .raw = strl("//foo%g0bar"),
            .authority = strl("foo%g0bar"),
            .host = strl("foo%g0bar"),
            .is_valid = false,
        },
        {
            .raw = strl("/foo@bar"),
            .path = strl("/foo@bar"),
            .is_valid = true,
        },
        {
            .raw = strl("/foo/^bar"),
            .path = strl("/foo/^bar"),
            .is_valid = false,
        },
        {
            .raw = strl("/foo^bar/"),
            .path = strl("/foo^bar/"),
            .is_valid = false,
        },
        {
            .raw = strl("foo/"),
            .path = strl("foo/"),
            .is_valid = true,
        },
        {
            .raw = strl("/foo"),
            .path = strl("/foo"),
            .is_valid = true,
        },
        {
            .raw = strl("//foo@[::1]:8080"),
            .authority = strl("foo@[::1]:8080"),
            .userinfo = strl("foo"),
            .host = strl("[::1]"),
            .port = strl("8080"),
            .is_valid = true,
        },
        {
            .raw = strl("//[::12345]"),
            .authority = strl("[::12345]"),
            .host = strl("[::12345]"),
            .is_valid = false,
        },
        {
            .raw = strl("//[::127.0.0.1]"),
            .authority = strl("[::127.0.0.1]"),
            .host = strl("[::127.0.0.1]"),
            .is_valid = true,
        },
        {
            .raw = strl("//[::1:2:3:4:5:6:7]"),
            .authority = strl("[::1:2:3:4:5:6:7]"),
            .host = strl("[::1:2:3:4:5:6:7]"),
            .is_valid = true,
        },
        {
            .raw = strl("//[::1:2:3:4:5:6:7:8]"),
            .authority = strl("[::1:2:3:4:5:6:7:8]"),
            .host = strl("[::1:2:3:4:5:6:7:8]"),
            .is_valid = false,
        },
        {
            .raw = strl("//[1::2:3:4:5:6:7]"),
            .authority = strl("[1::2:3:4:5:6:7]"),
            .host = strl("[1::2:3:4:5:6:7]"),
            .is_valid = true,
        },
        {
            .raw = strl("//[1::2:3:4:5:6:7:8]"),
            .authority = strl("[1::2:3:4:5:6:7:8]"),
            .host = strl("[1::2:3:4:5:6:7:8]"),
            .is_valid = false,
        },
        {
            .raw = strl("//[]"),
            .authority = strl("[]"),
            .host = strl("[]"),
            .is_valid = false,
        },
        {
            .raw = strl("//["),
            .authority = strl("["),
            .host = strl("["),
            .is_valid = false,
        },
        {
            .raw = strl("//[1:]"),
            .authority = strl("[1:]"),
            .host = strl("[1:]"),
            .is_valid = false,
        },
        {
            .raw = strl("//[1:2:3:4:5:6:7:8]"),
            .authority = strl("[1:2:3:4:5:6:7:8]"),
            .host = strl("[1:2:3:4:5:6:7:8]"),
            .is_valid = true,
        },
        {
            .raw = strl("//[0:0:0:0:0:0:127.0.0.1]"),
            .authority = strl("[0:0:0:0:0:0:127.0.0.1]"),
            .host = strl("[0:0:0:0:0:0:127.0.0.1]"),
            .is_valid = true,
        },
        {
            .raw = strl("//[0:0:0:0:0:0:0:127.0.0.1]"),
            .authority = strl("[0:0:0:0:0:0:0:127.0.0.1]"),
            .host = strl("[0:0:0:0:0:0:0:127.0.0.1]"),
            .is_valid = false,
        },
        {
            .raw = strl("//[::0:0:0:0:0:0:127.0.0.1]"),
            .authority = strl("[::0:0:0:0:0:0:127.0.0.1]"),
            .host = strl("[::0:0:0:0:0:0:127.0.0.1]"),
            .is_valid = false,
        },
        {
            .raw = strl("//[::0::1]"),
            .authority = strl("[::0::1]"),
            .host = strl("[::0::1]"),
            .is_valid = false,
        },
        {
            .raw = strl("//foo bar@localhost"),
            .authority = strl("foo bar@localhost"),
            .userinfo = strl("foo bar"),
            .host = strl("localhost"),
            .is_valid = false,
        },
        {
            .raw = strl("//localhost: 8080"),
            .authority = strl("localhost: 8080"),
            .host = strl("localhost"),
            .port = strl(" 8080"),
            .is_valid = false,
        },
        {
            .raw = strl("index.html?id=foo bar"),
            .path = strl("index.html"),
            .query = strl("id=foo bar"),
            .is_valid = false,
        },
        {
            .raw = strl("index.html#id=foo bar"),
            .path = strl("index.html"),
            .fragment = strl("id=foo bar"),
            .is_valid = false,
        },
        {
            .raw = strl("//localhost?id=1"),
            .authority = strl("localhost"),
            .host = strl("localhost"),
            .query = strl("id=1"),
            .is_valid = true,
        },
        {
            .raw = strl("//localhost#title"),
            .authority = strl("localhost"),
            .host = strl("localhost"),
            .fragment = strl("title"),
            .is_valid = true,
        },
    };

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
        {strl("//localhost"), strl(""), strl("//localhost")},
        {strl("//localhost"), strl("index.html"), strl("//localhost/index.html")},
        {strl("//localhost"), strl("https:a/.."), strl("https:")},

        // RFC 3986: https://datatracker.ietf.org/doc/html/rfc3986#section-5.4.1
        {strl("http://a/b/c/d;p?q"), strl("g:h"), strl("g:h")},
        {strl("http://a/b/c/d;p?q"), strl("g"), strl("http://a/b/c/g")},
        {strl("http://a/b/c/d;p?q"), strl("./g"), strl("http://a/b/c/g")},
        {strl("http://a/b/c/d;p?q"), strl("g/"), strl("http://a/b/c/g/")},
        {strl("http://a/b/c/d;p?q"), strl("/g"), strl("http://a/g")},
        {strl("http://a/b/c/d;p?q"), strl("//g"), strl("http://g")},
        {strl("http://a/b/c/d;p?q"), strl("?y"), strl("http://a/b/c/d;p?y")},
        {strl("http://a/b/c/d;p?q"), strl("g?y"), strl("http://a/b/c/g?y")},
        {strl("http://a/b/c/d;p?q"), strl("#s"), strl("http://a/b/c/d;p?q#s")},
        {strl("http://a/b/c/d;p?q"), strl("g#s"), strl("http://a/b/c/g#s")},
        {strl("http://a/b/c/d;p?q"), strl("g?y#s"), strl("http://a/b/c/g?y#s")},
        {strl("http://a/b/c/d;p?q"), strl(";x"), strl("http://a/b/c/;x")},
        {strl("http://a/b/c/d;p?q"), strl("g;x"), strl("http://a/b/c/g;x")},
        {strl("http://a/b/c/d;p?q"), strl("g;x?y#s"), strl("http://a/b/c/g;x?y#s")},
        {strl("http://a/b/c/d;p?q"), strl(""), strl("http://a/b/c/d;p?q")},
        {strl("http://a/b/c/d;p?q"), strl("."), strl("http://a/b/c/")},
        {strl("http://a/b/c/d;p?q"), strl("./"), strl("http://a/b/c/")},
        {strl("http://a/b/c/d;p?q"), strl(".."), strl("http://a/b/")},
        {strl("http://a/b/c/d;p?q"), strl("../"), strl("http://a/b/")},
        {strl("http://a/b/c/d;p?q"), strl("../g"), strl("http://a/b/g")},
        {strl("http://a/b/c/d;p?q"), strl("../.."), strl("http://a/")},
        {strl("http://a/b/c/d;p?q"), strl("../../"), strl("http://a/")},
        {strl("http://a/b/c/d;p?q"), strl("../../g"), strl("http://a/g")},
        {strl("http://a/b/c/d;p?q"), strl("/./g"), strl("http://a/g")},
        {strl("http://a/b/c/d;p?q"), strl("/../g"), strl("http://a/g")},
        {strl("http://a/b/c/d;p?q"), strl("g."), strl("http://a/b/c/g.")},
        {strl("http://a/b/c/d;p?q"), strl(".g"), strl("http://a/b/c/.g")},
        {strl("http://a/b/c/d;p?q"), strl("g.."), strl("http://a/b/c/g..")},
        {strl("http://a/b/c/d;p?q"), strl("..g"), strl("http://a/b/c/..g")},
        {strl("http://a/b/c/d;p?q"), strl("./../g"), strl("http://a/b/g")},
        {strl("http://a/b/c/d;p?q"), strl("./g/."), strl("http://a/b/c/g/")},
        {strl("http://a/b/c/d;p?q"), strl("g/./h"), strl("http://a/b/c/g/h")},
        {strl("http://a/b/c/d;p?q"), strl("g/../h"), strl("http://a/b/c/h")},
        {strl("http://a/b/c/d;p?q"), strl("g;x=1/./y"), strl("http://a/b/c/g;x=1/y")},
        {strl("http://a/b/c/d;p?q"), strl("g;x=1/../y"), strl("http://a/b/c/y")},
        {strl("http://a/b/c/d;p?q"), strl("g?y/./x"), strl("http://a/b/c/g?y/./x")},
        {strl("http://a/b/c/d;p?q"), strl("g?y/../x"), strl("http://a/b/c/g?y/../x")},
        {strl("http://a/b/c/d;p?q"), strl("g#s/./x"), strl("http://a/b/c/g#s/./x")},
        {strl("http://a/b/c/d;p?q"), strl("g#s/../x"), strl("http://a/b/c/g#s/../x")},
        {strl("http://a/b/c/d;p?q"), strl("http:g"), strl("http:g")},
    };

    capy_arena *arena = capy_arena_init(0, MiB(8));

    for (size_t i = 0; i < arrlen(uris_reference); i++)
    {
        capy_uri base = capy_uri_parse(uris_reference[i].base);
        capy_uri reference = capy_uri_parse(uris_reference[i].reference);
        capy_uri uri = capy_uri_resolve_reference(arena, base, reference);
        capy_string result = capy_uri_string(arena, uri);

        expect_str_eq(result, uris_reference[i].expected);
    }

    capy_arena_destroy(arena);

    return true;
}

static void test_uri(testbench *t)
{
    runtest(t, test_uri_parse, "capy_uri_parse");
    runtest(t, test_uri_resolve_reference, "capy_uri_resolve_reference");
}
