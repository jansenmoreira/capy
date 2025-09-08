#include <capy/test.h>

static int test_base64(void)
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

    capy_string result;
    capy_arena *arena = capy_arena_init(8 * 1024);

    result = capy_string_base64url(arena, str("foobar"), false);
    expect_str_eq(result, str("Zm9vYmFy"));

    result = capy_string_base64url(arena, str("abcdef"), false);
    expect_str_eq(result, str("YWJjZGVm"));

    result = capy_string_base64url(arena, str("foobarb"), false);
    expect_str_eq(result, str("Zm9vYmFyYg"));

    result = capy_string_base64url(arena, str("foobarb"), true);
    expect_str_eq(result, str("Zm9vYmFyYg=="));

    result = capy_string_base64url(arena, str("foobarbz"), false);
    expect_str_eq(result, str("Zm9vYmFyYno"));

    result = capy_string_base64url(arena, str("foobarbz"), true);
    expect_str_eq(result, str("Zm9vYmFyYno="));

    result = capy_string_base64url(arena, str("\x00\x1F\xBF"), false);
    expect_str_eq(result, str("AB-_"));

    result = capy_string_base64(arena, str("\x00\x1F\xBF"), false);
    expect_str_eq(result, str("AB+/"));

    capy_buffer *buffer = capy_buffer_init(arena, 1024);

    capy_buffer_resize(buffer, 0);
    capy_buffer_base64url(buffer, 6, "foobar", false);
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFy", buffer->size), 0);

    capy_buffer_resize(buffer, 0);
    capy_buffer_base64url(buffer, 6, "abcdef", false);
    expect_s_eq(memcmp(buffer->data, "YWJjZGVm", buffer->size), 0);

    capy_buffer_resize(buffer, 0);
    capy_buffer_base64url(buffer, 7, "foobarb", false);
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFyYg", buffer->size), 0);

    capy_buffer_resize(buffer, 0);
    capy_buffer_base64url(buffer, 7, "foobarb", true);
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFyYg==", buffer->size), 0);

    capy_buffer_resize(buffer, 0);
    capy_buffer_base64url(buffer, 8, "foobarbz", false);
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFyYno", buffer->size), 0);

    capy_buffer_resize(buffer, 0);
    capy_buffer_base64url(buffer, 8, "foobarbz", true);
    expect_s_eq(memcmp(buffer->data, "Zm9vYmFyYno=", buffer->size), 0);

    capy_buffer_resize(buffer, 0);
    capy_buffer_base64url(buffer, 3, "\x00\x1F\xBF", false);
    expect_s_eq(memcmp(buffer->data, "AB-_", buffer->size), 0);

    capy_buffer_resize(buffer, 0);
    capy_buffer_base64(buffer, 3, "\x00\x1F\xBF", false);
    expect_s_eq(memcmp(buffer->data, "AB+/", buffer->size), 0);

    capy_arena_free(arena);

    return 0;
}
