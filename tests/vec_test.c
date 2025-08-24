#include "capy/vec.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>

#include "capy/arena.h"
#include "test.h"

int test_vec(void)
{
    capy_arena *arena1 = capy_arena_init(KiB(1));

    int16_t *vec1 = capy_vec_of(int16_t, arena1, 8);

    int err = capy_vec_insert(&vec1, 1, 1, NULL);
    expect_s_eq(err, EINVAL);

    err = capy_vec_insert(&vec1, 0, KiB(2), NULL);
    expect_s_eq(err, ENOMEM);

    for (int16_t i = 1; i <= 10; i++)
    {
        err = capy_vec_push(&vec1, &i);
        expect_s_eq(err, 0);
    }

    int16_t buffer1[] = {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10};
    int16_t expected[] = {1, 2, 3, 4, 5, -1, -2, -3, -4, -5, 6, 7, 8, 9, 10, -6, -7, -8, -9, -10, 0};

    err = capy_vec_insert(&vec1, 5, 5, buffer1);
    expect_s_eq(err, 0);

    err = capy_vec_insert(&vec1, capy_vec_size(vec1), 5, buffer1 + 5);
    expect_s_eq(err, 0);

    err = capy_vec_insert(&vec1, capy_vec_size(vec1), 1, NULL);
    expect_s_eq(err, 0);
    expect_u_eq(capy_vec_size(vec1), 21);

    for (int16_t i = 0; i < arrlen(expected); i++)
    {
        expect_u_eq(vec1[i], expected[i]);
    }

    err = capy_vec_pop(&vec1);
    expect_s_eq(err, 0);
    expect_u_eq(capy_vec_size(vec1), 20);

    err = capy_vec_delete(&vec1, 5, 5);
    expect_s_eq(err, 0);
    expect_u_eq(capy_vec_size(vec1), 15);

    err = capy_vec_delete(&vec1, 10, 5);
    expect_s_eq(err, 0);
    expect_u_eq(capy_vec_size(vec1), 10);

    for (int16_t i = 0; i < 10; i++)
    {
        expect_u_eq(vec1[i], i + 1);
    }

    err = capy_vec_delete(&vec1, 0, 10);
    expect_s_eq(err, 0);
    expect_u_eq(capy_vec_size(vec1), 0);

    err = capy_vec_pop(&vec1);
    expect_s_eq(err, EINVAL);

    err = capy_vec_delete(&vec1, 0, 1);
    expect_s_eq(err, EINVAL);

    float *vec2 = capy_arena_make(float, arena1, 64);
    expect_p_ne(vec2, NULL);

    int16_t *last_vec1 = vec1;
    err = capy_vec_resize(&vec1, 64);
    expect_s_eq(err, 0);
    expect_p_ne(vec1, last_vec1);

    vec2 = capy_arena_make(float, arena1, 64);
    expect_p_ne(vec2, NULL);

    err = capy_vec_resize(&vec1, KiB(2));
    expect_s_eq(err, ENOMEM);

    capy_vec_fixed(vec1);

    err = capy_vec_resize(&vec1, 128);
    expect_s_eq(err, 0);

    err = capy_vec_resize(&vec1, 128);
    expect_s_eq(err, 0);

    err = capy_vec_reserve(&vec1, 64);
    expect_s_eq(err, 0);

    err = capy_vec_resize(&vec1, KiB(2));
    expect_s_eq(err, ENOMEM);

    return 0;
}
