#ifndef CAPY_SMAP_H
#define CAPY_SMAP_H

#include <stdint.h>
#include <stddef.h>

struct capy_smap
{
    ptrdiff_t size;
    ptrdiff_t capacity;
};

static inline struct capy_smap *capy_smap_head(void *data)
{
    return (data == NULL) ? NULL : (struct capy_smap *)(data)-1;
}

static inline ptrdiff_t capy_smap_size(void *data)
{
    return (data == NULL) ? 0 : ((struct capy_smap *)(data)-1)->size;
}

static inline ptrdiff_t capy_smap_capacity(void *data)
{
    return (data == NULL) ? 0 : ((struct capy_smap *)(data)-1)->capacity;
}

void *capy_smap_get(void **ptr, ptrdiff_t pair_size, const char *key);
int capy_smap_set(void **ptr, ptrdiff_t pair_size, const char *key, void *index);
void capy_smap_delete(void **ptr, ptrdiff_t pair_size, const char *key);
void *capy_smap_next(void *data, ptrdiff_t pair_size, void *it);

#define TOMBSTONE ((void *)SIZE_MAX)

#define capy_smap_define(tag, T)                                         \
    static inline int capy_smap_set_##tag(T **smap, T pair)              \
    {                                                                    \
        return capy_smap_set((void **)smap, sizeof(T), pair.key, &pair); \
    }                                                                    \
                                                                         \
    static inline void capy_smap_delete_##tag(T **smap, const char *key) \
    {                                                                    \
        capy_smap_delete((void **)smap, sizeof(T), key);                 \
    }                                                                    \
                                                                         \
    static inline T capy_smap_get_##tag(T **smap, const char *key)       \
    {                                                                    \
        T *pair = capy_smap_get((void **)smap, sizeof(T), key);          \
        return (pair == NULL) ? (T){key = NULL} : *pair;                 \
    }                                                                    \
                                                                         \
    static inline T *capy_smap_next_##tag(T *smap, T *it)                \
    {                                                                    \
        return capy_smap_next((void *)smap, sizeof(T), it);              \
    }

static inline int capy_sset_set(const char ***sset, const char *key)
{
    return capy_smap_set((void **)sset, sizeof(const char *), key, &key);
}

static inline void capy_sset_delete(const char ***sset, const char *key)
{
    capy_smap_delete((void **)sset, sizeof(const char *), key);
}

static inline const char *capy_sset_get(const char ***sset, const char *key)
{
    const char **value = capy_smap_get((void **)sset, sizeof(const char *), key);
    if (value == NULL) return NULL;
    return *value;
}

static inline const char **capy_sset_next(const char **sset, const char **it)
{
    return capy_smap_next((void *)sset, sizeof(const char *), it);
}

const char *capy_symbols_add(const char ***pool, const char *str);
void capy_symbols_free(const char ***pool);

#endif
