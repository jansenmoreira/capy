#include <capy/test.h>

static int test_capy_buffer_init(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    expect_p_eq(capy_buffer_init(arena, KiB(8)), NULL);

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    expect_u_eq(buffer->size, 0);
    expect_u_gte(buffer->capacity, 8);
    expect_p_eq(buffer->arena, arena);
    expect_p_ne(buffer->data, NULL);

    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_wbytes_enomem(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    expect_err(capy_buffer_wbytes(buffer, KiB(8), ""));
    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_resize_enomem(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    expect_err(capy_buffer_resize(buffer, KiB(8)));
    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_format_enomem(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    expect_err(capy_buffer_format(buffer, 0, "%*s", KiB(8), " "));
    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_writes(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);

    expect_ok(capy_buffer_wstring(buffer, strl("foobar\n")));
    expect_ok(capy_buffer_wcstr(buffer, "baz"));
    expect_ok(capy_buffer_wbytes(buffer, 2, "baz"));
    expect_ok(capy_buffer_format(buffer, 50, " %d %.1f", 5, 1.3f));
    expect_s_eq(memcmp(buffer->data, "foobar\nbazba 5 1.3", buffer->size), 0);

    capy_buffer_shl(buffer, 7);
    expect_s_eq(memcmp(buffer->data, "bazba 5 1.3", buffer->size), 0);

    expect_ok(capy_buffer_resize(buffer, 5));
    expect_s_eq(memcmp(buffer->data, "bazba", buffer->size), 0);

    expect_ok(capy_buffer_resize(buffer, 0));
    expect_ok(capy_buffer_format(buffer, 4, "%d", 123456));
    expect_s_eq(memcmp(buffer->data, "1234", buffer->size), 0);

    capy_arena_destroy(arena);
    return true;
}

static void test_buffer(testbench *t)
{
    runtest(t, test_capy_buffer_init, "capy_buffer_init");

    runtest(t, test_buffer_wbytes_enomem,
            "capy_buffer_wbytes: should fail when alloc fails");

    runtest(t, test_buffer_resize_enomem,
            "capy_buffer_resize: should fail when alloc fails");

    runtest(t, test_buffer_format_enomem,
            "capy_buffer_format: should fail when alloc fails");

    runtest(t, test_buffer_writes,
            "capy_buffer_(w*|shl|resize|format): should produce expected text");
}
