#include <capy/capy.h>
#include <errno.h>

static const uint64_t rapidhash_secret[8] = {
    0x2d358dccaa6c78a5ull,
    0x8bb84b93962eacc9ull,
    0x4b33a62ed433d4a3ull,
    0x4d5a2da51de1aa47ull,
    0xa0761d6478bd642full,
    0xe7037ed1a0b428dbull,
    0x90ed1765281c388cull,
    0xaaaaaaaaaaaaaaaaull,
};

static inline void rapidhash_mum(uint64_t *A, uint64_t *B)
{
    __uint128_t r = *A;
    r *= *B;
    *A = (uint64_t)(r);
    *B = (uint64_t)(r >> 64);
}

static inline uint64_t rapidhash_mix(uint64_t A, uint64_t B)
{
    rapidhash_mum(&A, &B);
    return A ^ B;
}

static inline uint64_t rapidhash_read64(const uint8_t *p)
{
    uint64_t v;
    memcpy(&v, p, sizeof(uint64_t));
    return v;
}

static inline uint64_t rapidhash_read32(const uint8_t *p)
{
    uint32_t v;
    memcpy(&v, p, sizeof(uint32_t));
    return v;
}

uint64_t rapidhash(const void *key, size_t len, uint64_t seed, const uint64_t *secret)
{
    const uint8_t *p = key;
    seed ^= rapidhash_mix(seed ^ secret[2], secret[1]);
    uint64_t a = 0, b = 0;
    size_t i = len;

    if (len <= 16)
    {
        if (len >= 4)
        {
            seed ^= len;

            if (len >= 8)
            {
                const uint8_t *plast = p + len - 8;
                a = rapidhash_read64(p);
                b = rapidhash_read64(plast);
            }
            else
            {
                const uint8_t *plast = p + len - 4;
                a = rapidhash_read32(p);
                b = rapidhash_read32(plast);
            }
        }
        else if (len > 0)
        {
            a = (((uint64_t)p[0]) << 45) | p[len - 1];
            b = p[len >> 1];
        }
        else
        {
            a = b = 0;
        }
    }
    else
    {
        if (i > 48)
        {
            uint64_t see1 = seed, see2 = seed;

            do
            {
                seed = rapidhash_mix(rapidhash_read64(p) ^ secret[0], rapidhash_read64(p + 8) ^ seed);
                see1 = rapidhash_mix(rapidhash_read64(p + 16) ^ secret[1], rapidhash_read64(p + 24) ^ see1);
                see2 = rapidhash_mix(rapidhash_read64(p + 32) ^ secret[2], rapidhash_read64(p + 40) ^ see2);
                p += 48;
                i -= 48;
            } while (i > 48);

            seed ^= see1;
            seed ^= see2;
        }
        if (i > 16)
        {
            seed = rapidhash_mix(rapidhash_read64(p) ^ secret[2], rapidhash_read64(p + 8) ^ seed);

            if (i > 32)
            {
                seed = rapidhash_mix(rapidhash_read64(p + 16) ^ secret[2], rapidhash_read64(p + 24) ^ seed);
            }
        }

        a = rapidhash_read64(p + i - 16) ^ i;
        b = rapidhash_read64(p + i - 8);
    }

    a ^= secret[1];
    b ^= seed;

    rapidhash_mum(&a, &b);

    return rapidhash_mix(a ^ secret[7], b ^ secret[1] ^ i);
}

uint64_t capy_hash(const char *key, uint64_t length)
{
    uint64_t hash = rapidhash(key, length, 0, rapidhash_secret);
    return hash;
}

int _capy_vec_reserve(struct capy_vec *arr, size_t element_size, size_t capacity)
{
    if (arr->data == NULL || arr->capacity < capacity)
    {
        void *data = realloc(arr->data, element_size * capacity);

        if (data == NULL)
        {
            return ENOMEM;
        }

        arr->data = data;
        arr->capacity = capacity;
    }

    return 0;
}

int _capy_vec_resize(struct capy_vec *arr, size_t element_size, size_t size)
{
    if (size > arr->capacity)
    {
        int err = _capy_vec_reserve(arr, element_size, 2 * size);

        if (err)
        {
            return err;
        }
    }

    arr->size = size;
    return 0;
}

int _capy_vec_insert_at(struct capy_vec *arr, size_t element_size, size_t position, size_t size, void *values)
{
    if (position > arr->size)
    {
        return EINVAL;
    }

    size_t tail_size = arr->size - position;

    int err = _capy_vec_resize(arr, element_size, arr->size + size);

    if (err)
    {
        return err;
    }

    if (tail_size > 0)
    {
        memmove(
            arr->data + (element_size * (position + size)),
            arr->data + (element_size * position),
            element_size * tail_size);
    }

    if (values != NULL)
    {
        memcpy(
            arr->data + (element_size * position),
            values,
            element_size * size);
    }

    return 0;
}

