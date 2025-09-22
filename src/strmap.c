#include <capy/capy.h>
#include <capy/macros.h>

static const char *DELETED = Cast(const char *, -1);

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

    capy_string *key = Cast(capy_string *, kvdata);

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

            capy_string *item_key = Cast(capy_string *, item);

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

        k = (k + j) % *capacity;
        j += 1;
    }

    memcpy(dest, kvdata, element_size);
    *size += 1;

    return data;
}

size_t capy_strmap_delete(void *items, size_t element_size, size_t capacity, size_t size, capy_string key)
{
    void *item = capy_strmap_get(items, element_size, capacity, key);

    if (item != NULL)
    {
        capy_string *key = Cast(capy_string *, item);

        key->data = DELETED;
        key->size = 0;
        size -= 1;
    }

    return size;
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
        set->items = ReinterpretCast(capy_string *, set + 1);
    }

    return set;
}

int capy_strset_has(capy_strset *s, capy_string key)
{
    return capy_strmap_get(s->items, sizeof(capy_string), s->capacity, key) != NULL;
}

MustCheck capy_err capy_strset_add(capy_strset *s, capy_string key)
{
    void *data = capy_strmap_set(s->arena, s->items, sizeof(capy_string), &s->capacity, &s->size, &key);

    if (data == NULL)
    {
        return ErrStd(ENOMEM);
    }

    s->items = data;

    return Ok;
}

void capy_strset_delete(capy_strset *s, capy_string key)
{
    s->size = capy_strmap_delete(s->items, sizeof(capy_string), s->capacity, s->size, key);
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
        m->items = ReinterpretCast(capy_strkv *, m + 1);
    }

    return m;
}

capy_strkv *capy_strkvmap_get(capy_strkvmap *m, capy_string key)
{
    return capy_strmap_get(m->items, sizeof(capy_strkv), m->capacity, key);
}

MustCheck capy_err capy_strkvmap_set(capy_strkvmap *m, capy_string key, capy_string value)
{
    capy_strkv pair = {.key = key, .value = value};

    void *items = capy_strmap_set(m->arena, m->items, sizeof(capy_strkv), &m->capacity, &m->size, &pair);

    if (items == NULL)
    {
        return ErrStd(ENOMEM);
    }

    m->items = items;

    return Ok;
}

void capy_strkvmap_delete(capy_strkvmap *m, capy_string key)
{
    m->size = capy_strmap_delete(m->items, sizeof(capy_strkv), m->capacity, m->size, key);
}

// capy_strkvnmap

capy_strkvnmap *capy_strkvnmap_init(capy_arena *arena, size_t capacity)
{
    capy_strkvnmap *mm = capy_arena_alloc(arena, sizeof(capy_strkvnmap) + (sizeof(capy_strkvn) * capacity), 8, true);

    if (mm != NULL)
    {
        mm->size = 0;
        mm->capacity = capacity;
        mm->arena = arena;
        mm->items = ReinterpretCast(capy_strkvn *, mm + 1);
    }

    return mm;
}

capy_strkvn *capy_strkvnmap_get(capy_strkvnmap *mm, capy_string key)
{
    return capy_strmap_get(mm->items, sizeof(capy_strkvn), mm->capacity, key);
}

MustCheck capy_err capy_strkvnmap_set(capy_strkvnmap *mm, capy_string key, capy_string value)
{
    capy_strkvn *item = capy_strmap_get(mm->items, sizeof(capy_strkvn), mm->capacity, key);

    if (item != NULL)
    {
        item->value = value;
        item->next = NULL;
        return Ok;
    }

    capy_strkvn pair = {
        .key = key,
        .value = value,
        .next = NULL,
    };

    void *items = capy_strmap_set(mm->arena, mm->items, sizeof(capy_strkvn), &mm->capacity, &mm->size, &pair);

    if (items == NULL)
    {
        return ErrStd(ENOMEM);
    }

    mm->items = Cast(capy_strkvn *, items);

    return Ok;
}

MustCheck capy_err capy_strkvnmap_add(capy_strkvnmap *mm, capy_string key, capy_string value)
{
    capy_strkvn *item = capy_strmap_get(Cast(uint8_t *, mm->items), sizeof(capy_strkvn), mm->capacity, key);

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
            return ErrStd(ENOMEM);
        }

        mm->items = Cast(capy_strkvn *, items);

        return Ok;
    }

    while (item->next != NULL)
    {
        item = item->next;
    }

    item->next = Make(mm->arena, capy_strkvn, 1);

    if (item->next == NULL)
    {
        return ErrStd(ENOMEM);
    }

    item = item->next;

    item->key = key;
    item->value = value;

    return Ok;
}

void capy_strkvnmap_delete(capy_strkvnmap *mm, capy_string key)
{
    mm->size = capy_strmap_delete(mm->items, sizeof(capy_strkvn), mm->capacity, mm->size, key);
}

void capy_strkvnmap_clear(capy_strkvnmap *mm)
{
    mm->size = 0;
    memset(mm->items, 0, mm->capacity * sizeof(capy_strkvn));
}
