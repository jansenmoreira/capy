#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>

// DEFINITIONS

void *capy_vec_reserve(capy_arena *arena, void *data, size_t element_size, size_t *capacity, size_t size)
{
    capy_assert(data != NULL);
    capy_assert(capacity != NULL);
    capy_assert(*capacity > 0);

    if (size > *capacity)
    {
        if (arena == NULL)
        {
            return NULL;
        }

        size = next_pow2(size);

        data = capy_arena_realloc(arena, data, (*capacity) * element_size, size * element_size, false);

        if (data == NULL)
        {
            return NULL;
        }

        *capacity = size;
    }

    return data;
}

void *capy_vec_insert(capy_arena *arena, void *src,
                      size_t element_size, size_t *capacity, size_t *size,
                      size_t position, size_t count, const void *values)
{
    capy_assert(src != NULL);
    capy_assert(size != NULL);
    capy_assert(position <= *size);

    size_t old_size = (*size);
    size_t new_size = old_size + count;
    size_t tail_size = old_size - position;

    void *tmp = capy_vec_reserve(arena, src, element_size, capacity, new_size);

    if (tmp != NULL)
    {
        char *data = cast(char *, tmp);

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

    return tmp;
}

void capy_vec_delete(void *items, size_t element_size, size_t *size, size_t position, size_t count)
{
    capy_assert(items != NULL);
    capy_assert(size != NULL);
    capy_assert(position + count <= *size);

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
}
