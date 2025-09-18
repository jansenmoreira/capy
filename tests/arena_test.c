#include <capy/macros.h>
#include <capy/test.h>

static int test_capy_arena_init(void)
{
    expect_null(capy_arena_init(0, -1ULL));

    capy_arena *arena = capy_arena_init(0, KiB(32));
    expect_notnull(arena);

    expect_ok(capy_arena_destroy(arena));

    return true;
}

static int test_capy_arena_alloc(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    size_t s16 = cast(size_t, capy_arena_alloc(arena, sizeof(int16_t), alignof(int16_t), true));
    expect_u_eq(s16 % alignof(int16_t), 0);

    size_t s32 = cast(size_t, capy_arena_alloc(arena, sizeof(int32_t), alignof(int32_t), true));
    expect_u_eq(s32 % alignof(int32_t), 0);

    size_t s64 = cast(size_t, capy_arena_alloc(arena, sizeof(int64_t), alignof(int64_t), true));
    expect_u_eq(s64 % alignof(int64_t), 0);

    size_t size = capy_arena_size(arena);
    void *chunk = capy_arena_alloc(arena, KiB(16), 0, false);
    expect_p_ne(chunk, NULL);
    expect_u_gte(capy_arena_size(arena) - size, KiB(16));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_arena_free(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    void *end = capy_arena_end(arena);
    void *chunk = capy_arena_alloc(arena, KiB(16), 0, false);

    expect_ok(capy_arena_free(arena, chunk));
    expect_p_eq(capy_arena_end(arena), chunk);
    expect_ok(capy_arena_free(arena, end));
    expect_p_eq(capy_arena_end(arena), end);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_arena_realloc(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    size_t size = capy_arena_size(arena);
    void *data = capy_arena_alloc(arena, KiB(1), 8, false);
    expect_p_ne(data, NULL);
    void *tmp = capy_arena_realloc(arena, data, KiB(1), KiB(2), false);
    expect_p_eq(data, tmp);
    expect_u_gte(capy_arena_size(arena) - size, KiB(2));

    data = tmp;

    expect_p_ne(make(arena, int, 1), NULL);
    size = capy_arena_size(arena);
    tmp = capy_arena_realloc(arena, data, KiB(2), KiB(4), false);

    expect_p_ne(tmp, NULL);
    expect_p_ne(data, tmp);
    expect_u_gte(capy_arena_size(arena) - size, KiB(2));

    data = tmp;

    expect_p_eq(capy_arena_realloc(arena, data, KiB(2), MiB(2), false), NULL);
    expect_p_ne(make(arena, int, 1), NULL);
    expect_p_eq(capy_arena_realloc(arena, data, KiB(2), MiB(2), false), NULL);

    capy_arena_destroy(arena);
    return true;
}

static void test_arena(testbench *t)
{
    runtest(t, test_capy_arena_init, "capy_arena_init");
    runtest(t, test_capy_arena_alloc, "capy_arena_alloc");
    runtest(t, test_capy_arena_free, "capy_arena_free");
    runtest(t, test_capy_arena_realloc, "capy_arena_realloc");
}
