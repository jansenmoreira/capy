#ifndef CAPY_SMAP_H
#define CAPY_SMAP_H

#include <capy/arena.h>
#include <capy/assert.h>
#include <capy/std.h>
#include <capy/string.h>

typedef struct capy_smap
{
    size_t size;
    size_t capacity;
    size_t element_size;
    capy_arena *arena;
    uint8_t data[];
} capy_smap;

static inline size_t capy_smap_size(void *data)
{
    capy_assert(data != NULL);  // GCOVR_EXCL_LINE

    return ((capy_smap *)(data)-1)->size;
}

static inline size_t capy_smap_capacity(void *data)
{
    capy_assert(data != NULL);  // GCOVR_EXCL_LINE

    return ((capy_smap *)(data)-1)->capacity;
}

void *capy_smap_init(capy_arena *arena, size_t element_size, size_t capacity);
void *capy_smap_get(void *ptr, capy_string key);
void *capy_smap_set(void *ptr, capy_string *pair);
void *capy_smap_delete(void *ptr, capy_string key);

#define TOMBSTONE ((void *)-1)

#define capy_smap_of(T, arena, capacity) \
    ((T *)(capy_smap_init((arena), sizeof(T), (capacity))))

// String Set

static inline capy_string *capy_sset_init(capy_arena *arena, size_t capacity)
{
    return capy_smap_init(arena, sizeof(capy_string), capacity);
}

static inline bool capy_sset_has(capy_string **ptr, capy_string key)
{
    return capy_smap_get(ptr, key) != NULL;
}

static inline capy_string *capy_sset_set(capy_string **ptr, capy_string key)
{
    return capy_smap_set(ptr, &key);
}

static inline capy_string *capy_sset_delete(capy_string **ptr, capy_string key)
{
    return capy_smap_delete(ptr, key);
}

#endif
