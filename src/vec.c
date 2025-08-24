#include <capy/capy.h>
#include <errno.h>

static inline capy_vec *capy_vec_head(void *ptr)
{
    capy_assert(ptr != NULL);  // GCOVR_EXCL_LINE

    capy_vec *data = *(capy_vec **)(ptr);

    capy_assert(data != NULL);  // GCOVR_EXCL_LINE

    return data - 1;
}

void *capy_vec_init(capy_arena *arena, size_t element_size, size_t capacity)
{
    size_t total = sizeof(capy_vec) + (element_size * capacity);

    capy_vec *vector = capy_arena_grow(arena, total, alignof(capy_vec));

    if (!vector)
    {
        return NULL;
    }

    vector->arena = arena;
    vector->capacity = capacity;
    vector->element_size = element_size;

    return vector->data;
}

int capy_vec_reserve(void *ptr, size_t capacity)
{
    capy_vec *vec = capy_vec_head(ptr);

    capy_assert(vec != NULL);  // GCOVR_EXCL_LINE

    if (capacity <= vec->capacity)
    {
        return 0;
    }

    if (vec->arena == NULL)
    {
        return ENOMEM;
    }

    uint8_t *vec_top = vec->data + (vec->element_size * vec->capacity);
    uint8_t *arena_top = capy_arena_top(vec->arena);

    if (vec_top == arena_top)
    {
        if (capy_arena_grow(vec->arena, vec->element_size * (capacity - vec->capacity), 0) == NULL)
        {
            return ENOMEM;
        }
    }
    else
    {
        void *data = capy_vec_init(vec->arena, vec->element_size, capacity);

        if (data == NULL)
        {
            return ENOMEM;
        }

        memcpy(data, vec->data, vec->size * vec->element_size);

        *(void **)(ptr) = data;

        vec = capy_vec_head(ptr);
    }

    vec->capacity = capacity;

    return 0;
}

int capy_vec_push(void *ptr, void *value)
{
    capy_vec *vec = capy_vec_head(ptr);
    return capy_vec_insert(ptr, vec->size, 1, value);
}

int capy_vec_pop(void *ptr)
{
    capy_vec *vec = capy_vec_head(ptr);

    if (vec->size)
    {
        return capy_vec_resize(ptr, vec->size - 1);
    }

    return EINVAL;
}

int capy_vec_resize(void *ptr, size_t size)
{
    capy_vec *vec = capy_vec_head(ptr);

    capy_assert(vec != NULL);  // GCOVR_EXCL_LINE

    if (size > vec->capacity)
    {
        int err = capy_vec_reserve(ptr, 2 * size);

        if (err)
        {
            return err;
        }

        vec = capy_vec_head(ptr);
    }

    vec->size = size;

    return 0;
}

int capy_vec_insert(void *ptr, size_t position, size_t size, void *values)
{
    capy_vec *vec = capy_vec_head(ptr);

    capy_assert(vec != NULL);  // GCOVR_EXCL_LINE

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

    vec = capy_vec_head(ptr);

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

int capy_vec_delete(void *ptr, size_t position, size_t size)
{
    capy_vec *vec = capy_vec_head(ptr);

    capy_assert(vec != NULL);  // GCOVR_EXCL_LINE

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
