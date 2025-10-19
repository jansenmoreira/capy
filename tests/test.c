#include "arena_test.c"
#include "base64_test.c"
#include "buffer_test.c"
#include "error_test.c"
#include "http_test.c"
#include "json_test.c"
#include "string_test.c"
#include "strmap_test.c"
#include "task_test.c"
#include "uri_test.c"
#include "vec_test.c"

int main(void)
{
    testbench t = {0, 0};

    printf("Running tests...\n\n");

    test_arena(&t);
    test_buffer(&t);
    test_string(&t);
    test_uri(&t);
    test_vec(&t);
    test_http(&t);
    test_base64(&t);
    test_strmap(&t);
    test_error(&t);
    test_json(&t);
    test_task(&t);

    printf("\nSummary - %d of %d tests succeeded\n", t.succeded, t.succeded + t.failed);

    if (t.failed)
    {
        return 1;
    }

    return 0;
}
