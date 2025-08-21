#include "test.h"

int test_arena(void)
{
    int err;

    ptrdiff_t limit = GiB(8ULL);

    capy_arena *arena = capy_arena_init(limit);

    expect(arena != NULL);
    expect(arena->limit = limit);

    capy_arena *addr = arena;

    ptrdiff_t last_size = arena->size;
    uint8_t *a1 = capy_arena_make(uint8_t, arena, 99);

    void *top = capy_arena_top(arena);

    expect(a1 != NULL);
    expect(arena == addr);
    expect((ptrdiff_t)(a1) % alignof(uint8_t) == 0);
    expect(arena->size - last_size >= 99);

    last_size = arena->size;
    uint16_t *a2 = capy_arena_make(uint16_t, arena, 49);

    expect(a2 != NULL);
    expect(arena == addr);
    expect((ptrdiff_t)(a2) % alignof(uint16_t) == 0);
    expect(arena->size - last_size >= sizeof(uint16_t) * 49);

    last_size = arena->size;
    double *a3 = capy_arena_make(double, arena, 3333);

    expect(a3 != NULL);
    expect(arena == addr);
    expect((ptrdiff_t)(a3) % alignof(double) == 0);
    expect(arena->size - last_size >= sizeof(double) * 3333);

    last_size = arena->size;
    uint8_t *a4 = capy_arena_make(uint8_t, arena, 1);

    expect(a4 != NULL);
    expect(arena == addr);
    expect((ptrdiff_t)(a4) % alignof(uint8_t) == 0);
    expect(arena->size - last_size >= 1);

    last_size = arena->size;
    struct point *a5 = capy_arena_make(struct point, arena, 1);

    expect(a5 != NULL);
    expect(arena == addr);
    expect((ptrdiff_t)(a5) % alignof(struct point) == 0);
    expect(arena->size - last_size >= sizeof(struct point) * 1);

    err = capy_arena_shrink(arena, a3);
    expect(err == 0);
    expect((uint8_t *)(a3) == (uint8_t *)(arena) + arena->size);

    err = capy_arena_shrink(arena, top);
    expect(err == 0);
    expect((uint8_t *)(top) == (uint8_t *)(arena) + arena->size);

    err = capy_arena_shrink(arena, (uint8_t *)(arena) + arena->size + 1);
    expect(err == EINVAL);

    err = capy_arena_shrink(arena, arena);
    expect(err == EINVAL);

    err = capy_arena_shrink(arena, NULL);
    expect(err == EINVAL);

    err = capy_arena_free(arena), arena = NULL;
    expect(err == 0);

    return 0;
}
