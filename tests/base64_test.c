#include <capy/test.h>

static int test_capy_base64(void)
{
    char content[256];
    size_t bytes;

    bytes = capy_base64url(content, 6, "foobar", false);
    expect_u_eq(bytes, 8);
    expect_s_eq(memcmp(content, "Zm9vYmFy", bytes), 0);

    bytes = capy_base64url(content, 6, "abcdef", false);
    expect_u_eq(bytes, 8);
    expect_s_eq(memcmp(content, "YWJjZGVm", bytes), 0);

    bytes = capy_base64url(content, 7, "foobarb", false);
    expect_u_eq(bytes, 10);
    expect_s_eq(memcmp(content, "Zm9vYmFyYg", bytes), 0);

    bytes = capy_base64url(content, 7, "foobarb", true);
    expect_u_eq(bytes, 12);
    expect_s_eq(memcmp(content, "Zm9vYmFyYg==", bytes), 0);

    bytes = capy_base64url(content, 8, "foobarbz", false);
    expect_u_eq(bytes, 11);
    expect_s_eq(memcmp(content, "Zm9vYmFyYno", bytes), 0);

    bytes = capy_base64url(content, 8, "foobarbz", true);
    expect_u_eq(bytes, 12);
    expect_s_eq(memcmp(content, "Zm9vYmFyYno=", bytes), 0);

    bytes = capy_base64url(content, 3, "\x00\x1F\xBF", false);
    expect_u_eq(bytes, 4);
    expect_s_eq(memcmp(content, "AB-_", bytes), 0);

    bytes = capy_base64(content, 3, "\x00\x1F\xBF", false);
    expect_u_eq(bytes, 4);
    expect_s_eq(memcmp(content, "AB+/", bytes), 0);

    return true;
}

static int test_capy_string_base64(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(8));

    capy_string result;

    expect_ok(capy_string_base64url(arena, &result, strl("foobar"), false));
    expect_str_eq(result, strl("Zm9vYmFy"));

    expect_ok(capy_string_base64url(arena, &result, strl("abcdef"), false));
    expect_str_eq(result, strl("YWJjZGVm"));

    expect_ok(capy_string_base64url(arena, &result, strl("foobarb"), false));
    expect_str_eq(result, strl("Zm9vYmFyYg"));

    expect_ok(capy_string_base64url(arena, &result, strl("foobarb"), true));
    expect_str_eq(result, strl("Zm9vYmFyYg=="));

    expect_ok(capy_string_base64url(arena, &result, strl("foobarbz"), false));
    expect_str_eq(result, strl("Zm9vYmFyYno"));

    expect_ok(capy_string_base64url(arena, &result, strl("foobarbz"), true));
    expect_str_eq(result, strl("Zm9vYmFyYno="));

    expect_ok(capy_string_base64url(arena, &result, strl("\x00\x1F\xBF"), false));
    expect_str_eq(result, strl("AB-_"));

    expect_ok(capy_string_base64(arena, &result, strl("\x00\x1F\xBF"), false));
    expect_str_eq(result, strl("AB+/"));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_buffer_base64(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(8));

    capy_buffer *buffer = capy_buffer_init(arena, 1024);

    expect_ok(capy_buffer_resize(buffer, 0));
    expect_ok(capy_buffer_wbase64url(buffer, 6, "foobar", false));
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFy", buffer->size), 0);

    expect_ok(capy_buffer_resize(buffer, 0));
    expect_ok(capy_buffer_wbase64url(buffer, 6, "abcdef", false));
    expect_s_eq(memcmp(buffer->data, "YWJjZGVm", buffer->size), 0);

    expect_ok(capy_buffer_resize(buffer, 0));
    expect_ok(capy_buffer_wbase64url(buffer, 7, "foobarb", false));
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFyYg", buffer->size), 0);

    expect_ok(capy_buffer_resize(buffer, 0));
    expect_ok(capy_buffer_wbase64url(buffer, 7, "foobarb", true));
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFyYg==", buffer->size), 0);

    expect_ok(capy_buffer_resize(buffer, 0));
    expect_ok(capy_buffer_wbase64url(buffer, 8, "foobarbz", false));
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFyYno", buffer->size), 0);

    expect_ok(capy_buffer_resize(buffer, 0));
    expect_ok(capy_buffer_wbase64url(buffer, 8, "foobarbz", true));
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFyYno=", buffer->size), 0);

    expect_ok(capy_buffer_resize(buffer, 0));
    expect_ok(capy_buffer_wbase64url(buffer, 3, "\x00\x1F\xBF", false));
    expect_s_eq(memcmp(buffer->data, "AB-_", buffer->size), 0);

    expect_ok(capy_buffer_resize(buffer, 0));
    expect_ok(capy_buffer_wbase64(buffer, 3, "\x00\x1F\xBF", false));
    expect_s_eq(memcmp(buffer->data, "AB+/", buffer->size), 0);

    capy_arena_destroy(arena);
    return true;
}

static void test_base64(testbench *t)
{
    runtest(t, test_capy_base64, "capy_base64");
    runtest(t, test_capy_string_base64, "capy_string_base64");
    runtest(t, test_capy_buffer_base64, "capy_buffer_base64");
}
