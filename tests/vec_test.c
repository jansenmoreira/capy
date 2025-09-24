#include <capy/test.h>

static int test_capy_vec_insert(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_vec vec = {
        .size = 0,
        .capacity = 8,
        .element_size = sizeof(int),
        .items = NULL,
    };

    vec.items = Make(arena, int, vec.capacity);

    ExpectOk(capy_vec_insert(arena, &vec, 0, 5, Arr(int, 6, 7, 8, 9, 10)));
    ExpectEqU(vec.size, 5);

    ExpectOk(capy_vec_insert(arena, &vec, 0, 5, Arr(int, 1, 2, 3, 4, 5)));
    ExpectEqU(vec.size, 10);

    ExpectOk(capy_vec_insert(arena, &vec, 0, 1, NULL));
    ExpectEqU(vec.size, 11);

    int *data = Cast(int *, vec.items);
    data[0] = 0;

    for (int i = 0; i < 11; i++)
    {
        ExpectEqS(data[i], i);
    }

    ExpectErr(capy_vec_insert(arena, &vec, 0, MiB(1), NULL));

    return true;
}

static int test_capy_vec_delete(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_vec vec = {
        .size = 0,
        .capacity = 8,
        .element_size = sizeof(int),
        .items = NULL,
    };

    vec.items = Make(arena, int, vec.capacity);

    ExpectOk(capy_vec_insert(arena, &vec, 0, 11, Arr(int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)));
    ExpectEqU(vec.size, 11);

    ExpectOk(capy_vec_delete(&vec, 11, 0));
    ExpectEqU(vec.size, 11);

    ExpectOk(capy_vec_delete(&vec, 0, 5));
    ExpectEqU(vec.size, 6);

    ExpectOk(capy_vec_delete(&vec, 0, 6));
    ExpectEqU(vec.size, 0);

    return true;
}

static void test_vec(testbench *t)
{
    runtest(t, test_capy_vec_insert, "capy_vec_insert");
    runtest(t, test_capy_vec_delete, "capy_vec_delete");
}
