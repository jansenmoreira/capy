#include <capy/hash.h>
#include <capy/macros.h>
#include <capy/strmap.h>
#include <errno.h>

static const char *DELETED = cast(const char *, -1);

void *capy_strmap_get(void *items, size_t element_size, size_t capacity, capy_string key)
{
    capy_assert(items != NULL);
    capy_assert(capacity > 0);

    char *data = items;

    size_t k = capy_hash(key.data, key.size) % capacity;
    size_t j = 1;

    for (;;)
    {
        char *item = data + (element_size * k);

        capy_string *item_key = cast(capy_string *, item);

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

        k = (k + j) % capacity;
        j += 1;
    }
}

void *capy_strmap_set(capy_arena *arena, void *items,
                      size_t element_size, size_t *capacity,
                      size_t *size, const void *kv)
{
    const char *kvdata = kv;
    char *data = items;

    capy_string *key = cast(capy_string *, kvdata);

    char *dest = capy_strmap_get(items, element_size, *capacity, *key);

    if (dest != NULL)
    {
        memcpy(dest, kvdata, element_size);
        return data;
    }

    if (*size >= (*capacity * 2) / 3)
    {
        size_t new_capacity = *capacity * 2;
        size_t new_size = 0;

        char *new_data = capy_arena_alloc(arena, element_size * new_capacity, 8, true);

        if (new_data == NULL)
        {
            return NULL;
        }

        for (size_t i = 0; i < *capacity; i++)
        {
            char *item = data + (i * element_size);

            capy_string *item_key = cast(capy_string *, item);

            if (item_key->data != DELETED && item_key->size != 0)
            {
                (void)!capy_strmap_set(arena, new_data, element_size, &new_capacity, &new_size, item);
            }
        }

        *capacity = new_capacity;
        *size = new_size;
        data = new_data;
    }

    size_t k = capy_hash(key->data, key->size) % *capacity;
    size_t j = 1;

    for (;;)
    {
        char *item = data + (element_size * k);

        capy_string *item_key = cast(capy_string *, item);

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

        k = (k + j) % *capacity;
        j += 1;
    }

    memcpy(dest, kvdata, element_size);
    *size += 1;

    return data;
}

void capy_strmap_delete(void *items, size_t element_size, size_t capacity, size_t *size, capy_string key)
{
    void *item = capy_strmap_get(items, element_size, capacity, key);

    if (item != NULL)
    {
        capy_string *key = cast(capy_string *, item);

        key->data = DELETED;
        key->size = 0;
        *size -= 1;
    }
}

// capy_strset

capy_strset *capy_strset_init(capy_arena *arena, size_t capacity)
{
    capy_strset *set = capy_arena_alloc(arena, sizeof(capy_strset) + (sizeof(capy_string) * capacity), 8, true);

    if (set != NULL)
    {
        set->size = 0;
        set->capacity = capacity;
        set->arena = arena;
        set->items = recast(capy_string *, set + 1);
    }

    return set;
}

int capy_strset_has(capy_strset *s, capy_string key)
{
    return capy_strmap_get(s->items, sizeof(capy_string), s->capacity, key) != NULL;
}

must_check capy_err capy_strset_add(capy_strset *s, capy_string key)
{
    void *data = capy_strmap_set(s->arena, s->items, sizeof(capy_string), &s->capacity, &s->size, &key);

    if (data == NULL)
    {
        return capy_errno(ENOMEM);
    }

    s->items = data;

    return ok;
}

void capy_strset_delete(capy_strset *s, capy_string key)
{
    capy_strmap_delete(s->items, sizeof(capy_string), s->capacity, &s->size, key);
}

// capy_strkvmap

