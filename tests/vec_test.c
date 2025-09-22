#include <capy/test.h>

static int test_capy_vec_insert(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    size_t size = 0;
    size_t capacity = 8;
    int *data = Make(arena, int, capacity);

    ExpectNotNull(data = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, 5, Arr(int, 6, 7, 8, 9, 10)));
    ExpectEqU(size, 5);

    ExpectNotNull(data = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, 5, Arr(int, 1, 2, 3, 4, 5)));
    ExpectEqU(size, 10);

    ExpectNotNull(data = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, 1, NULL));
    ExpectEqU(size, 11);

    data[0] = 0;

    for (int i = 0; i < 11; i++)
    {
        ExpectEqS(data[i], i);
    }

    ExpectNull(capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, MiB(1), NULL));

    return true;
}

static int test_capy_vec_delete(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    size_t size = 0;
    size_t capacity = 8;
    int *data = Make(arena, int, capacity);

    ExpectNotNull(data = capy_vec_insert(arena, data, sizeof(int), &capacity, &size, 0, 11, Arr(int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)));
    ExpectEqU(size, 11);

    size = capy_vec_delete(data, sizeof(int), size, 11, 0);
    ExpectEqU(size, 11);

    size = capy_vec_delete(data, sizeof(int), size, 0, 5);
    ExpectEqU(size, 6);

    size = capy_vec_delete(data, sizeof(int), size, 0, 6);
    ExpectEqU(size, 0);

    return true;
}

static void test_vec(testbench *t)
{
    runtest(t, test_capy_vec_insert, "capy_vec_insert");
    runtest(t, test_capy_vec_delete, "capy_vec_delete");
}
