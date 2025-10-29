#include <capy/macros.h>

// PUBLIC DEFINITIONS

size_t capy_next_pow2(size_t v)
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

int64_t capy_timespec_diff(struct timespec a, struct timespec b)
{
    int64_t ns = Cast(int64_t, a.tv_sec - b.tv_sec) * (1000 * 1000 * 1000);
    ns += Cast(int64_t, a.tv_nsec - b.tv_nsec);
    return ns;
}

struct timespec capy_timespec_addms(struct timespec t, uint64_t ms)
{
    t.tv_sec += Cast(int64_t, ms) / 1000;
    t.tv_nsec += (Cast(int64_t, ms) % 1000) * 1000000;
    return t;
}

struct timespec capy_now(void)
{
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    return now;
}

void capy_normalize_ns(int64_t *ns, const char **unit)
{
    const int64_t s = SecondsNano(1);
    const int64_t ms = MillisecondsNano(1);
    const int64_t us = MicrosecondsNano(1);

    if (*ns > s)
    {
        *unit = "s";
        *ns /= s;
    }
    else if (*ns > ms)
    {
        *unit = "ms";
        *ns /= ms;
    }
    else if (*ns > us)
    {
        *unit = "μs";
        *ns /= us;
    }
    else
    {
        *unit = "ns";
    }
}
