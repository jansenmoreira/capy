#include <capy/test.h>

static int test_arena(void)
{
    capy_arena *arena;
    size_t s16, s32, s64, size;
    void *end, *chunk, *chunk2;

    // init should return NULL for invalid limit

    expect_p_eq(capy_arena_init(-1ULL), NULL);

    // init should return a valid arena when limit is valid

    arena = capy_arena_init(KiB(32));
    expect_p_ne(arena, NULL);

    end = capy_arena_end(arena);

    // alloc should return NULL when requested size exceeds limit

    expect_p_eq(capy_arena_alloc(arena, MiB(1), 0, false), NULL);

    // alloc should return aligned pointers when align is not 0

    s16 = cast(size_t, capy_arena_alloc(arena, sizeof(int16_t), alignof(int16_t), true));
    expect_u_eq(s16 % alignof(int16_t), 0);

    s32 = cast(size_t, capy_arena_alloc(arena, sizeof(int32_t), alignof(int32_t), true));
    expect_u_eq(s32 % alignof(int32_t), 0);

    s64 = cast(size_t, capy_arena_alloc(arena, sizeof(int64_t), alignof(int64_t), true));
    expect_u_eq(s64 % alignof(int64_t), 0);

    // alloc should retun a pointer that fits the requested size

    size = capy_arena_size(arena);
    chunk = capy_arena_alloc(arena, KiB(16), 0, false);
    expect_p_ne(chunk, NULL);
    expect_u_gte(capy_arena_size(arena) - size, KiB(16));

    // free should shrink the arena

    expect_s_eq(capy_arena_free(arena, chunk), 0);
    expect_p_eq(capy_arena_end(arena), chunk);
    expect_s_eq(capy_arena_free(arena, end), 0);
    expect_p_eq(capy_arena_end(arena), end);

    // realloc should zero-copy grow pointer when possible

    size = capy_arena_size(arena);
    chunk = capy_arena_alloc(arena, KiB(1), 8, false);
    expect_p_ne(chunk, NULL);
    chunk2 = capy_arena_realloc(arena, chunk, KiB(1), KiB(2), false);
    expect_p_eq(chunk, chunk2);
    expect_u_gte(capy_arena_size(arena) - size, KiB(2));
    chunk = chunk2;

    // realloc should return NULL when requested size exceeds limit

    expect_p_eq(capy_arena_realloc(arena, chunk, KiB(2), MiB(2), false), NULL);  // zero-copy path
    expect_p_ne(capy_arena_make(arena, int, 1), NULL);                           // trigger copy
    expect_p_eq(capy_arena_realloc(arena, chunk, KiB(2), MiB(2), false), NULL);  // copy path

    // realloc should copy if cannot grow

    size = capy_arena_size(arena);
    chunk2 = capy_arena_realloc(arena, chunk, KiB(2), KiB(4), false);
    expect_p_ne(chunk2, NULL);
    expect_p_ne(chunk, chunk2);
    expect_u_gte(capy_arena_size(arena) - size, KiB(4));

    capy_arena_destroy(arena);

    return 0;
}
