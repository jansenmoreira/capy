#include <capy/test.h>

static int test_vec(void)
{
    capy_arena *arena = capy_arena_init(KiB(1));

    capy_vec *vec = capy_vec_of(int16_t, arena, 8);

    expect_u_eq(vec->capacity, 8);

    for (int16_t i = 1; i <= 10; i++)
    {
        capy_vec_push(vec, &i);
        expect_p_ne(vec, NULL);
    }

    int16_t buffer1[] = {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10};
    int16_t expected[] = {1, 2, 3, 4, 5, -1, -2, -3, -4, -5, 6, 7, 8, 9, 10, -6, -7, -8, -9, -10, 0};

    capy_vec_insert(vec, 5, 5, buffer1);
    expect_p_ne(vec, NULL);

    capy_vec_insert(vec, vec->size, 5, buffer1 + 5);
    expect_p_ne(vec, NULL);

    capy_vec_insert(vec, vec->size, 1, NULL);
    expect_p_ne(vec, NULL);
    expect_u_eq(vec->size, 21);

    for (size_t i = 0; i < arrlen(expected); i++)
    {
        expect_s_eq(capy_vec_data(int16_t, vec)[i], expected[i]);
    }

    capy_vec_pop(vec);
    expect_p_ne(vec, NULL);
    expect_u_eq(vec->size, 20);

    capy_vec_delete(vec, 5, 5);
    expect_p_ne(vec, NULL);
    expect_u_eq(vec->size, 15);

    capy_vec_delete(vec, 10, 5);
    expect_p_ne(vec, NULL);
    expect_u_eq(vec->size, 10);

    for (int16_t i = 0; i < 10; i++)
    {
        expect_s_eq(capy_vec_data(int16_t, vec)[i], i + 1);
    }

    capy_vec_delete(vec, 0, 10);
    expect_p_ne(vec, NULL);
    expect_u_eq(vec->size, 0);

    float *vec2 = capy_arena_make(float, arena, 64);
    expect_p_ne(vec2, NULL);

    void *last_vec1 = vec->data;
    capy_vec_resize(vec, 64);
    expect_p_ne(vec, NULL);
    expect_p_ne(vec->data, last_vec1);

    vec2 = capy_arena_make(float, arena, 64);
    expect_p_ne(vec2, NULL);

    vec->arena = NULL;

    capy_vec_resize(vec, 128);
    expect_p_ne(vec, NULL);

    capy_vec_reserve(vec, 64);
    expect_p_ne(vec, NULL);

    return 0;
}
