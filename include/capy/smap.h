#ifndef CAPY_SMAP_H
#define CAPY_SMAP_H

#include <capy/arena.h>
#include <capy/assert.h>
#include <capy/std.h>
#include <capy/string.h>

#define TOMBSTONE ((void *)-1)

typedef struct capy_smap
{
    size_t size;
    size_t capacity;
    size_t element_size;
    capy_arena *arena;
    uint8_t *data;
} capy_smap;

capy_smap *capy_smap_init(capy_arena *arena, size_t element_size, size_t capacity);
void *capy_smap_get(capy_smap *smap, capy_string key);
void capy_smap_set(capy_smap *smap, capy_string *pair);
void capy_smap_delete(capy_smap *smap, capy_string key);
void capy_smap_clear(capy_smap *smap);

#define capy_smap_of(T, arena, capacity) capy_smap_init((arena), sizeof(T), (capacity))
#define capy_smap_data(T, smap) ((T *)((smap)->data))

#endif
