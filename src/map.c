#include <capy/capy.h>
#include <errno.h>

void *capy_smap_get(void **ptr, ptrdiff_t pair_size, const char *key)
{
    if (*ptr == NULL)
    {
        return NULL;
    }

    ptrdiff_t capacity = capy_smap_capacity(*ptr);
    ptrdiff_t k = capy_hash(key, strlen(key)) % capacity;
    ptrdiff_t j = 1;

    for (;;)
    {
        const char **item = (const char **)(*ptr) + (pair_size * k);

        if (item[0] == NULL)
        {
            return NULL;
        }

        if (item[0] != TOMBSTONE && strcmp(key, item[0]) == 0)
        {
            return item;
        }

        k = (k + j) % capacity;
        j += 1;
    }
}

int capy_smap_set(void **ptr, ptrdiff_t pair_size, const char *key, void *value)
{
    struct capy_smap *smap = capy_smap_head(*ptr);

    if (smap == NULL || smap->size >= (smap->capacity * 2) / 3)
    {
        ptrdiff_t buffer_capacity = 8;

        if (smap != NULL && smap->capacity != 0)
        {
            buffer_capacity = smap->capacity * 2;
        }

        uint8_t *buffer = malloc(sizeof(struct capy_smap) + buffer_capacity * pair_size);

        if (buffer == NULL)
        {
            return ENOMEM;
        }

        memset(buffer, 0, buffer_capacity * pair_size);

        for (ptrdiff_t i = 0; smap != NULL && i < smap->capacity; i++)
        {
            const char **src = (const char **)(*ptr) + (pair_size * i);

            if (src[0] == NULL || src[0] == TOMBSTONE)
            {
                continue;
            }

            ptrdiff_t k = capy_hash(src[0], strlen(src[0])) % buffer_capacity;
            ptrdiff_t j = 1;

            for (;;)
            {
                size_t index = sizeof(struct capy_smap) + (pair_size * k);
                const char **dst = (const char **)(buffer + index);

                if (dst[0] == NULL)
                {
                    memcpy(dst, src, pair_size);
                    break;
                }

                k = (k + j) % buffer_capacity;
                j += 1;
            }
        }

        free(smap);
        smap = (struct capy_smap *)(buffer);
        smap->capacity = buffer_capacity;
        *ptr = &smap[1];
    }

    ptrdiff_t k = capy_hash(key, strlen(key)) % smap->capacity;
    ptrdiff_t j = 1;

    for (;;)
    {
        size_t index = sizeof(struct capy_smap) + (pair_size * k);
        const char **dst = (const char **)((uint8_t *)(*ptr) + index);

        if (dst[0] == NULL || dst[0] == TOMBSTONE || strcmp(*dst, key) == 0)
        {
            memcpy(dst, value, pair_size);
            smap->size += 1;
            return 0;
        }

        k = (k + j) % smap->capacity;
        j += 1;
    }
}

void capy_smap_delete(void **ptr, ptrdiff_t pair_size, const char *key)
{
    const char **pair = capy_smap_get(ptr, pair_size, key);

    if (pair != NULL)
    {
        pair[0] = TOMBSTONE;
        capy_smap_head(*ptr)->size -= 1;
    }
}

void *capy_smap_next(void *data, ptrdiff_t pair_size, void *it)
{
    uint8_t *end = (uint8_t *)(data) + (pair_size * capy_smap_capacity(data));
    uint8_t *cursor = it;

    cursor = (cursor == NULL) ? data : cursor + pair_size;

    while (cursor < end)
    {
        const char **item = (const char **)(cursor);

        if (item[0] != NULL && item[0] != TOMBSTONE)
        {
            return cursor;
        }

        cursor += pair_size;
    }

    return NULL;
}
