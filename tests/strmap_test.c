#include <capy/test.h>

static int test_strmap(void)
{
    capy_arena *arena = capy_arena_init(KiB(2));

    expect_p_eq(capy_strset_init(arena, KiB(8)), NULL);

    capy_strset *strset = capy_strset_init(arena, 4);

    expect_p_ne(strset, NULL);
    expect_u_eq(strset->capacity, 4);
    expect_u_eq(strset->size, 0);
    expect_p_eq(strset->arena, arena);
    expect_p_ne(strset->items, NULL);

    expect_s_eq(capy_strset_has(strset, str("foo")), false);
    expect_s_eq(capy_strset_add(strset, str("foo")), 0);
    expect_s_eq(capy_strset_add(strset, str("buz")), 0);
    expect_s_eq(capy_strset_has(strset, str("foo")), true);
    expect_s_eq(capy_strset_has(strset, str("buz")), true);

    capy_strset_delete(strset, str("foo"));
    capy_strset_delete(strset, str("bar"));
    capy_strset_delete(strset, str("buz"));

    expect_s_eq(capy_strset_has(strset, str("foo")), false);
    expect_s_eq(capy_strset_has(strset, str("buz")), false);

    expect_s_eq(capy_strset_add(strset, str("foo")), 0);
    expect_s_eq(capy_strset_add(strset, str("boz")), 0);
    expect_s_eq(capy_strset_add(strset, str("bar")), 0);

    expect_s_eq(capy_strset_has(strset, str("foo")), true);
    expect_s_eq(capy_strset_has(strset, str("boz")), true);
    expect_s_eq(capy_strset_has(strset, str("bar")), true);

    capy_strkv *pairs = capy_arena_make(arena, capy_strkv, 64);

    for (size_t i = 0; i < 64; i++)
    {
        char *buffer = capy_arena_make(arena, char, 4);
        snprintf(buffer, 4, "%lu", i);
        pairs[i].key = capy_string_cstr(buffer);
        pairs[i].value = pairs[i].key;
    }

    expect_p_eq(capy_strkvmap_init(arena, 64), NULL);

    capy_strkvmap *strkvmap = capy_strkvmap_init(arena, 32);

    expect_p_ne(strkvmap, NULL);
    expect_u_eq(strkvmap->capacity, 32);
    expect_u_eq(strkvmap->size, 0);
    expect_p_eq(strkvmap->arena, arena);
    expect_p_ne(strkvmap->items, NULL);

    capy_strkv *strkv;

    for (size_t i = 0; i < 21; i++)
    {
        expect_s_eq(capy_strkvmap_set(strkvmap, pairs[i].key, pairs[i].value), 0);
    }

    for (size_t i = 0; i < 21; i++)
    {
        capy_strkv *pair = capy_strkvmap_get(strkvmap, pairs[i].key);
        expect_p_ne(pair, NULL);
        expect_str_eq(pair->key, pairs[i].key);
        expect_str_eq(pair->value, pairs[i].value);
    }

    expect_s_eq(capy_strkvmap_set(strkvmap, pairs[21].key, pairs[21].value), ENOMEM);

    expect_s_eq(capy_strkvmap_set(strkvmap, pairs[0].key, str("")), 0);
    strkv = capy_strkvmap_get(strkvmap, pairs[0].key);
    expect_p_ne(strkv, NULL);
    expect_str_eq(strkv->key, pairs[0].key);
    expect_str_eq(strkv->value, str(""));

    capy_strkvmap_delete(strkvmap, pairs[0].key);
    strkv = capy_strkvmap_get(strkvmap, pairs[0].key);
    expect_p_eq(strkv, NULL);

    expect_p_eq(capy_strkvmmap_init(arena, KiB(8)), NULL);

    capy_strkvmmap *strkvmmap = capy_strkvmmap_init(arena, 8);

    expect_p_ne(strkvmmap, NULL);
    expect_u_eq(strkvmmap->capacity, 8);
    expect_u_eq(strkvmmap->size, 0);
    expect_p_eq(strkvmmap->arena, arena);
    expect_p_ne(strkvmmap->items, NULL);

    capy_strkvn *strkvn;

    expect_s_eq(capy_strkvmmap_add(strkvmmap, str("foo"), str("bar")), 0);
    expect_s_eq(capy_strkvmmap_add(strkvmmap, str("foo"), str("baz")), 0);
    expect_s_eq(capy_strkvmmap_add(strkvmmap, str("foo"), str("buz")), 0);

    strkvn = capy_strkvmmap_get(strkvmmap, str("foo"));
    expect_p_ne(strkvn, NULL);
    expect_str_eq(strkvn->key, str("foo"));
    expect_str_eq(strkvn->value, str("bar"));
    expect_p_ne(strkvn->next, NULL);
    expect_str_eq(strkvn->next->key, str("foo"));
    expect_str_eq(strkvn->next->value, str("baz"));
    expect_p_ne(strkvn->next->next, NULL);
    expect_str_eq(strkvn->next->next->key, str("foo"));
    expect_str_eq(strkvn->next->next->value, str("buz"));
    expect_p_eq(strkvn->next->next->next, NULL);

    expect_s_eq(capy_strkvmmap_set(strkvmmap, str("bar"), str("foo")), 0);
    expect_s_eq(capy_strkvmmap_set(strkvmmap, str("foo"), str("bar")), 0);

    strkvn = capy_strkvmmap_get(strkvmmap, str("foo"));
    expect_p_ne(strkvn, NULL);
    expect_str_eq(strkvn->key, str("foo"));
    expect_str_eq(strkvn->value, str("bar"));
    expect_p_eq(strkvn->next, NULL);

    capy_strkvmmap_delete(strkvmmap, str("foo"));
    strkvn = capy_strkvmmap_get(strkvmmap, str("foo"));
    expect_p_eq(strkvn, NULL);

    strkvn = capy_strkvmmap_get(strkvmmap, str("bar"));
    expect_p_ne(strkvn, NULL);
    expect_str_eq(strkvn->key, str("bar"));
    expect_str_eq(strkvn->value, str("foo"));
    expect_p_eq(strkvn->next, NULL);

    expect_s_eq(capy_strkvmmap_add(strkvmmap, str("foo"), str("1")), 0);
    expect_s_eq(capy_strkvmmap_add(strkvmmap, str("foo"), str("2")), 0);
    expect_s_eq(capy_strkvmmap_add(strkvmmap, str("foo"), str("fail")), ENOMEM);

    return 0;
}
