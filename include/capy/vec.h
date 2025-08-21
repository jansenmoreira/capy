#ifndef CAPY_VECTOR_H
#define CAPY_VECTOR_H

#include <capy/arena.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define CAPY_VEC_UNBOUNDED 0
#define CAPY_VEC_FIXED 1
#define CAPY_VEC_REALLOC 2

typedef struct capy_vec
{
    capy_arena *arena;
    size_t element_size;
    size_t size;
    size_t capacity;
    uint64_t strategy;
    uint8_t data[];
} capy_vec;

static inline capy_vec *capy_vec_head(void *data)
{
    return (data) ? (capy_vec *)(data)-1 : NULL;
}

static inline size_t capy_vec_size(void *data)
{
    return (data) ? ((capy_vec *)(data)-1)->size : 0;
}

static inline size_t capy_vec_capacity(void *data)
{
    return (data) ? ((capy_vec *)(data)-1)->capacity : 0;
}

void *capy_vec_init(capy_arena *arena, size_t element_size, size_t capacity, int strategy);
int capy_vec_reserve(void **ptr, size_t capacity);
int capy_vec_resize(void **ptr, size_t size);
int capy_vec_insert(void **ptr, size_t position, size_t size, void *values);
int capy_vec_delete(void **ptr, size_t position, size_t size);

#define capy_vec_define(tag, T)                                                                         \
    static inline T *capy_vec_init_##tag(capy_arena *arena, ptrdiff_t capacity, int strategy)           \
    {                                                                                                   \
        return capy_vec_init(arena, sizeof(T), capacity, strategy);                                     \
    }                                                                                                   \
    static inline int capy_vec_reserve_##tag(T **vec, ptrdiff_t capacity)                               \
    {                                                                                                   \
        return capy_vec_reserve((void **)(vec), capacity);                                              \
    }                                                                                                   \
                                                                                                        \
    static inline int capy_vec_resize_##tag(T **vec, ptrdiff_t size)                                    \
    {                                                                                                   \
        return capy_vec_resize((void **)(vec), size);                                                   \
    }                                                                                                   \
                                                                                                        \
    static inline int capy_vec_insert_##tag(T **vec, ptrdiff_t position, T value)                       \
    {                                                                                                   \
        return capy_vec_insert((void **)(vec), position, 1, &value);                                    \
    }                                                                                                   \
                                                                                                        \
    static inline int capy_vec_insertvals_##tag(T **vec, ptrdiff_t position, ptrdiff_t size, T *values) \
    {                                                                                                   \
        return capy_vec_insert((void **)(vec), position, size, values);                                 \
    }                                                                                                   \
                                                                                                        \
    static inline int capy_vec_delete_##tag(T **vec, ptrdiff_t position, ptrdiff_t size)                \
    {                                                                                                   \
        return capy_vec_delete((void **)(vec), position, size);                                         \
    }                                                                                                   \
                                                                                                        \
    static inline int capy_vec_push_##tag(T **vec, T value)                                             \
    {                                                                                                   \
        return capy_vec_insert((void **)(vec), capy_vec_size(*vec), 1, &value);                         \
    }                                                                                                   \
                                                                                                        \
    static inline int capy_vec_pop_##tag(T **vec)                                                       \
    {                                                                                                   \
        return capy_vec_resize((void **)(vec), capy_vec_size(*vec) - 1);                                \
    }

#endif
