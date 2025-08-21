#include <capy/capy.h>

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

uint64_t rapidhash(const void *key, ptrdiff_t len, uint64_t seed, const uint64_t *secret)
{
    const uint8_t *p = key;
    seed ^= rapidhash_mix(seed ^ secret[2], secret[1]);
    uint64_t a = 0, b = 0;
    ptrdiff_t i = len;

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
