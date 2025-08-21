#include <asm-generic/errno-base.h>
#include <assert.h>
#include <capy/capy.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

void *capy_vec_init(capy_arena *arena, size_t element_size, size_t capacity, int strategy)
{
    size_t total = sizeof(capy_vec) + (element_size * capacity);

    capy_vec *vector = capy_arena_grow(arena, total, alignof(capy_vec));

    vector->arena = arena;
    vector->capacity = capacity;
    vector->element_size = element_size;
    vector->strategy = strategy;

    if (vector)
    {
        return vector->data;
    }

    return NULL;
}

int capy_vec_reserve(void **ptr, size_t capacity)
{
    capy_vec *vec = capy_vec_head(*ptr);

    assert(vec != NULL);

    if (vec->capacity < capacity)
    {
        switch (vec->strategy)
        {
            case CAPY_VEC_UNBOUNDED:
            {
                if (capy_arena_grow(vec->arena, capacity - vec->capacity, 0) == NULL)
                {
                    return ENOMEM;
                }
            }
            break;

            case CAPY_VEC_REALLOC:
            {
                void *data = capy_vec_init(vec->arena, vec->element_size, capacity, vec->strategy);

                if (data == NULL)
                {
                    return ENOMEM;
                }

                memcpy(data, vec->data, vec->size * vec->element_size);

                *ptr = data;
                vec = capy_vec_head(data);
            }
            break;

            default:
            {
                return ENOMEM;
            }
        }

        vec->capacity = capacity;
    }

    return 0;
}

int capy_vec_resize(void **ptr, size_t size)
{
    capy_vec *vec = capy_vec_head(*ptr);

    assert(vec != NULL);

    if (size > vec->capacity)
    {
        int err = capy_vec_reserve(ptr, 2 * size);

        if (err)
        {
            return err;
        }

        vec = capy_vec_head(*ptr);
    }

    vec->size = size;

    return 0;
}

int capy_vec_insert(void **ptr, size_t position, size_t size, void *values)
{
    capy_vec *vec = capy_vec_head(*ptr);

    assert(vec != NULL);

    if (position > vec->size)
    {
        return EINVAL;
    }

    size_t tail_size = vec->size - position;

    int err = capy_vec_resize(ptr, vec->size + size);

    if (err)
    {
        return err;
    }

    vec = capy_vec_head(*ptr);

    if (tail_size > 0)
    {
        memmove(vec->data + (vec->element_size * (position + size)),
                vec->data + (vec->element_size * (position)),
                vec->element_size * tail_size);
    }

    if (values != NULL)
    {
        memcpy(vec->data + (vec->element_size * position),
               values,
               vec->element_size * size);
    }

    return 0;
}

int capy_vec_delete(void **ptr, size_t position, size_t size)
{
    capy_vec *vec = capy_vec_head(*ptr);

    assert(vec != NULL);

    if (position + size > vec->size)
    {
        return EINVAL;
    }

    size_t tail_size = vec->size - (position + size);

    if (tail_size > 0)
    {
        memmove(vec->data + (vec->element_size * (position)),
                vec->data + (vec->element_size * (position + size)),
                vec->element_size * tail_size);
    }

    return capy_vec_resize(ptr, vec->size - size);
}
