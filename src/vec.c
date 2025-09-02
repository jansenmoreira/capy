#include <capy/capy.h>
#include <errno.h>

extern inline capy_vec *capy_vec_head(void *data);
extern inline size_t capy_vec_size(void *data);
extern inline size_t capy_vec_capacity(void *data);
extern inline void capy_vec_fixed(void *data);

void *capy_vec_init(capy_arena *arena, size_t element_size, size_t capacity)
{
    size_t total = sizeof(capy_vec) + (element_size * capacity);

    capy_vec *vector = capy_arena_grow(arena, total, alignof(capy_vec));

    vector->arena = arena;
    vector->capacity = capacity;
    vector->element_size = element_size;

    return vector->data;
}

void *capy_vec_reserve(void *data, size_t capacity)
{
    capy_vec *vec = capy_vec_head(data);

    capy_assert(vec != NULL);

    if (capacity <= vec->capacity)
    {
        return data;
    }

    capy_assert(vec->arena != NULL);

    uint8_t *vec_top = vec->data + (vec->element_size * vec->capacity);
    uint8_t *arena_top = capy_arena_top(vec->arena);

    if (vec_top == arena_top)
    {
        capy_arena_grow(vec->arena, vec->element_size * (capacity - vec->capacity), 0);
    }
    else
    {
        data = capy_vec_init(vec->arena, vec->element_size, capacity);
        memcpy(data, vec->data, vec->size * vec->element_size);
        vec = capy_vec_head(data);
    }

    vec->capacity = capacity;

    return data;
}

void *capy_vec_push(void *data, void *value)
{
    capy_vec *vec = capy_vec_head(data);
    return capy_vec_insert(data, vec->size, 1, value);
}

void *capy_vec_pop(void *data)
{
    capy_vec *vec = capy_vec_head(data);

    capy_assert(vec != NULL);
    capy_assert(vec->size > 0);

    return capy_vec_resize(data, vec->size - 1);
}

void *capy_vec_resize(void *data, size_t size)
{
    capy_vec *vec = capy_vec_head(data);

    capy_assert(vec != NULL);

    if (size > vec->capacity)
    {
        data = capy_vec_reserve(data, 2 * size);
        vec = capy_vec_head(data);
    }

    vec->size = size;

    return data;
}

void *capy_vec_insert(void *data, size_t position, size_t size, void *values)
{
    capy_vec *vec = capy_vec_head(data);

    capy_assert(vec != NULL);
    capy_assert(position <= vec->size);

    size_t tail_size = vec->size - position;

    data = capy_vec_resize(data, vec->size + size);
    vec = capy_vec_head(data);

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

    return data;
}

void *capy_vec_delete(void *data, size_t position, size_t size)
{
    capy_vec *vec = capy_vec_head(data);

    capy_assert(vec != NULL);
    capy_assert(position + size <= vec->size);

    size_t tail_size = vec->size - (position + size);

    if (tail_size > 0)
    {
        memmove(vec->data + (vec->element_size * (position)),
                vec->data + (vec->element_size * (position + size)),
                vec->element_size * tail_size);
    }

    return capy_vec_resize(data, vec->size - size);
}
