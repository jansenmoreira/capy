#ifndef CAPY_MATH_H
#define CAPY_MATH_H

#include <capy/std.h>
#include <limits.h>

// INLINE DEFINITIONS

inline size_t align_to(size_t v, size_t n)
{
    size_t rem = v % n;
    return (rem == 0) ? v : v + n - rem;
}

inline size_t next_pow2(size_t v)
{
    if (v == 0) return 0;

    size_t max = sizeof(size_t) * CHAR_BIT;

    v--;

    for (size_t i = 1; i < max; i <<= 1)
    {
        v |= v >> i;
    }

    v++;

    return v;
}

#endif
