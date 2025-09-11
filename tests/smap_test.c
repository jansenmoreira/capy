#include <capy/test.h>

// static int test_smap(void)
// {
//     typedef struct string_pair
//     {
//         capy_string key;
//         capy_string value;
//     } string_pair;

//     string_pair fields[] = {
//         {.key = str("a"), .value = str("A")},
//         {.key = str("b"), .value = str("B")},
//         {.key = str("c"), .value = str("C")},
//         {.key = str("d"), .value = str("D")},
//         {.key = str("e"), .value = str("E")},
//         {.key = str("f"), .value = str("F")},
//         {.key = str("g"), .value = str("G")},
//         {.key = str("h"), .value = str("H")},
//         {.key = str("i"), .value = str("I")},
//         {.key = str("j"), .value = str("J")},
//         {.key = str("k"), .value = str("K")},
//         {.key = str("l"), .value = str("L")},
//         {.key = str("m"), .value = str("M")},
//         {.key = str("n"), .value = str("N")},
//         {.key = str("o"), .value = str("O")},
//         {.key = str("p"), .value = str("P")},
//         {.key = str("q"), .value = str("Q")},
//         {.key = str("r"), .value = str("R")},
//         {.key = str("s"), .value = str("S")},
//         {.key = str("t"), .value = str("T")},
//         {.key = str("u"), .value = str("U")},
//         {.key = str("v"), .value = str("V")},
//         {.key = str("w"), .value = str("W")},
//         {.key = str("x"), .value = str("X")},
//         {.key = str("y"), .value = str("Y")},
//         {.key = str("z"), .value = str("Z")},
//     };

//     capy_arena *arena;
//     capy_smap *smap;

//     arena = capy_arena_init(KiB(4));
//     expect_p_ne(capy_arena_make(arena, char, 4000), NULL);
//     expect_p_eq(capy_smap_of(capy_string, arena, 512), NULL);
//     smap = capy_smap_of(capy_string, arena, 0);
//     expect_p_ne(smap, NULL);
//     expect_s_eq(capy_smap_set(smap, &fields[0].key), ENOMEM);
//     capy_arena_destroy(arena);

//     arena = capy_arena_init(KiB(8));

//     smap = capy_smap_of(string_pair, arena, 32);

//     for (size_t i = 0; i < arrlen(fields); i++)
//     {
//         expect_s_eq(capy_smap_set(smap, &fields[i].key), 0);
//     }

//     string_pair *pair = NULL;

//     for (size_t i = 0; i < arrlen(fields); i++)
//     {
//         pair = capy_smap_get(smap, fields[i].key);
//         expect_p_ne(pair, NULL);
//         expect_str_eq(pair->key, fields[i].key);
//         expect_str_eq(pair->value, fields[i].value);
//     }

//     capy_smap_delete(smap, str("d"));
//     expect_p_ne(smap, NULL);

//     pair = capy_smap_get(smap, str("d"));
//     expect_p_eq(pair, NULL);

//     expect_s_eq(capy_smap_set(smap, &(string_pair){str("d"), str("d")}.key), 0);
//     expect_p_ne(smap, NULL);

//     expect_s_eq(capy_smap_set(smap, &(string_pair){str("q"), str("q")}.key), 0);
//     expect_p_ne(smap, NULL);

//     pair = capy_smap_get(smap, str("d"));
//     expect_p_ne(pair, NULL);
//     expect_str_eq(pair->key, str("d"));
//     expect_str_eq(pair->value, str("d"));

//     pair = capy_smap_get(smap, str("q"));
//     expect_p_ne(pair, NULL);
//     expect_str_eq(pair->key, str("q"));
//     expect_str_eq(pair->value, str("q"));

//     capy_smap_delete(smap, str("0"));
//     expect_p_ne(smap, NULL);

//     capy_smap_delete(smap, str("z"));
//     expect_p_ne(smap, NULL);

//     char buffer[32];

//     for (size_t i = 0; i < 20; i++)
//     {
//         size_t written = (size_t)(snprintf(buffer, 31, "%d", (int)(i)));
//         capy_string number = capy_string_bytes(written, buffer);

//         expect_s_eq(capy_string_copy(arena, &number, number), 0);

//         expect_s_eq(capy_smap_set(smap, &(string_pair){number, number}.key), 0);
//         expect_p_ne(smap, NULL);
//     }

//     for (size_t i = 0; i < 20; i++)
//     {
//         size_t written = (size_t)(snprintf(buffer, 31, "%d", (int)(i)));
//         capy_string number = capy_string_bytes(written, buffer);

//         pair = capy_smap_get(smap, number);
//         expect_p_ne(pair, NULL);
//         expect_str_eq(pair->key, number);
//         expect_str_eq(pair->value, number);
//     }

//     expect_u_eq(smap->size, 45);

//     return 0;
// }
