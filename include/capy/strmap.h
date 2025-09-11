#ifndef CAPY_STRMAP_H
#define CAPY_STRMAP_H

#include <capy/string.h>

void *capy_strmap_get(void *data, size_t element_size, size_t capacity, capy_string key);
must_check void *capy_strmap_set(capy_arena *arena, void *data, size_t element_size, size_t *capacity, size_t *size, const void *entry);
void capy_strmap_delete(void *data, size_t element_size, size_t capacity, size_t *size, capy_string key);

typedef struct capy_strset
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_string *items;
} capy_strset;

must_check capy_strset *capy_strset_init(capy_arena *arena, size_t capacity);
int capy_strset_has(capy_strset *s, capy_string key);
must_check int capy_strset_add(capy_strset *s, capy_string key);
void capy_strset_delete(capy_strset *s, capy_string key);

typedef struct capy_strkv
{
    capy_string key;
    capy_string value;
} capy_strkv;

typedef struct capy_strkvmap
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_strkv *items;
} capy_strkvmap;

must_check capy_strkvmap *capy_strkvmap_init(capy_arena *arena, size_t capacity);
capy_strkv *capy_strkvmap_get(capy_strkvmap *m, capy_string key);
must_check int capy_strkvmap_set(capy_strkvmap *m, capy_string key, capy_string value);
void capy_strkvmap_delete(capy_strkvmap *m, capy_string key);

typedef struct capy_strkvn
{
    capy_string key;
    capy_string value;
    struct capy_strkvn *next;
} capy_strkvn;

typedef struct capy_strkvmmap
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_strkvn *items;
} capy_strkvmmap;

must_check capy_strkvmmap *capy_strkvmmap_init(capy_arena *arena, size_t capacity);
capy_strkvn *capy_strkvmmap_get(capy_strkvmmap *mm, capy_string key);
must_check int capy_strkvmmap_set(capy_strkvmmap *mm, capy_string key, capy_string value);
must_check int capy_strkvmmap_add(capy_strkvmmap *mm, capy_string key, capy_string value);
void capy_strkvmmap_delete(capy_strkvmmap *mm, capy_string key);
void capy_strkvmmap_clear(capy_strkvmmap *mm);

#endif
