#include <capy/test.h>

static int test_vec(void)
{
    capy_arena *arena = capy_arena_init(KiB(4));

    size_t size = 0;
    size_t capacity = 4;

    int *data, *tmp;

    data = capy_arena_make(arena, int, capacity);

    data = capy_vec_reserve(arena, data, sizeof(int), &capacity, 2);
    expect_p_ne(data, NULL);
    expect_u_eq(capacity, 4);

    data = capy_vec_reserve(arena, data, sizeof(int), &capacity, 8);
    expect_p_ne(data, NULL);
    expect_u_gte(capacity, 8);

    tmp = capy_vec_reserve(arena, data, sizeof(int), &capacity, MiB(1));
    expect_p_eq(tmp, NULL);

    // should fail when insert is too big

    tmp = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, MiB(1), NULL);
    expect_p_eq(tmp, NULL);

    tmp = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 10, 10, NULL);
    expect_p_eq(tmp, NULL);

    data = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, 5, arr(int, 6, 7, 8, 9, 10));
    expect_p_ne(data, NULL);
    expect_u_eq(size, 5);

    data = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, 5, arr(int, 1, 2, 3, 4, 5));
    expect_p_ne(data, NULL);
    expect_u_eq(size, 10);

    data = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, 1, NULL);
    expect_p_ne(data, NULL);
    expect_u_eq(size, 11);

    data[0] = 0;

    for (int i = 0; i < 11; i++)
    {
        expect_s_eq(data[i], i);
    }

    expect_s_eq(capy_vec_delete(data, sizeof(int), &size, 11, 1), EINVAL);

    expect_s_eq(capy_vec_delete(data, sizeof(int), &size, 11, 0), 0);
    expect_u_eq(size, 11);

    expect_s_eq(capy_vec_delete(data, sizeof(int), &size, 0, 5), 0);
    expect_u_eq(size, 6);

    expect_s_eq(capy_vec_delete(data, sizeof(int), &size, 0, 6), 0);
    expect_u_eq(size, 0);

    return 0;
}