int _capy_vec_delete_at(struct capy_vec *arr, size_t element_size, size_t position, size_t size)
{
    if (position + size > arr->size)
    {
        return EINVAL;
    }

    size_t tail_size = arr->size - (position + size);

    if (tail_size > 0)
    {
        memmove(
            arr->data + (element_size * position),
            arr->data + (element_size * (position + size)),
            element_size * tail_size);
    }

    arr->size -= size;
    return 0;
}

void *_capy_smap_get(struct capy_smap *smap, size_t pair_size, const char *key)
{
    if (smap->data == NULL)
    {
        return NULL;
    }

    size_t k = capy_hash(key, strlen(key)) % smap->capacity;
    size_t j = 1;

    for (;;)
    {
        const char **item = smap->data + (pair_size * k);

        if (item[0] == NULL)
        {
            return NULL;
        }

        if (item[0] != TOMBSTONE && strcmp(key, item[0]) == 0)
        {
            return item;
        }

        k = (k + j) % smap->capacity;
        j += 1;
    }
}

int _capy_smap_set(struct capy_smap *smap, size_t pair_size, const char *key, void *value)
{
    if (smap->size >= (smap->capacity * 2) / 3)
    {
        size_t buffer_capacity = smap->capacity * 2;

        if (buffer_capacity == 0)
        {
            buffer_capacity = 8;
        }

        void *buffer = malloc(buffer_capacity * pair_size);

        if (buffer == NULL)
        {
            return ENOMEM;
        }

        memset(buffer, 0, buffer_capacity * pair_size);

        for (size_t i = 0; i < smap->capacity; i++)
        {
            const char **src = smap->data + (pair_size * i);

            if (src[0] == NULL || src[0] == TOMBSTONE)
            {
                continue;
            }

            size_t k = capy_hash(src[0], strlen(src[0])) % buffer_capacity;
            size_t j = 1;

            for (;;)
            {
                const char **dst = buffer + (pair_size * k);

                if (dst[0] == NULL)
                {
                    memcpy(dst, src, pair_size);
                    break;
                }

                k = (k + j) % buffer_capacity;
                j += 1;
            }
        }

        free(smap->data);
        smap->data = buffer;
        smap->capacity = buffer_capacity;
    }

    size_t k = capy_hash(key, strlen(key)) % smap->capacity;
    size_t j = 1;

    for (;;)
    {
        const char **dst = smap->data + (pair_size * k);

        if (dst[0] == NULL || dst[0] == TOMBSTONE || strcmp(*dst, key) == 0)
        {
            memcpy(dst, value, pair_size);
            smap->size += 1;
            return 0;
        }

        k = (k + j) % smap->capacity;
        j += 1;
    }
}

void _capy_smap_delete(struct capy_smap *smap, size_t pair_size, const char *key)
{
    const char **pair = _capy_smap_get(smap, pair_size, key);

    if (pair != NULL)
    {
        pair[0] = TOMBSTONE;
        smap->size -= 1;
    }
}

void *_capy_smap_next(struct capy_smap *smap, size_t pair_size, void *it)
{
    void *end = smap->data + (pair_size * smap->capacity);

    it = (it == NULL) ? smap->data : it + pair_size;

    while (it < end)
    {
        const char **item = it;

        if (item[0] != NULL && item[0] != TOMBSTONE)
        {
            return it;
        }

        it += pair_size;
    }

    return NULL;
}

capy_smap_define(size_t);

struct capy_symbol capy_symbols_add(struct capy_symbols *pool, const char *str)
{
    const char *entry = capy_sset_get(&pool->symbols, str);

    if (entry != NULL)
    {
        return (struct capy_symbol){pool, entry};
    }

    size_t length = strlen(str) + 1;

    void *buffer = malloc(length);

    if (buffer == NULL)
    {
        return (struct capy_symbol){NULL};
    }

    memcpy(buffer, str, length);

    int err = capy_sset_set(&pool->symbols, buffer);

    if (err)
    {
        return (struct capy_symbol){NULL};
    }

    return (struct capy_symbol){pool, buffer};
}

void capy_symbols_free(struct capy_symbols *pool)
{
    for (const char **it = capy_sset_next(&pool->symbols, NULL);
         it != NULL;
         it = capy_sset_next(&pool->symbols, it))
    {
        free((void *)it[0]);
    }

    free(pool->symbols.data);

    pool->symbols = (union capy_sset){NULL};
}