capy_strkvmap *capy_strkvmap_init(capy_arena *arena, size_t capacity)
{
    capy_strkvmap *m = capy_arena_alloc(arena, sizeof(capy_strkvmap) + (sizeof(capy_strkv) * capacity), 8, true);

    if (m != NULL)
    {
        m->size = 0;
        m->capacity = capacity;
        m->arena = arena;
        m->items = recast(capy_strkv *, m + 1);
    }

    return m;
}

capy_strkv *capy_strkvmap_get(capy_strkvmap *m, capy_string key)
{
    return capy_strmap_get(m->items, sizeof(capy_strkv), m->capacity, key);
}

must_check capy_err capy_strkvmap_set(capy_strkvmap *m, capy_string key, capy_string value)
{
    capy_strkv pair = {.key = key, .value = value};

    void *items = capy_strmap_set(m->arena, m->items, sizeof(capy_strkv), &m->capacity, &m->size, &pair);

    if (items == NULL)
    {
        return capy_errno(ENOMEM);
    }

    m->items = items;

    return ok;
}

void capy_strkvmap_delete(capy_strkvmap *m, capy_string key)
{
    capy_strmap_delete(m->items, sizeof(capy_strkv), m->capacity, &m->size, key);
}

// capy_strkvmmap

capy_strkvmmap *capy_strkvmmap_init(capy_arena *arena, size_t capacity)
{
    capy_strkvmmap *mm = capy_arena_alloc(arena, sizeof(capy_strkvmmap) + (sizeof(capy_strkvn) * capacity), 8, true);

    if (mm != NULL)
    {
        mm->size = 0;
        mm->capacity = capacity;
        mm->arena = arena;
        mm->items = recast(capy_strkvn *, mm + 1);
    }

    return mm;
}

capy_strkvn *capy_strkvmmap_get(capy_strkvmmap *mm, capy_string key)
{
    return capy_strmap_get(mm->items, sizeof(capy_strkvn), mm->capacity, key);
}

must_check capy_err capy_strkvmmap_set(capy_strkvmmap *mm, capy_string key, capy_string value)
{
    capy_strkvn *item = capy_strmap_get(mm->items, sizeof(capy_strkvn), mm->capacity, key);

    if (item != NULL)
    {
        item->value = value;
        item->next = NULL;
        return ok;
    }

    capy_strkvn pair = {
        .key = key,
        .value = value,
        .next = NULL,
    };

    void *items = capy_strmap_set(mm->arena, mm->items, sizeof(capy_strkvn), &mm->capacity, &mm->size, &pair);

    if (items == NULL)
    {
        return capy_errno(ENOMEM);
    }

    mm->items = cast(capy_strkvn *, items);

    return ok;
}

must_check capy_err capy_strkvmmap_add(capy_strkvmmap *mm, capy_string key, capy_string value)
{
    capy_strkvn *item = capy_strmap_get(cast(uint8_t *, mm->items), sizeof(capy_strkvn), mm->capacity, key);

    if (item == NULL)
    {
        capy_strkvn pair = {
            .key = key,
            .value = value,
            .next = NULL,
        };

        void *items = capy_strmap_set(mm->arena, mm->items, sizeof(capy_strkvn), &mm->capacity, &mm->size, &pair);

        if (items == NULL)
        {
            return capy_errno(ENOMEM);
        }

        mm->items = cast(capy_strkvn *, items);

        return ok;
    }

    while (item->next != NULL)
    {
        item = item->next;
    }

    item->next = make(mm->arena, capy_strkvn, 1);

    if (item->next == NULL)
    {
        return capy_errno(ENOMEM);
    }

    item = item->next;

    item->key = key;
    item->value = value;

    return ok;
}

void capy_strkvmmap_delete(capy_strkvmmap *mm, capy_string key)
{
    capy_strmap_delete(mm->items, sizeof(capy_strkvn), mm->capacity, &mm->size, key);
}

void capy_strkvmmap_clear(capy_strkvmmap *mm)
{
    mm->size = 0;
    memset(mm->items, 0, mm->capacity * sizeof(capy_strkvn));
}
