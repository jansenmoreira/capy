#include "test.h"

static int test_vec(void)
{
    capy_arena *arena = capy_arena_init(KiB(1));

    int16_t *vec = capy_vec_of(int16_t, arena, 8);
    int16_t *vec_tmp;

    expect_u_eq(capy_vec_capacity(vec), 8);

    vec_tmp = capy_vec_insert(vec, 0, KiB(2), NULL);
    expect_p_eq(vec_tmp, NULL);

    for (int16_t i = 1; i <= 10; i++)
    {
        vec = capy_vec_push(vec, &i);
        expect_p_ne(vec, NULL);
    }

    int16_t buffer1[] = {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10};
    int16_t expected[] = {1, 2, 3, 4, 5, -1, -2, -3, -4, -5, 6, 7, 8, 9, 10, -6, -7, -8, -9, -10, 0};

    vec = capy_vec_insert(vec, 5, 5, buffer1);
    expect_p_ne(vec, NULL);

    vec = capy_vec_insert(vec, capy_vec_size(vec), 5, buffer1 + 5);
    expect_p_ne(vec, NULL);

    vec = capy_vec_insert(vec, capy_vec_size(vec), 1, NULL);
    expect_p_ne(vec, NULL);
    expect_u_eq(capy_vec_size(vec), 21);

    for (size_t i = 0; i < arrlen(expected); i++)
    {
        expect_s_eq(vec[i], expected[i]);
    }

    vec = capy_vec_pop(vec);
    expect_p_ne(vec, NULL);
    expect_u_eq(capy_vec_size(vec), 20);

    vec = capy_vec_delete(vec, 5, 5);
    expect_p_ne(vec, NULL);
    expect_u_eq(capy_vec_size(vec), 15);

    vec = capy_vec_delete(vec, 10, 5);
    expect_p_ne(vec, NULL);
    expect_u_eq(capy_vec_size(vec), 10);

    for (int16_t i = 0; i < 10; i++)
    {
        expect_s_eq(vec[i], i + 1);
    }

    vec = capy_vec_delete(vec, 0, 10);
    expect_p_ne(vec, NULL);
    expect_u_eq(capy_vec_size(vec), 0);

    float *vec2 = capy_arena_make(float, arena, 64);
    expect_p_ne(vec2, NULL);

    int16_t *last_vec1 = vec;
    vec = capy_vec_resize(vec, 64);
    expect_p_ne(vec, NULL);
    expect_p_ne(vec, last_vec1);

    vec2 = capy_arena_make(float, arena, 64);
    expect_p_ne(vec2, NULL);

    vec_tmp = capy_vec_resize(vec, KiB(2));
    expect_p_eq(vec_tmp, NULL);

    capy_vec_fixed(vec);

    vec = capy_vec_resize(vec, 128);
    expect_p_ne(vec, NULL);

    vec = capy_vec_resize(vec, 128);
    expect_p_ne(vec, NULL);

    vec = capy_vec_reserve(vec, 64);
    expect_p_ne(vec, NULL);

    vec = capy_vec_resize(vec, KiB(2));
    expect_p_eq(vec, NULL);

    return 0;
}
