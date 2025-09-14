#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>

// DEFINITIONS

void *capy_vec_reserve(capy_arena *arena, void *items, size_t element_size, size_t *capacity, size_t size)
{
    capy_assert(items != NULL);
    capy_assert(capacity != NULL);
    capy_assert(*capacity > 0);

    char *data = cast(char *, items);

    if (size > *capacity)
    {
        if (arena == NULL)
        {
            return NULL;
        }

        size = next_pow2(size);

        data = capy_arena_realloc(arena, data, element_size * (*capacity), element_size * size, false);

        if (data != NULL)
        {
            *capacity = size;
        }
    }

    return data;
}

void *capy_vec_insert(capy_arena *arena, void *restrict items,
                      size_t element_size, size_t *restrict capacity, size_t *restrict size,
                      size_t position, size_t count, const void *restrict values)
{
    capy_assert(items != NULL);
    capy_assert(size != NULL);

    if (position > *size)
    {
        return NULL;
    }

    size_t old_size = (*size);
    size_t new_size = old_size + count;
    size_t tail_size = old_size - position;

    items = capy_vec_reserve(arena, items, element_size, capacity, new_size);

    if (items != NULL)
    {
        char *data = cast(char *, items);

        if (tail_size > 0)
        {
            memmove(data + (element_size * (position + count)),
                    data + (element_size * (position)),
                    element_size * tail_size);
        }

        if (values != NULL)
        {
            memcpy(data + (element_size * position),
                   values,
                   element_size * count);
        }

        *size = new_size;
    }

    return items;
}

int capy_vec_delete(void *items, size_t element_size, size_t *size, size_t position, size_t count)
{
    capy_assert(items != NULL);
    capy_assert(size != NULL);

    if (position + count > *size)
    {
        return EINVAL;
    }

    size_t old_size = *size;
    size_t new_size = old_size - count;
    size_t tail_size = old_size - (position + count);

    char *data = cast(char *, items);

    if (tail_size > 0)
    {
        memmove(data + (element_size * (position)),
                data + (element_size * (position + count)),
                element_size * tail_size);
    }

    *size = new_size;

    return 0;
}
