#include <capy/test.h>

static int test_capy_base64(void)
{
    char content[256];
    size_t bytes;

    bytes = capy_base64url(content, 6, "foobar", false);
    ExpectEqU(bytes, 8);
    ExpectEqMem(content, "Zm9vYmFy", bytes);

    bytes = capy_base64url(content, 6, "abcdef", false);
    ExpectEqU(bytes, 8);
    ExpectEqMem(content, "YWJjZGVm", bytes);

    bytes = capy_base64url(content, 7, "foobarb", false);
    ExpectEqU(bytes, 10);
    ExpectEqMem(content, "Zm9vYmFyYg", bytes);

    bytes = capy_base64url(content, 7, "foobarb", true);
    ExpectEqU(bytes, 12);
    ExpectEqMem(content, "Zm9vYmFyYg==", bytes);

    bytes = capy_base64url(content, 8, "foobarbz", false);
    ExpectEqU(bytes, 11);
    ExpectEqMem(content, "Zm9vYmFyYno", bytes);

    bytes = capy_base64url(content, 8, "foobarbz", true);
    ExpectEqU(bytes, 12);
    ExpectEqMem(content, "Zm9vYmFyYno=", bytes);

    bytes = capy_base64url(content, 3, "\x00\x1F\xBF", false);
    ExpectEqU(bytes, 4);
    ExpectEqMem(content, "AB-_", bytes);

    bytes = capy_base64std(content, 3, "\x00\x1F\xBF", false);
    ExpectEqU(bytes, 4);
    ExpectEqMem(content, "AB+/", bytes);

    return true;
}

static int test_capy_string_base64(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(8));

    capy_string result;

    ExpectOk(capy_string_base64url(arena, &result, Str("foobar"), false));
    ExpectEqStr(result, Str("Zm9vYmFy"));

    ExpectOk(capy_string_base64url(arena, &result, Str("abcdef"), false));
    ExpectEqStr(result, Str("YWJjZGVm"));

    ExpectOk(capy_string_base64url(arena, &result, Str("foobarb"), false));
    ExpectEqStr(result, Str("Zm9vYmFyYg"));

    ExpectOk(capy_string_base64url(arena, &result, Str("foobarb"), true));
    ExpectEqStr(result, Str("Zm9vYmFyYg=="));

    ExpectOk(capy_string_base64url(arena, &result, Str("foobarbz"), false));
    ExpectEqStr(result, Str("Zm9vYmFyYno"));

    ExpectOk(capy_string_base64url(arena, &result, Str("foobarbz"), true));
    ExpectEqStr(result, Str("Zm9vYmFyYno="));

    ExpectOk(capy_string_base64url(arena, &result, Str("\x00\x1F\xBF"), false));
    ExpectEqStr(result, Str("AB-_"));

    ExpectOk(capy_string_base64std(arena, &result, Str("\x00\x1F\xBF"), false));
    ExpectEqStr(result, Str("AB+/"));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_buffer_base64(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(8));

    capy_buffer *buffer = capy_buffer_init(arena, 1024);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 6, "foobar", false));
    ExpectEqMem(buffer->data, "Zm9vYmFy", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 6, "abcdef", false));
    ExpectEqMem(buffer->data, "YWJjZGVm", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 7, "foobarb", false));
    ExpectEqMem(buffer->data, "Zm9vYmFyYg", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 7, "foobarb", true));
    ExpectEqMem(buffer->data, "Zm9vYmFyYg==", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 8, "foobarbz", false));
    ExpectEqMem(buffer->data, "Zm9vYmFyYno", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 8, "foobarbz", true));
    ExpectEqMem(buffer->data, "Zm9vYmFyYno=", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64url(buffer, 3, "\x00\x1F\xBF", false));
    ExpectEqMem(buffer->data, "AB-_", buffer->size);

    buffer->size = 0;
    ExpectOk(capy_buffer_write_base64std(buffer, 3, "\x00\x1F\xBF", false));
    ExpectEqMem(buffer->data, "AB+/", buffer->size);

    capy_arena_destroy(arena);
    return true;
}

static void test_base64(testbench *t)
{
    runtest(t, test_capy_base64, "capy_base64");
    runtest(t, test_capy_string_base64, "capy_string_base64");
    runtest(t, test_capy_buffer_base64, "capy_buffer_base64");
}
