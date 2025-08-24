#include "test.h"

struct smap_pair_point tests[] = {
    {.key = "a", .value.x = 1.0f},
    {.key = "b", .value.x = 2.0f},
    {.key = "c", .value.x = 3.0f},
    {.key = "d", .value.x = 4.0f},
    {.key = "e", .value.x = 5.0f},
    {.key = "f", .value.x = 6.0f},
    {.key = "g", .value.x = 7.0f},
    {.key = "h", .value.x = 8.0f},
    {.key = "i", .value.x = 9.0f},
    {.key = "j", .value.x = 10.0f},
    {.key = "k", .value.x = 11.0f},
    {.key = "l", .value.x = 12.0f},
    {.key = "m", .value.x = 13.0f},
    {.key = "n", .value.x = 14.0f},
    {.key = "o", .value.x = 15.0f},
    {.key = "p", .value.x = 16.0f},
    {.key = "q", .value.x = 17.0f},
    {.key = "r", .value.x = 18.0f},
    {.key = "s", .value.x = 19.0f},
    {.key = "t", .value.x = 20.0f},
    {.key = "u", .value.x = 21.0f},
    {.key = "v", .value.x = 22.0f},
    {.key = "w", .value.x = 23.0f},
    {.key = "x", .value.x = 24.0f},
    {.key = "y", .value.x = 25.0f},
    {.key = "z", .value.x = 26.0f},
};

int test_smap(void)
{
    //     struct smap_pair_point pair;

    //     struct smap_pair_point *smap1 = NULL;

    //     for (int i = 0; i < 11; i++)
    //     {
    //         capy_smap_set_point(&smap1, tests[i]);
    //     }

    //     for (int i = 0; i < 11; i++)
    //     {
    //         pair = capy_smap_get_point(&smap1, tests[i].key);
    //         capy_expect(strcmp(pair.key, tests[i].key) == 0);
    //         capy_expect(pair.value.x == tests[i].value.x);
    //     }

    //     pair = capy_smap_get_point(&smap1, tests[11].key);
    //     capy_expect(pair.key == NULL);

    //     capy_smap_delete_point(&smap1, tests[10].key);
    //     capy_smap_delete_point(&smap1, "0");

    //     for (int i = 11; i < 26; i++)
    //     {
    //         capy_smap_set_point(&smap1, tests[i]);
    //     }

    //     pair = capy_smap_get_point(&smap1, tests[10].key);
    //     capy_expect(pair.key == NULL);

    //     return 0;
    // }

    // int test_sset(void)
    // {
    //     const char **sset1 = NULL;

    //     for (int i = 0; i < 11; i++)
    //     {
    //         capy_sset_set(&sset1, tests[i].key);
    //     }

    //     for (int i = 0; i < 11; i++)
    //     {
    //         const char *value = capy_sset_get(&sset1, tests[i].key);
    //         capy_expect(strcmp(value, tests[i].key) == 0);
    //     }

    //     capy_expect(capy_sset_get(&sset1, tests[11].key) == NULL);

    //     capy_sset_delete(&sset1, tests[10].key);
    //     capy_sset_delete(&sset1, "0");

    //     for (int i = 11; i < 26; i++)
    //     {
    //         capy_sset_set(&sset1, tests[i].key);
    //     }

    //     capy_expect(capy_sset_get(&sset1, tests[10].key) == NULL);

    //     return 0;
    // }

    // int test_symbols(void)
    // {
    //     char txt[100] = {'a', 0, 'b', 0, 'a', 0};

    //     const char *a1 = "a";
    //     const char *a2 = &txt[0];
    //     const char *a3 = &txt[4];
    //     const char *b1 = "b";
    //     const char *b2 = &txt[2];

    //     const char **symbols = NULL;

    //     const char *str_a1 = capy_symbols_add(&symbols, a1);
    //     const char *str_a2 = capy_symbols_add(&symbols, a2);
    //     const char *str_a3 = capy_symbols_add(&symbols, a3);
    //     const char *str_a4 = capy_symbols_add(&symbols, "a");

    //     capy_expect(str_a1 == str_a2);
    //     capy_expect(str_a1 == str_a3);
    //     capy_expect(str_a1 == str_a4);

    //     const char *str_b1 = capy_symbols_add(&symbols, b1);
    //     const char *str_b2 = capy_symbols_add(&symbols, b2);
    //     const char *str_b3 = capy_symbols_add(&symbols, "b");

    //     capy_expect(str_b1 == str_b2);
    //     capy_expect(str_b1 == str_b3);

    //     capy_symbols_add(&symbols, "test1");
    //     capy_symbols_add(&symbols, "example1");

    //     capy_symbols_free(&symbols);

    return 0;
}
