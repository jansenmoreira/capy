#include <capy/macros.h>

// PUBLIC DEFINITIONS

MustCheck capy_err capy_vec_insert(capy_arena *arena, capy_vec *vec, size_t position, size_t count, const void *values)
{
    if (position > vec->size)
    {
        return ErrStd(EINVAL);
    }

    size_t new_size = vec->size + count;
    size_t tail_size = vec->size - position;

    if (new_size > vec->capacity)
    {
        if (arena == NULL)
        {
            return ErrStd(ENOMEM);
        }

        size_t new_capacity = next_pow2(new_size);

        char *tmp = capy_arena_realloc(arena,
                                       vec->items,
                                       vec->capacity * vec->element_size,
                                       new_capacity * vec->element_size,
                                       false);

        if (tmp == NULL)
        {
            return ErrStd(ENOMEM);
        }

        vec->capacity = new_capacity;
        vec->items = tmp;
    }

    if (tail_size > 0)
    {
        memmove(vec->items + (vec->element_size * (position + count)),
                vec->items + (vec->element_size * position),
                vec->element_size * tail_size);
    }

    if (values != NULL)
    {
        memcpy(vec->items + (vec->element_size * position),
               values,
               vec->element_size * count);
    }

    vec->size = new_size;
    return Ok;
}

capy_err capy_vec_delete(capy_vec *vec, size_t position, size_t count)
{
    if (position + count > vec->size)
    {
        return ErrStd(EINVAL);
    }

    if (count == 0)
    {
        return Ok;
    }

    size_t tail_size = vec->size - (position + count);

    if (tail_size > 0)
    {
        memmove(vec->items + (vec->element_size * position),
                vec->items + (vec->element_size * (position + count)),
                vec->element_size * tail_size);
    }

    vec->size = vec->size - count;

    return Ok;
}
