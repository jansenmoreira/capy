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
    uint8_t *data;
} capy_vec;

capy_vec *capy_vec_init(capy_arena *arena, size_t element_size, size_t capacity);
void capy_vec_reserve(capy_vec *vec, size_t capacity);
void capy_vec_resize(capy_vec *vec, size_t size);
void capy_vec_insert(capy_vec *vec, size_t position, size_t size, const void *values);
void capy_vec_delete(capy_vec *vec, size_t position, size_t size);
void capy_vec_push(capy_vec *vec, void *value);
void capy_vec_pop(capy_vec *vec);

#define capy_vec_of(T, arena, capacity) capy_vec_init((arena), sizeof(T), (capacity))
#define capy_vec_data(T, vec) ((T *)((vec)->data))

#endif
