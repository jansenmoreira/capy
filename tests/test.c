#include "arena_test.c"
#include "base64_test.c"
#include "http_test.c"
#include "smap_test.c"
#include "string_test.c"
#include "uri_test.c"
#include "vec_test.c"

int main(void)
{
    test_arena();
    test_vec();
    test_string();
    test_uri();
    test_smap();
    test_http();
    test_base64();
}
