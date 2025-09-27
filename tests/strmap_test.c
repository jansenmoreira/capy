#include <capy/test.h>

static int test_capy_strset(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    ExpectNull(capy_strset_init(arena, KiB(8)));

    capy_strset *strset = capy_strset_init(arena, 2);

    ExpectNotNull(strset);
    ExpectEqU(strset->capacity, 2);
    ExpectEqU(strset->size, 0);
    ExpectEqPtr(strset->arena, arena);
    ExpectNotNull(strset->items);

    ExpectEqS(capy_strset_has(strset, Str("foo")), false);

    ExpectOk(capy_strset_add(strset, Str("foo")));
    ExpectEqS(capy_strset_has(strset, Str("foo")), true);

    capy_strset_delete(strset, Str("foo"));

    ExpectEqS(capy_strset_has(strset, Str("foo")), false);

    ExpectOk(capy_strset_add(strset, Str("foo")));
    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_strset_add(strset, Str("bar")));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_strkvmap(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    ExpectNull(capy_strkvmap_init(arena, KiB(8)));

    capy_strkvmap *strkvmap = capy_strkvmap_init(arena, 2);

    ExpectNotNull(strkvmap);
    ExpectEqU(strkvmap->capacity, 2);
    ExpectEqU(strkvmap->size, 0);
    ExpectEqPtr(strkvmap->arena, arena);
    ExpectNotNull(strkvmap->items);

    capy_strkv *kv = capy_strkvmap_get(strkvmap, Str("foo"));
    ExpectNull(kv);

    ExpectOk(capy_strkvmap_set(strkvmap, Str("foo"), Str("bar")));

    kv = capy_strkvmap_get(strkvmap, Str("foo"));
    ExpectNotNull(kv);
    ExpectEqStr(kv->key, Str("foo"));
    ExpectEqStr(kv->value, Str("bar"));

    capy_strkvmap_delete(strkvmap, Str("foo"));
    ExpectNull(capy_strkvmap_get(strkvmap, Str("foo")));

    ExpectOk(capy_strkvmap_set(strkvmap, Str("foo"), Str("bar")));

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_strkvmap_set(strkvmap, Str("bar"), Str("foo")));

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_strkvnmap(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    ExpectNull(capy_strkvnmap_init(arena, KiB(8)));

    capy_strkvnmap *strkvnmap = capy_strkvnmap_init(arena, 2);

    ExpectNotNull(strkvnmap);
    ExpectEqU(strkvnmap->capacity, 2);
    ExpectEqU(strkvnmap->size, 0);
    ExpectEqPtr(strkvnmap->arena, arena);
    ExpectNotNull(strkvnmap->items);

    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("bar")));
    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("baz")));
    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("buz")));

    capy_strkvn *strkvn = capy_strkvnmap_get(strkvnmap, Str("foo"));
    ExpectNotNull(strkvn);
    ExpectEqStr(strkvn->key, Str("foo"));
    ExpectEqStr(strkvn->value, Str("bar"));
    ExpectNotNull(strkvn->next);
    ExpectEqStr(strkvn->next->key, Str("foo"));
    ExpectEqStr(strkvn->next->value, Str("baz"));
    ExpectNotNull(strkvn->next->next);
    ExpectEqStr(strkvn->next->next->key, Str("foo"));
    ExpectEqStr(strkvn->next->next->value, Str("buz"));
    ExpectNull(strkvn->next->next->next);

    ExpectOk(capy_strkvnmap_set(strkvnmap, Str("foo"), Str("bar")));

    strkvn = capy_strkvnmap_get(strkvnmap, Str("foo"));
    ExpectNotNull(strkvn);
    ExpectEqStr(strkvn->key, Str("foo"));
    ExpectEqStr(strkvn->value, Str("bar"));
    ExpectNull(strkvn->next);

    capy_strkvnmap_delete(strkvnmap, Str("foo"));
    strkvn = capy_strkvnmap_get(strkvnmap, Str("foo"));
    ExpectNull(strkvn);

    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("bar"), Str("foo")));
    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("bar")));
    ExpectOk(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("baz")));

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_strkvnmap_add(strkvnmap, Str("foo"), Str("fail")));
    ExpectErr(capy_strkvnmap_add(strkvnmap, Str("baz"), Str("fail")));
    ExpectErr(capy_strkvnmap_set(strkvnmap, Str("baz"), Str("fail")));

    return true;
}

static void test_strmap(testbench *t)
{
    runtest(t, test_capy_strset, "capy_strset_(init|has|add)");
    runtest(t, test_capy_strkvmap, "capy_strset_(init|get|set)");
    runtest(t, test_capy_strkvnmap, "capy_strset_(init|get|set|add)");
}
