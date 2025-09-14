#include <capy/test.h>

static int test_capy_vec_reserve(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    size_t capacity = 4;
    int *data = make(arena, int, capacity);

    data = capy_vec_reserve(arena, data, sizeof(int), &capacity, 2);
    expect_p_ne(data, NULL);
    expect_u_eq(capacity, 4);

    data = capy_vec_reserve(arena, data, sizeof(int), &capacity, 8);
    expect_p_ne(data, NULL);
    expect_u_gte(capacity, 8);

    expect_p_eq(capy_vec_reserve(arena, data, sizeof(int), &capacity, MiB(1)), NULL);
    expect_p_eq(capy_vec_reserve(NULL, data, sizeof(int), &capacity, 16), NULL);

    return true;
}

static int test_capy_vec_insert(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    size_t size = 0;
    size_t capacity = 8;
    int *data = make(arena, int, capacity);

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

    expect_p_eq(capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, MiB(1), NULL), NULL);
    expect_p_eq(capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 20, 10, NULL), NULL);

    return true;
}

static int test_capy_vec_delete(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    size_t size = 0;
    size_t capacity = 8;
    int *data = make(arena, int, capacity);

    data = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, 11, arr(int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
    expect_p_ne(data, NULL);
    expect_u_eq(size, 11);

    expect_s_eq(capy_vec_delete(data, sizeof(int), &size, 11, 1), EINVAL);

    expect_s_eq(capy_vec_delete(data, sizeof(int), &size, 11, 0), 0);
    expect_u_eq(size, 11);

    expect_s_eq(capy_vec_delete(data, sizeof(int), &size, 0, 5), 0);
    expect_u_eq(size, 6);

    expect_s_eq(capy_vec_delete(data, sizeof(int), &size, 0, 6), 0);
    expect_u_eq(size, 0);

    return true;
}

static void test_vec(testbench *t)
{
    runtest(t, test_capy_vec_reserve, "capy_vec_reserve");
    runtest(t, test_capy_vec_insert, "capy_vec_insert");
    runtest(t, test_capy_vec_delete, "capy_vec_delete");
}
