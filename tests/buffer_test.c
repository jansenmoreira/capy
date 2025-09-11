#include <capy/test.h>

static int test_buffer(void)
{
    capy_arena *arena;
    capy_buffer *buffer;

    arena = capy_arena_init(KiB(4));

    expect_p_eq(capy_buffer_init(arena, KiB(8)), NULL);

    buffer = capy_buffer_init(arena, 8);
    expect_u_eq(buffer->size, 0);
    expect_u_eq(buffer->capacity, 8);
    expect_p_eq(buffer->arena, arena);
    expect_p_ne(buffer->data, NULL);

    expect_s_eq(capy_buffer_wbytes(buffer, KiB(8), ""), ENOMEM);
    expect_s_eq(capy_buffer_resize(buffer, KiB(8)), ENOMEM);
    expect_s_eq(capy_buffer_format(buffer, 0, "%*s", KiB(8), " "), ENOMEM);

    expect_s_eq(capy_buffer_wstring(buffer, str("foobar\n")), 0);
    expect_s_eq(capy_buffer_wcstr(buffer, "baz"), 0);
    expect_s_eq(capy_buffer_wbytes(buffer, 2, "baz"), 0);
    expect_s_eq(capy_buffer_format(buffer, 50, " %d %.1f", 5, 1.3f), 0);
    expect_s_eq(memcmp(buffer->data, "foobar\nbazba 5 1.3", buffer->size), 0);

    capy_buffer_shl(buffer, 7);
    expect_s_eq(memcmp(buffer->data, "bazba 5 1.3", buffer->size), 0);

    expect_s_eq(capy_buffer_resize(buffer, 5), 0);
    expect_s_eq(memcmp(buffer->data, "bazba", buffer->size), 0);

    expect_s_eq(capy_buffer_resize(buffer, 0), 0);
    expect_s_eq(capy_buffer_format(buffer, 4, "%d", 123456), 0);
    expect_s_eq(memcmp(buffer->data, "1234", buffer->size), 0);

    capy_arena_destroy(arena);

    return 0;
}
