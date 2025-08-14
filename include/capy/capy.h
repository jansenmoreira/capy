#ifndef CAPY_H
#define CAPY_H

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct capy_vec
{
    void *data;
    size_t size;
    size_t capacity;
};

int _capy_vec_reserve(struct capy_vec *vec, size_t element_size, size_t capacity);
int _capy_vec_resize(struct capy_vec *vec, size_t element_size, size_t size);
int _capy_vec_insert_at(struct capy_vec *vec, size_t element_size, size_t position, size_t size, void *values);
int _capy_vec_delete_at(struct capy_vec *vec, size_t element_size, size_t position, size_t size);

uint64_t capy_hash(const char *key, uint64_t length);

#define capy_vec_define(T)                                                                                     \
    union capy_vec_##T                                                                                         \
    {                                                                                                          \
        struct                                                                                                 \
        {                                                                                                      \
            T *data;                                                                                           \
            size_t size;                                                                                       \
        };                                                                                                     \
        struct capy_vec vector;                                                                                \
    };                                                                                                         \
                                                                                                               \
    static inline int capy_vec_reserve_##T(union capy_vec_##T *vec, size_t capacity)                           \
    {                                                                                                          \
        return _capy_vec_reserve(&vec->vector, sizeof(T), capacity);                                           \
    }                                                                                                          \
                                                                                                               \
    static inline int capy_vec_resize_##T(union capy_vec_##T *vec, size_t size)                                \
    {                                                                                                          \
        return _capy_vec_resize(&vec->vector, sizeof(T), size);                                                \
    }                                                                                                          \
                                                                                                               \
    static inline int capy_vec_insert_value_at_##T(union capy_vec_##T *vec, size_t position, T value)          \
    {                                                                                                          \
        return _capy_vec_insert_at(&vec->vector, sizeof(T), position, 1, &value);                              \
    }                                                                                                          \
                                                                                                               \
    static inline int capy_vec_insert_at_##T(union capy_vec_##T *vec, size_t position, size_t size, T *values) \
    {                                                                                                          \
        return _capy_vec_insert_at(&vec->vector, sizeof(T), position, size, values);                           \
    }                                                                                                          \
                                                                                                               \
    static inline int capy_vec_delete_at_##T(union capy_vec_##T *vec, size_t position, size_t size)            \
    {                                                                                                          \
        return _capy_vec_delete_at(&vec->vector, sizeof(T), position, size);                                   \
    }                                                                                                          \
                                                                                                               \
    static inline int capy_vec_push_##T(union capy_vec_##T *vec, T value)                                      \
    {                                                                                                          \
        return _capy_vec_insert_at(&vec->vector, sizeof(T), vec->size, 1, &value);                             \
    }                                                                                                          \
                                                                                                               \
    static inline int capy_vec_pop_##T(union capy_vec_##T *vec)                                                \
    {                                                                                                          \
        return _capy_vec_resize(&vec->vector, sizeof(T), vec->size - 1);                                       \
    }

struct capy_smap
{
    void *data;
    size_t size;
    size_t capacity;
};

void *_capy_smap_get(struct capy_smap *smap, size_t pair_size, const char *key);
int _capy_smap_set(struct capy_smap *smap, size_t pair_size, const char *key, void *index);
void _capy_smap_delete(struct capy_smap *smap, size_t pair_size, const char *key);
void *_capy_smap_next(struct capy_smap *smap, size_t pair_size, void *it);

#define TOMBSTONE ((void *)SIZE_MAX)

#define capy_smap_define(T)                                                                                               \
    struct capy_smap_pair_##T                                                                                             \
    {                                                                                                                     \
        const char *key;                                                                                                  \
        T value;                                                                                                          \
    };                                                                                                                    \
                                                                                                                          \
    union capy_smap_##T                                                                                                   \
    {                                                                                                                     \
        struct                                                                                                            \
        {                                                                                                                 \
            struct capy_smap_pair_##T *data;                                                                              \
            size_t size;                                                                                                  \
            size_t capacity;                                                                                              \
        };                                                                                                                \
        struct capy_smap smap;                                                                                            \
    };                                                                                                                    \
                                                                                                                          \
    static inline int capy_smap_set_##T(union capy_smap_##T *smap, const char *key, T value)                              \
    {                                                                                                                     \
        struct capy_smap_pair_##T pair = {key, value};                                                                    \
        return _capy_smap_set(&smap->smap, sizeof(struct capy_smap_pair_##T), key, &pair);                                \
    }                                                                                                                     \
                                                                                                                          \
    static inline void capy_smap_delete_##T(union capy_smap_##T *smap, const char *key)                                   \
    {                                                                                                                     \
        _capy_smap_delete(&smap->smap, sizeof(struct capy_smap_pair_##T), key);                                           \
    }                                                                                                                     \
                                                                                                                          \
    static inline struct capy_smap_pair_##T *capy_smap_next_##T(union capy_smap_##T *smap, struct capy_smap_pair_##T *it) \
    {                                                                                                                     \
        return _capy_smap_next(&smap->smap, sizeof(struct capy_smap_pair_##T), it);                                       \
    }                                                                                                                     \
                                                                                                                          \
    static inline struct capy_smap_pair_##T capy_smap_get_##T(union capy_smap_##T *smap, const char *key)                 \
    {                                                                                                                     \
        struct capy_smap_pair_##T *pair = _capy_smap_get(&smap->smap, sizeof(struct capy_smap_pair_##T), key);            \
        if (pair == NULL) return (struct capy_smap_pair_##T){NULL};                                                       \
        return (struct capy_smap_pair_##T){pair->key, pair->value};                                                       \
    }

union capy_sset
{
    struct
    {
        const char **data;
        size_t size;
        size_t capacity;
    };
    struct capy_smap smap;
};

static inline int capy_sset_set(union capy_sset *strset, const char *key)
{
    return _capy_smap_set(&strset->smap, sizeof(const char *), key, &key);
}

static inline void capy_sset_delete(union capy_sset *strset, const char *key)
{
    _capy_smap_delete(&strset->smap, sizeof(const char *), key);
}

static inline const char *capy_sset_get(union capy_sset *smap, const char *key)
{
    const char **value = _capy_smap_get(&smap->smap, sizeof(const char *), key);
    if (value == NULL) return NULL;
    return *value;
}

static inline const char **capy_sset_next(union capy_sset *strset, const char **it)
{
    return _capy_smap_next(&strset->smap, sizeof(const char *), it);
}

struct capy_symbols
{
    union capy_sset symbols;
};

struct capy_symbol
{
    struct capy_symbols *pool;
    const char *data;
};

struct capy_symbol capy_symbols_add(struct capy_symbols *pool, const char *str);
void capy_symbols_free(struct capy_symbols *pool);

#endif
