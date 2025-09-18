#include <capy/macros.h>
#include <capy/math.h>

size_t align_to(size_t v, size_t n)
{
    size_t rem = v % n;
    return (rem == 0) ? v : v + n - rem;
}

size_t next_pow2(size_t v)
{
    if (v == 0)
    {
        return 0;
    }

    size_t max = sizeof(size_t) * CHAR_BIT;

    v--;

    for (size_t i = 1; i < max; i <<= 1)
    {
        v |= v >> i;
    }

    v++;

    return v;
}

int64_t timespec_diff(struct timespec a, struct timespec b)
{
    int64_t ns = cast(int64_t, a.tv_sec - b.tv_sec) * 1000000000;
    ns += cast(int64_t, a.tv_nsec - b.tv_nsec);
    return ns;
}

void nanoseconds_normalize(int64_t *ns, const char **unit)
{
    if (*ns > 1000000000)
    {
        *unit = "s";
        *ns /= 1000000000;
    }
    else if (*ns > 1000000)
    {
        *unit = "ms";
        *ns /= 1000000;
    }
    else if (*ns > 1000)
    {
        *unit = "μs";
        *ns /= 1000;
    }
    else
    {
        *unit = "ns";
    }
}
