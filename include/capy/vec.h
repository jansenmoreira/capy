#ifndef CAPY_VECTOR_H
#define CAPY_VECTOR_H

#include <capy/arena.h>
#include <capy/assert.h>
#include <capy/std.h>

typedef struct capy_vec
{
    size_t size;
    size_t capacity;
    size_t element_size;
    capy_arena *arena;
    uint8_t data[];
} capy_vec;

static inline size_t capy_vec_size(void *data)
{
    capy_assert(data != NULL);  // GCOVR_EXCL_LINE

    return ((capy_vec *)(data)-1)->size;
}

static inline size_t capy_vec_capacity(void *data)
{
    capy_assert(data != NULL);  // GCOVR_EXCL_LINE

    return ((capy_vec *)(data)-1)->capacity;
}

static inline void capy_vec_fixed(void *data)
{
    capy_assert(data != NULL);  // GCOVR_EXCL_LINE

    ((capy_vec *)(data)-1)->arena = NULL;
}

void *capy_vec_init(capy_arena *arena, size_t element_size, size_t capacity);
void *capy_vec_reserve(void *data, size_t capacity);
void *capy_vec_resize(void *data, size_t size);
void *capy_vec_insert(void *data, size_t position, size_t size, void *values);
void *capy_vec_delete(void *data, size_t position, size_t size);
void *capy_vec_pop(void *data);
void *capy_vec_push(void *data, void *value);

#define capy_vec_of(T, arena, capacity) \
    ((T *)(capy_vec_init((arena), sizeof(T), (capacity))))

#endif
