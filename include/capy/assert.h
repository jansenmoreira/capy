#ifndef CAPY_ASSERT_H
#define CAPY_ASSERT_H

#include <capy/std.h>

static inline void capy_assert_abort(int condition, const char *file, int line, const char *expression)
{
    if (!condition)
    {
        fprintf(stderr, "%s:%d: expected condition `%s` failed!!!\n", file, line, expression);
        abort();
    }
}

#ifdef NDEBUG
#define capy_assert(exp) (0)
#else
#define capy_assert(exp) capy_assert_abort((exp), __FILE__, __LINE__, #exp)
#endif

#endif
