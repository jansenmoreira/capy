#include "test.h"

capy_string uris_good[] = {
    capy_string_lit("https://user:1234@localhost:8080/?a=b%20b@&c=1#main"),
    capy_string_lit("//user:1234@[1:2:3:4:5:6:7:8]:8080"),
    capy_string_lit("//[1:2:3:4:5:6:7::]"),
    capy_string_lit("//[1:2:3:4:5:6::]"),
    capy_string_lit("//[1:2:3:4:5::]"),
    capy_string_lit("//[1:2:3:4::]"),
    capy_string_lit("//[1:2:3::]"),
    capy_string_lit("//[1:2::]"),
    capy_string_lit("//[1::]"),
    capy_string_lit("//[::2:3:4:5:6:7:8]"),
    capy_string_lit("//[::3:4:5:6:7:8]"),
    capy_string_lit("//[::4:5:6:7:8]"),
    capy_string_lit("//[::5:6:7:8]"),
    capy_string_lit("//[::6:7:8]"),
    capy_string_lit("//[::7:8]"),
    capy_string_lit("//[::8]"),
    capy_string_lit("//[1:2:3:4:5:6:127.0.0.1]"),
    capy_string_lit("//[::2:3:4:5:6:127.0.0.1]"),
    capy_string_lit("//[::3:4:5:6:127.0.0.1]"),
    capy_string_lit("//[::4:5:6:127.0.0.1]"),
    capy_string_lit("//[::5:6:127.0.0.1]"),
    capy_string_lit("//[::6:127.0.0.1]"),
    capy_string_lit("//localhost"),
    capy_string_lit("//example.com"),
    capy_string_lit("//a%20g.example.com"),
    capy_string_lit("//localhost/?a=b:&c=1"),
    capy_string_lit("//localhost/?a=b/&c=1"),
    capy_string_lit("//localhost/?a=b?&c=1"),
    capy_string_lit("//localhost/?a=b@&c=1"),
};

capy_string uris_bad[] = {
    capy_string_lit("1http://user:1234@localhost:8080/?a=b%20b@&c=1#main"),
    capy_string_lit("//[1:2:3:4:5:6:7::8]"),
    capy_string_lit("//[1::2:3:4:5:6:7:8]"),
    capy_string_lit("//[1:2:3:4::5:6:7:8]"),
    capy_string_lit("//[1:2:3:4:5:6:7:127.0.0.1]"),
    capy_string_lit("//[1::2:3:4:5:6:7:127.0.0.1]"),
    capy_string_lit("//a%2g.example.com"),
    capy_string_lit("//localhost/?a=[b]&c=1"),
    capy_string_lit("//localhost/?a=b%2g&c=1"),
};

int test_uri(void)
{
    capy_arena *arena = capy_arena_init(GiB(8ULL));
    capy_URI *uri;

    for (int i = 0; i < (sizeof(uris_good) / sizeof(capy_string)); i++)
    {
        uri = capy_URI_decode(arena, uris_good[i]);
        expect(uri != NULL);
    }

    for (int i = 0; i < (sizeof(uris_bad) / sizeof(capy_string)); i++)
    {
        uri = capy_URI_decode(arena, uris_bad[i]);
        expect(uri == NULL);
    }

    return 0;
}
