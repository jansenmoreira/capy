#include <capy/test.h>

static int test_capy_strset(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    expect_p_eq(capy_strset_init(arena, KiB(8)), NULL);

    capy_strset *strset = capy_strset_init(arena, 2);

    expect_p_ne(strset, NULL);
    expect_u_eq(strset->capacity, 2);
    expect_u_eq(strset->size, 0);
    expect_p_eq(strset->arena, arena);
    expect_p_ne(strset->items, NULL);

    expect_s_eq(capy_strset_has(strset, strl("foo")), false);

    expect_ok(capy_strset_add(strset, strl("foo")));
    expect_s_eq(capy_strset_has(strset, strl("foo")), true);

    capy_strset_delete(strset, strl("foo"));

    expect_s_eq(capy_strset_has(strset, strl("foo")), false);

    expect_ok(capy_strset_add(strset, strl("foo")));
    expect_p_ne(make(arena, char, KiB(4) - capy_arena_size(arena)), NULL);
    expect_err(capy_strset_add(strset, strl("bar")));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_strkvmap(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    expect_p_eq(capy_strkvmap_init(arena, KiB(8)), NULL);

    capy_strkvmap *strkvmap = capy_strkvmap_init(arena, 2);

    expect_p_ne(strkvmap, NULL);
    expect_u_eq(strkvmap->capacity, 2);
    expect_u_eq(strkvmap->size, 0);
    expect_p_eq(strkvmap->arena, arena);
    expect_p_ne(strkvmap->items, NULL);

    capy_strkv *kv = capy_strkvmap_get(strkvmap, strl("foo"));
    expect_p_eq(kv, NULL);

    expect_ok(capy_strkvmap_set(strkvmap, strl("foo"), strl("bar")));

    kv = capy_strkvmap_get(strkvmap, strl("foo"));
    expect_p_ne(kv, NULL);
    expect_str_eq(kv->key, strl("foo"));
    expect_str_eq(kv->value, strl("bar"));

    capy_strkvmap_delete(strkvmap, strl("foo"));
    expect_p_eq(capy_strkvmap_get(strkvmap, strl("foo")), NULL);

    expect_ok(capy_strkvmap_set(strkvmap, strl("foo"), strl("bar")));

    expect_p_ne(make(arena, char, KiB(4) - capy_arena_size(arena)), NULL);
    expect_err(capy_strkvmap_set(strkvmap, strl("bar"), strl("foo")));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_strkvmmap(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    expect_p_eq(capy_strkvmmap_init(arena, KiB(8)), NULL);

    capy_strkvmmap *strkvmmap = capy_strkvmmap_init(arena, 4);

    expect_p_ne(strkvmmap, NULL);
    expect_u_eq(strkvmmap->capacity, 4);
    expect_u_eq(strkvmmap->size, 0);
    expect_p_eq(strkvmmap->arena, arena);
    expect_p_ne(strkvmmap->items, NULL);

    expect_ok(capy_strkvmmap_add(strkvmmap, strl("foo"), strl("bar")));
    expect_ok(capy_strkvmmap_add(strkvmmap, strl("foo"), strl("baz")));
    expect_ok(capy_strkvmmap_add(strkvmmap, strl("foo"), strl("buz")));

    capy_strkvn *strkvn = capy_strkvmmap_get(strkvmmap, strl("foo"));
    expect_p_ne(strkvn, NULL);
    expect_str_eq(strkvn->key, strl("foo"));
    expect_str_eq(strkvn->value, strl("bar"));
    expect_p_ne(strkvn->next, NULL);
    expect_str_eq(strkvn->next->key, strl("foo"));
    expect_str_eq(strkvn->next->value, strl("baz"));
    expect_p_ne(strkvn->next->next, NULL);
    expect_str_eq(strkvn->next->next->key, strl("foo"));
    expect_str_eq(strkvn->next->next->value, strl("buz"));
    expect_p_eq(strkvn->next->next->next, NULL);

    expect_ok(capy_strkvmmap_set(strkvmmap, strl("foo"), strl("bar")));

    strkvn = capy_strkvmmap_get(strkvmmap, strl("foo"));
    expect_p_ne(strkvn, NULL);
    expect_str_eq(strkvn->key, strl("foo"));
    expect_str_eq(strkvn->value, strl("bar"));
    expect_p_eq(strkvn->next, NULL);

    capy_strkvmmap_delete(strkvmmap, strl("foo"));
    strkvn = capy_strkvmmap_get(strkvmmap, strl("foo"));
    expect_p_eq(strkvn, NULL);

    expect_ok(capy_strkvmmap_add(strkvmmap, strl("bar"), strl("foo")));
    expect_ok(capy_strkvmmap_add(strkvmmap, strl("foo"), strl("bar")));
    expect_ok(capy_strkvmmap_add(strkvmmap, strl("foo"), strl("baz")));

    expect_p_ne(make(arena, char, KiB(4) - capy_arena_size(arena)), NULL);
    expect_err(capy_strkvmmap_add(strkvmmap, strl("foo"), strl("fail")));
    expect_err(capy_strkvmmap_add(strkvmmap, strl("baz"), strl("fail")));
    expect_err(capy_strkvmmap_set(strkvmmap, strl("baz"), strl("fail")));

    return true;
}

static void test_strmap(testbench *t)
{
    runtest(t, test_capy_strset, "capy_strset_(init|has|add)");
    runtest(t, test_capy_strkvmap, "capy_strset_(init|get|set)");
    runtest(t, test_capy_strkvmmap, "capy_strset_(init|get|set|add)");
}
