#ifndef CAPY_VEC_H
#define CAPY_VEC_H

#include <capy/arena.h>
#include <capy/assert.h>
#include <capy/std.h>

must_check void *capy_vec_reserve(capy_arena *arena, void *items,
                                  size_t element_size, size_t *capacity, size_t size);

must_check void *capy_vec_insert(capy_arena *arena, void *items,
                                 size_t element_size, size_t *capacity, size_t *size,
                                 size_t position, size_t count, const void *values);

void capy_vec_delete(void *items, size_t element_size, size_t *size, size_t position, size_t count);

#endif
