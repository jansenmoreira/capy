#include <capy/capy.h>
#include <errno.h>

extern inline capy_vec *capy_vec_head(void *data);
extern inline size_t capy_vec_size(void *data);
extern inline size_t capy_vec_capacity(void *data);
extern inline void capy_vec_fixed(void *data);

capy_vec *capy_vec_init(capy_arena *arena, size_t element_size, size_t capacity)
{
    size_t total = sizeof(capy_vec) + (element_size * capacity);

    capy_vec *vector = capy_arena_grow(arena, total, 8, true);

    vector->arena = arena;
    vector->capacity = capacity;
    vector->element_size = element_size;
    vector->data = (uint8_t *)(&vector[1]);

    return vector;
}

void capy_vec_reserve(capy_vec *vec, size_t capacity)
{
    capy_assert(vec != NULL);

    if (capacity <= vec->capacity)
    {
        return;
    }

    capy_assert(vec->arena != NULL);

    uint8_t *vec_top = vec->data + (vec->element_size * vec->capacity);
    uint8_t *arena_top = capy_arena_top(vec->arena);

    if (vec_top == arena_top)
    {
        capy_arena_grow(vec->arena, vec->element_size * (capacity - vec->capacity), 0, true);
    }
    else
    {
        uint8_t *data = capy_arena_grow(vec->arena, vec->element_size * capacity, 8, true);
        memcpy(data, vec->data, vec->size * vec->element_size);
        vec->data = data;
    }

    vec->capacity = capacity;
}

void capy_vec_push(capy_vec *vec, void *value)
{
    capy_vec_insert(vec, vec->size, 1, value);
}

void capy_vec_pop(capy_vec *vec)
{
    capy_assert(vec != NULL);
    capy_assert(vec->size > 0);
    capy_vec_resize(vec, vec->size - 1);
}

void capy_vec_resize(capy_vec *vec, size_t size)
{
    capy_assert(vec != NULL);

    if (size > vec->capacity)
    {
        capy_vec_reserve(vec, 2 * size);
    }

    vec->size = size;
}

void capy_vec_insert(capy_vec *vec, size_t position, size_t size, const void *values)
{
    capy_assert(vec != NULL);
    capy_assert(position <= vec->size);

    size_t tail_size = vec->size - position;

    capy_vec_resize(vec, vec->size + size);

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
}

void capy_vec_delete(capy_vec *vec, size_t position, size_t size)
{
    capy_assert(vec != NULL);
    capy_assert(position + size <= vec->size);

    size_t tail_size = vec->size - (position + size);

    if (tail_size > 0)
    {
        memmove(vec->data + (vec->element_size * (position)),
                vec->data + (vec->element_size * (position + size)),
                vec->element_size * tail_size);
    }

    capy_vec_resize(vec, vec->size - size);
}
