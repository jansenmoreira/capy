#ifndef CAPY_ARENA_H
#define CAPY_ARENA_H

#include <capy/std.h>

// TYPES

typedef struct capy_arena capy_arena;

// DECLARATIONS

must_check capy_arena *capy_arena_init(size_t limit);
void capy_arena_destroy(capy_arena *arena);
must_check void *capy_arena_realloc(capy_arena *arena, void *data, size_t size, size_t new_size, int zeroinit);
must_check void *capy_arena_alloc(capy_arena *arena, size_t size, size_t align, int zeroinit);
must_check int capy_arena_free(capy_arena *arena, void *addr);
void *capy_arena_end(capy_arena *arena);
size_t capy_arena_size(capy_arena *arena);

// MACROS

#define capy_arena_make(arena, T, size) \
    (capy_arena_alloc((arena), sizeof(T) * (size), alignof(T), true))

#define capy_arena_umake(arena, T, size) \
    (capy_arena_alloc((arena), sizeof(T) * (size), alignof(T), false))

#endif
