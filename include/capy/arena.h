#ifndef CAPY_ARENA_H
#define CAPY_ARENA_H

#include <capy/std.h>

typedef struct capy_arena capy_arena;

capy_arena *capy_arena_init(size_t limit);
void capy_arena_free(capy_arena *arena);

void *capy_arena_grow(capy_arena *arena, size_t size, size_t align, int zeroinit);
void capy_arena_shrink(capy_arena *arena, void *addr);

void *capy_arena_top(capy_arena *arena);
size_t capy_arena_size(capy_arena *arena);

#define capy_arena_make(T, arena, size) ((T *)(capy_arena_grow((arena), sizeof(T) * (size), alignof(T), true)))

#define capy_arena_umake(T, arena, size) \
    ((T *)(capy_arena_grow((arena), sizeof(T) * (size), alignof(T), false)))

#endif
