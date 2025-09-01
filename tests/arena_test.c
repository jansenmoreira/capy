#include "test.h"

static int test_arena(void)
{
    int err;

    size_t limit = GiB(8ULL);

    // expect_s_lt(capy_arena_init(TiB(128ULL)), 0);

    capy_arena *arena = capy_arena_init(limit);
    expect_p_ne(arena, NULL);

    capy_arena *addr = arena;

    uint8_t *last_top = capy_arena_top(arena);
    uint8_t *a1 = capy_arena_make(uint8_t, arena, 99);

    void *top = capy_arena_top(arena);

    expect_p_ne(a1, NULL);
    expect_p_eq(arena, addr);
    expect_u_eq((size_t)(a1) % alignof(uint8_t), 0);
    expect_s_gte((uint8_t *)(capy_arena_top(arena)) - last_top, 99);

    last_top = capy_arena_top(arena);
    uint16_t *a2 = capy_arena_make(uint16_t, arena, 49);

    expect_p_ne(a2, NULL);
    expect_p_eq(arena, addr);
    expect_u_eq((size_t)(a2) % alignof(uint16_t), 0);
    expect_s_gte((uint8_t *)(capy_arena_top(arena)) - last_top, sizeof(uint16_t) * 49);

    last_top = capy_arena_top(arena);
    double *a3 = capy_arena_make(double, arena, 3333);

    expect_p_ne(a3, NULL);
    expect_p_eq(arena, addr);
    expect_u_eq((size_t)(a3) % alignof(double), 0);
    expect_s_gte((uint8_t *)(capy_arena_top(arena)) - last_top, sizeof(double) * 3333);

    last_top = capy_arena_top(arena);
    uint8_t *a4 = capy_arena_make(uint8_t, arena, 1);

    expect_p_ne(a4, NULL);
    expect_p_eq(arena, addr);
    expect_u_eq((size_t)(a4) % alignof(uint8_t), 0);
    expect_s_gte((uint8_t *)(capy_arena_top(arena)) - last_top, 1);

    last_top = capy_arena_top(arena);
    struct point *a5 = capy_arena_make(struct point, arena, 1);

    expect_p_ne(a5, NULL);
    expect_p_eq(arena, addr);
    expect_u_eq((size_t)(a5) % alignof(struct point), 0);
    expect_s_gte((uint8_t *)(capy_arena_top(arena)) - last_top, sizeof(struct point) * 1);

    err = capy_arena_shrink(arena, a3);
    expect_s_eq(err, 0);
    expect_p_eq((uint8_t *)(a3), (uint8_t *)(capy_arena_top(arena)));

    err = capy_arena_shrink(arena, top);
    expect_s_eq(err, 0);
    expect_p_eq((uint8_t *)(top), (uint8_t *)(capy_arena_top(arena)));

    err = capy_arena_free(arena);
    expect_s_eq(err, 0);

    return 0;
}
