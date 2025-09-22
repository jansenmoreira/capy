#include <capy/macros.h>
#include <capy/test.h>

static int test_capy_arena_init(void)
{
    ExpectNull(capy_arena_init(0, -1ULL));

    capy_arena *arena = capy_arena_init(0, KiB(32));
    ExpectNotNull(arena);

    ExpectOk(capy_arena_destroy(arena));

    return true;
}

static int test_capy_arena_alloc(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    size_t s16 = Cast(size_t, capy_arena_alloc(arena, sizeof(int16_t), alignof(int16_t), true));
    ExpectEqU(s16 % alignof(int16_t), 0);

    size_t s32 = Cast(size_t, capy_arena_alloc(arena, sizeof(int32_t), alignof(int32_t), true));
    ExpectEqU(s32 % alignof(int32_t), 0);

    size_t s64 = Cast(size_t, capy_arena_alloc(arena, sizeof(int64_t), alignof(int64_t), true));
    ExpectEqU(s64 % alignof(int64_t), 0);

    size_t size = capy_arena_size(arena);
    void *chunk = capy_arena_alloc(arena, KiB(16), 0, false);
    ExpectNotNull(chunk);
    ExpectGteU(capy_arena_size(arena) - size, KiB(16));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_arena_free(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    void *end = capy_arena_end(arena);
    void *chunk = capy_arena_alloc(arena, KiB(16), 0, false);

    ExpectOk(capy_arena_free(arena, chunk));
    ExpectEqPtr(capy_arena_end(arena), chunk);
    ExpectOk(capy_arena_free(arena, end));
    ExpectEqPtr(capy_arena_end(arena), end);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_arena_realloc(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(32));

    size_t size = capy_arena_size(arena);
    void *data = capy_arena_alloc(arena, KiB(1), 8, false);
    ExpectNotNull(data);
    void *tmp = capy_arena_realloc(arena, data, KiB(1), KiB(2), false);
    ExpectEqPtr(data, tmp);
    ExpectGteU(capy_arena_size(arena) - size, KiB(2));

    data = tmp;

    ExpectNotNull(Make(arena, int, 1));
    size = capy_arena_size(arena);
    tmp = capy_arena_realloc(arena, data, KiB(2), KiB(4), false);

    ExpectNotNull(tmp);
    ExpectNePtr(data, tmp);
    ExpectGteU(capy_arena_size(arena) - size, KiB(2));

    data = tmp;

    ExpectNull(capy_arena_realloc(arena, data, KiB(2), MiB(2), false));
    ExpectNotNull(Make(arena, int, 1));
    ExpectNull(capy_arena_realloc(arena, data, KiB(2), MiB(2), false));

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
