#include <capy/capy.h>

extern inline capy_smap *capy_smap_head(void *data);
extern inline size_t capy_smap_size(void *data);
extern inline size_t capy_smap_capacity(void *data);
extern inline capy_string *capy_sset_init(capy_arena *arena, size_t capacity);
extern inline bool capy_sset_has(capy_string **ptr, capy_string key);
extern inline capy_string *capy_sset_set(capy_string **ptr, capy_string key);
extern inline capy_string *capy_sset_delete(capy_string **ptr, capy_string key);

capy_smap *capy_smap_init(capy_arena *arena, size_t element_size, size_t capacity)
{
    size_t total = sizeof(capy_smap) + (element_size * capacity);
    capy_smap *smap = capy_arena_grow(arena, total, 8, true);

    smap->arena = arena;
    smap->capacity = capacity;
    smap->element_size = element_size;
    smap->data = (uint8_t *)(&smap[1]);

    return smap;
}

void *capy_smap_get(capy_smap *smap, capy_string key)
{
    capy_assert(smap != NULL);

    size_t capacity = smap->capacity;
    size_t k = capy_hash(key.data, key.size) % capacity;
    size_t j = 1;

    for (;;)
    {
        capy_string *item = (capy_string *)(smap->data + (smap->element_size * k));

        if (item[0].data == TOMBSTONE)
        {
        }
        else if (item[0].size == 0)
        {
            return NULL;
        }
        else if (capy_string_eq(key, item[0]))
        {
            return item;
        }

        k = (k + j) % capacity;
        j += 1;
    }
}

void capy_smap_set(capy_smap *smap, capy_string *pair)
{
    capy_assert(smap != NULL);

    capy_string *dest = capy_smap_get(smap, pair[0]);

    if (dest != NULL)
    {
        memcpy(dest, pair, smap->element_size);
        return;
    }

    if (smap->size >= (smap->capacity * 2) / 3)
    {
        capy_smap tmp = {
            .capacity = smap->capacity * 2,
            .element_size = smap->element_size,
            .size = 0,
        };

        tmp.data = capy_arena_grow(smap->arena, tmp.capacity * tmp.element_size, 8, true);

        for (size_t i = 0; i < smap->capacity; i++)
        {
            capy_string *item = (capy_string *)(smap->data + (i * smap->element_size));

            if (item->data != TOMBSTONE && item->size != 0)
            {
                capy_smap_set(&tmp, item);
            }
        }

        smap->capacity = tmp.capacity;
        smap->data = tmp.data;
    }

    size_t k = capy_hash(pair[0].data, pair[0].size) % smap->capacity;
    size_t j = 1;

    for (;;)
    {
        capy_string *item = (capy_string *)(smap->data + (smap->element_size * k));

        if (item[0].data == TOMBSTONE)
        {
            dest = item;
        }
        else if (item[0].size == 0)
        {
            if (dest == NULL)
            {
                dest = item;
            }

            break;
        }

        k = (k + j) % smap->capacity;
        j += 1;
    }

    memcpy(dest, pair, smap->element_size);
    smap->size += 1;
}

void capy_smap_delete(capy_smap *smap, capy_string key)
{
    capy_string *pair = capy_smap_get(smap, key);

    if (pair != NULL)
    {
        pair[0].data = TOMBSTONE;
        pair[0].size = 0;
        smap->size -= 1;
    }
}

void capy_smap_clear(capy_smap *smap)
{
    smap->size = 0;
    memset(smap->data, 0, smap->capacity * smap->element_size);
}
