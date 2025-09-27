#include <capy/capy.h>
#include <capy/macros.h>

static const char *DELETED = Cast(const char *, -1);

MustCheck void *capy_strmap_get(capy_strmap *map, capy_string key)
{
    size_t k = capy_hash(key.data, key.size) % map->capacity;
    size_t j = 1;

    while (j <= map->capacity)
    {
        char *item = map->items + (map->element_size * k);
        capy_string *item_key = Cast(capy_string *, item);

        if (item_key->data == DELETED)
        {
        }
        else if (item_key->size == 0)
        {
            return NULL;
        }
        else if (capy_string_eq(key, *item_key))
        {
            return item;
        }

        k = (k + j) % map->capacity;
        j += 1;
    }

    return NULL;
}

capy_err capy_strmap_set(capy_arena *arena, capy_strmap *map, const void *kv)
{
    const capy_string *key = ReinterpretCast(const capy_string *, kv);

    void *dest = capy_strmap_get(map, *key);

    if (dest != NULL)
    {
        memcpy(dest, kv, map->element_size);
        return Ok;
    }

    if (map->size >= (map->capacity * 2) / 3)
    {
        capy_strmap tmp = {
            .capacity = map->capacity * 2,
            .element_size = map->element_size,
        };

        tmp.items = capy_arena_alloc(arena, tmp.element_size * tmp.capacity, 8, true);

        if (tmp.items == NULL)
        {
            return ErrStd(ENOMEM);
        }

        for (size_t i = 0; i < map->capacity; i++)
        {
            char *item = map->items + (i * map->element_size);
            capy_string *item_key = Cast(capy_string *, item);

            if (item_key->size != 0)
            {
                Ignore capy_strmap_set(arena, &tmp, item).code;
            }
        }

        map->capacity = tmp.capacity;
        map->size = tmp.size;
        map->items = tmp.items;
    }

    size_t k = capy_hash(key->data, key->size) % map->capacity;
    size_t j = 1;

    char *data = map->items;

    while (j <= map->capacity)
    {
        char *item = data + (map->element_size * k);
        capy_string *item_key = Cast(capy_string *, item);

        if (item_key->data == DELETED)
        {
            dest = item;
        }
        else if (item_key->size == 0)
        {
            if (dest == NULL)
            {
                dest = item;
            }

            break;
        }

        k = (k + j) % map->capacity;
        j += 1;
    }

    memcpy(dest, kv, map->element_size);
    map->size += 1;

    return Ok;
}

void capy_strmap_delete(capy_strmap *map, capy_string key)
{
    char *item = capy_strmap_get(map, key);

    if (item != NULL)
    {
        capy_string *key = Cast(capy_string *, item);

        key->data = DELETED;
        key->size = 0;
        map->size -= 1;
    }
}

// capy_strset

capy_strset *capy_strset_init(capy_arena *arena, size_t capacity)
{
    char *addr = capy_arena_alloc(arena, sizeof(capy_strset) + (sizeof(capy_string) * capacity), 8, true);

    if (addr == NULL)
    {
        return NULL;
    }

    capy_strset *set = Cast(capy_strset *, addr);

    set->size = 0;
    set->capacity = capacity;
    set->element_size = sizeof(capy_string);
    set->arena = arena;
    set->items = Cast(capy_string *, addr + sizeof(capy_strset));

    return set;
}

int capy_strset_has(capy_strset *s, capy_string key)
{
    return capy_strmap_get(&s->strmap, key) != NULL;
}

capy_err capy_strset_add(capy_strset *s, capy_string key)
{
    return capy_strmap_set(s->arena, &s->strmap, &key);
}

void capy_strset_delete(capy_strset *s, capy_string key)
{
    capy_strmap_delete(&s->strmap, key);
}

// capy_strkvmap

capy_strkvmap *capy_strkvmap_init(capy_arena *arena, size_t capacity)
{
    char *addr = capy_arena_alloc(arena, sizeof(capy_strkvmap) + (sizeof(capy_strkv) * capacity), 8, true);

    if (addr == NULL)
    {
        return NULL;
    }

    capy_strkvmap *m = Cast(capy_strkvmap *, addr);

    m->size = 0;
    m->capacity = capacity;
    m->element_size = sizeof(capy_strkv);
    m->arena = arena;
    m->items = Cast(capy_strkv *, addr + sizeof(capy_strkvmap));

    return m;
}

capy_strkv *capy_strkvmap_get(capy_strkvmap *m, capy_string key)
{
    return capy_strmap_get(&m->strmap, key);
}

capy_err capy_strkvmap_set(capy_strkvmap *m, capy_string key, capy_string value)
{
    capy_strkv pair = {.key = key, .value = value};
    return capy_strmap_set(m->arena, &m->strmap, &pair);
}

void capy_strkvmap_delete(capy_strkvmap *m, capy_string key)
{
    capy_strmap_delete(&m->strmap, key);
}

// capy_strkvnmap

capy_strkvnmap *capy_strkvnmap_init(capy_arena *arena, size_t capacity)
{
    char *addr = capy_arena_alloc(arena, sizeof(capy_strkvnmap) + (sizeof(capy_strkvn) * capacity), 8, true);

    if (addr == NULL)
    {
        return NULL;
    }

    capy_strkvnmap *mm = Cast(capy_strkvnmap *, addr);

    mm->size = 0;
    mm->capacity = capacity;
    mm->element_size = sizeof(capy_strkvn);
    mm->arena = arena;
    mm->items = Cast(capy_strkvn *, addr + sizeof(capy_strkvnmap));

    return mm;
}

capy_strkvn *capy_strkvnmap_get(capy_strkvnmap *mm, capy_string key)
{
    return capy_strmap_get(&mm->strmap, key);
}

capy_err capy_strkvnmap_set(capy_strkvnmap *mm, capy_string key, capy_string value)
{
    capy_strkvn pair = {.key = key, .value = value, .next = NULL};
    return capy_strmap_set(mm->arena, &mm->strmap, &pair);
}

capy_err capy_strkvnmap_add(capy_strkvnmap *mm, capy_string key, capy_string value)
{
    capy_strkvn *item = capy_strmap_get(&mm->strmap, key);

    if (item == NULL)
    {
        return capy_strkvnmap_set(mm, key, value);
    }

    while (item->next != NULL)
    {
        item = item->next;
    }

    capy_strkvn *next = Make(mm->arena, capy_strkvn, 1);

    if (next == NULL)
    {
        return ErrStd(ENOMEM);
    }

    next->key = key;
    next->value = value;

    item->next = next;

    return Ok;
}

void capy_strkvnmap_delete(capy_strkvnmap *mm, capy_string key)
{
    capy_strmap_delete(&mm->strmap, key);
}

void capy_strkvnmap_clear(capy_strkvnmap *mm)
{
    mm->size = 0;
    memset(mm->items, 0, mm->capacity * sizeof(capy_strkvn));
}

capy_strkvn *capy_strkvnmap_at(capy_strkvnmap *mm, size_t index)
{
    capy_strkvn *item = mm->items + index;

    if (item->key.size == 0)
    {
        return NULL;
    }

    return item;
}
