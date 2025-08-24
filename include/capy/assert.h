#ifndef CAPY_ASSERT_H
#define CAPY_ASSERT_H

#include <capy/std.h>

#ifdef NDEBUG
#define capy_assert(exp) (0)
#else
#define capy_assert(exp) ((exp) ? 0 : (fprintf(stderr, "%s:%d: expected condition `%s` failed\n", __FILE__, __LINE__, #exp), abort(), 0))
#endif

#endif
