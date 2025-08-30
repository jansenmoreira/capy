#include "test.h"

static struct
{
    capy_string request;
    int64_t is_valid;

} requests[] = {
    {
        .request = str("POST /users HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "Content-Type:   application/x-www-form-urlencoded  \r\n"
                       "Content-Type: text/plain\r\n"
                       "Content-Length: 49\r\n"
                       "\r\n"
                       "name=FirstName+LastName&email=bsmth%40example.com"),
        .is_valid = true,
    },
};

static int test_http(void)
{
    // capy_arena *arena = capy_arena_init(GiB(8ULL));

    // for (size_t i = 0; i < arrlen(requests); i++)
    // {
    //     capy_http_request *request = capy_arena_make(capy_http_request, arena, 1);

    //     int err = capy_http_parse_field(arena, requests[i].request, request);

    //     expect_s_eq(err, 0);
    // }

    return 0;
}
