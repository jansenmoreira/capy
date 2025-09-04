#ifndef CAPY_MATH_H
#define CAPY_MATH_H

#include <capy/std.h>

inline size_t align_to(size_t v, size_t n)
{
    size_t rem = v % n;
    return (rem == 0) ? v : v + n - rem;
}

#endif
