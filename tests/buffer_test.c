#include <capy/test.h>

static int test_capy_buffer_init(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    ExpectNull(capy_buffer_init(arena, KiB(8)));

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    ExpectEqU(buffer->size, 0);
    ExpectGteU(buffer->capacity, 8);
    ExpectEqPtr(buffer->arena, arena);
    ExpectNotNull(buffer->data);

    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_wbytes_enomem(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    ExpectErr(capy_buffer_write_bytes(buffer, KiB(8), ""));
    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_resize_enomem(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    ExpectErr(capy_buffer_resize(buffer, KiB(8)));
    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_format_enomem(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);
    ExpectErr(capy_buffer_write_fmt(buffer, 0, "%*s", KiB(8), " "));
    capy_arena_destroy(arena);
    return true;
}

static int test_buffer_writes(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_buffer *buffer = capy_buffer_init(arena, 8);

    ExpectOk(capy_buffer_write_string(buffer, Str("foobar\n")));
    ExpectOk(capy_buffer_write_cstr(buffer, "baz"));
    ExpectOk(capy_buffer_write_bytes(buffer, 2, "baz"));
    ExpectOk(capy_buffer_write_fmt(buffer, 50, " %d %.1f", 5, 1.3f));
    ExpectEqMem(buffer->data, "foobar\nbazba 5 1.3", buffer->size);

    capy_buffer_shl(buffer, 7);
    ExpectEqMem(buffer->data, "bazba 5 1.3", buffer->size);

    ExpectOk(capy_buffer_resize(buffer, 5));
    ExpectEqMem(buffer->data, "bazba", buffer->size);

    ExpectOk(capy_buffer_resize(buffer, 0));
    ExpectOk(capy_buffer_write_fmt(buffer, 4, "%d", 123456));
    ExpectEqMem(buffer->data, "1234", buffer->size);

    capy_arena_destroy(arena);
    return true;
}

static void test_buffer(testbench *t)
{
    runtest(t, test_capy_buffer_init, "capy_buffer_init");

    runtest(t, test_buffer_wbytes_enomem,
            "capy_buffer_write_bytes: should fail when alloc fails");

    runtest(t, test_buffer_resize_enomem,
            "capy_buffer_resize: should fail when alloc fails");

    runtest(t, test_buffer_format_enomem,
            "capy_buffer_write_fmt: should fail when alloc fails");

    runtest(t, test_buffer_writes,
            "capy_buffer_(w*|shl|resize|format): should produce expected text");
}
