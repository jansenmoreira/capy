#include <capy/capy.h>
#include <capy/macros.h>

// DEFINITIONS

void *capy_vec_insert(capy_arena *arena, void *items,
                      size_t element_size, size_t *capacity, size_t *size,
                      size_t position, size_t count, const void *values)
{
    capy_assert(items != NULL);
    capy_assert(capacity != NULL);
    capy_assert(size != NULL);
    capy_assert(position <= *size);

    size_t old_size = (*size);
    size_t new_size = old_size + count;
    size_t tail_size = old_size - position;

    void *tmp = items;

    if (new_size > *capacity)
    {
        if (arena == NULL)
        {
            return NULL;
        }

        size_t tmp_capacity = next_pow2(new_size);
        tmp = capy_arena_realloc(arena, tmp, (*capacity) * element_size, tmp_capacity * element_size, false);

        if (tmp == NULL)
        {
            return NULL;
        }

        *capacity = tmp_capacity;
    }

    char *data = Cast(char *, tmp);

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

    return tmp;
}

size_t capy_vec_delete(void *items, size_t element_size, size_t size, size_t position, size_t count)
{
    capy_assert(items != NULL);
    capy_assert(position + count <= size);

    size_t old_size = size;
    size_t new_size = old_size - count;
    size_t tail_size = old_size - (position + count);

    char *data = Cast(char *, items);

    if (tail_size > 0)
    {
        memmove(data + (element_size * (position)),
                data + (element_size * (position + count)),
                element_size * tail_size);
    }

    return new_size;
}
