#ifndef CAPY_ASSERT_H
#define CAPY_ASSERT_H

#include <capy/std.h>

void capy_assert_(int condition, const char *file, int line, const char *expression);

#ifdef NDEBUG
#define capy_assert(exp)
#else
#define capy_assert(exp) capy_assert_((exp) ? 1 : 0, __FILE__, __LINE__, #exp)
#endif

#endif
