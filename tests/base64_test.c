#include <capy/test.h>

static int test_base64(void)
{
    char buffer[256];
    size_t bytes;

    bytes = capy_base64_url(buffer, 6, "foobar", false);
    expect_u_eq(bytes, 8);
    expect_s_eq(memcmp(buffer, "Zm9vYmFy", bytes), 0);

    bytes = capy_base64_url(buffer, 6, "abcdef", false);
    expect_u_eq(bytes, 8);
    expect_s_eq(memcmp(buffer, "YWJjZGVm", bytes), 0);

    bytes = capy_base64_url(buffer, 7, "foobarb", false);
    expect_u_eq(bytes, 10);
    expect_s_eq(memcmp(buffer, "Zm9vYmFyYg", bytes), 0);

    bytes = capy_base64_url(buffer, 7, "foobarb", true);
    expect_u_eq(bytes, 12);
    expect_s_eq(memcmp(buffer, "Zm9vYmFyYg==", bytes), 0);

    bytes = capy_base64_url(buffer, 8, "foobarbz", false);
    expect_u_eq(bytes, 11);
    expect_s_eq(memcmp(buffer, "Zm9vYmFyYno", bytes), 0);

    bytes = capy_base64_url(buffer, 8, "foobarbz", true);
    expect_u_eq(bytes, 12);
    expect_s_eq(memcmp(buffer, "Zm9vYmFyYno=", bytes), 0);

    bytes = capy_base64_url(buffer, 3, "\x00\x1F\xBF", false);
    expect_u_eq(bytes, 4);
    expect_s_eq(memcmp(buffer, "AB-_", bytes), 0);

    bytes = capy_base64(buffer, 3, "\x00\x1F\xBF", false);
    expect_u_eq(bytes, 4);
    expect_s_eq(memcmp(buffer, "AB+/", bytes), 0);

    capy_string result;
    capy_arena *arena = capy_arena_init(8 * 1024);

    result = capy_string_base64_url(arena, str("foobar"), false);
    expect_str_eq(result, str("Zm9vYmFy"));

    result = capy_string_base64_url(arena, str("abcdef"), false);
    expect_str_eq(result, str("YWJjZGVm"));

    result = capy_string_base64_url(arena, str("foobarb"), false);
    expect_str_eq(result, str("Zm9vYmFyYg"));

    result = capy_string_base64_url(arena, str("foobarb"), true);
    expect_str_eq(result, str("Zm9vYmFyYg=="));

    result = capy_string_base64_url(arena, str("foobarbz"), false);
    expect_str_eq(result, str("Zm9vYmFyYno"));

    result = capy_string_base64_url(arena, str("foobarbz"), true);
    expect_str_eq(result, str("Zm9vYmFyYno="));

    result = capy_string_base64_url(arena, str("\x00\x1F\xBF"), false);
    expect_str_eq(result, str("AB-_"));

    result = capy_string_base64(arena, str("\x00\x1F\xBF"), false);
    expect_str_eq(result, str("AB+/"));

    capy_strbuf *strbuf = capy_strbuf_init(arena, 1024);

    capy_strbuf_resize(strbuf, 0);
    capy_strbuf_base64_url(strbuf, 6, "foobar", false);
    expect_s_eq(memcmp(strbuf->data, "Zm9vYmFy", strbuf->size), 0);

    capy_strbuf_resize(strbuf, 0);
    capy_strbuf_base64_url(strbuf, 6, "abcdef", false);
    expect_s_eq(memcmp(strbuf->data, "YWJjZGVm", strbuf->size), 0);

    capy_strbuf_resize(strbuf, 0);
    capy_strbuf_base64_url(strbuf, 7, "foobarb", false);
    expect_s_eq(memcmp(strbuf->data, "Zm9vYmFyYg", strbuf->size), 0);

    capy_strbuf_resize(strbuf, 0);
    capy_strbuf_base64_url(strbuf, 7, "foobarb", true);
    expect_s_eq(memcmp(strbuf->data, "Zm9vYmFyYg==", strbuf->size), 0);

    capy_strbuf_resize(strbuf, 0);
    capy_strbuf_base64_url(strbuf, 8, "foobarbz", false);
    expect_s_eq(memcmp(strbuf->data, "Zm9vYmFyYno", strbuf->size), 0);

    capy_strbuf_resize(strbuf, 0);
    capy_strbuf_base64_url(strbuf, 8, "foobarbz", true);
    expect_s_eq(memcmp(strbuf->data, "Zm9vYmFyYno=", strbuf->size), 0);

    capy_strbuf_resize(strbuf, 0);
    capy_strbuf_base64_url(strbuf, 3, "\x00\x1F\xBF", false);
    expect_s_eq(memcmp(strbuf->data, "AB-_", strbuf->size), 0);

    capy_strbuf_resize(strbuf, 0);
    capy_strbuf_base64(strbuf, 3, "\x00\x1F\xBF", false);
    expect_s_eq(memcmp(strbuf->data, "AB+/", strbuf->size), 0);

    capy_arena_free(arena);

    return 0;
}
