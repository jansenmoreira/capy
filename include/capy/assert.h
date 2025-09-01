#ifndef CAPY_ASSERT_H
#define CAPY_ASSERT_H

#include <capy/std.h>

int capy_log_errno_(int err, const char *file, int line, const char *msg);
void capy_assert_(int condition, const char *file, int line, const char *expression);

#define capy_log_errno(err, msg) capy_log_errno_((err), __FILE__, __LINE__, (msg));

#ifdef NDEBUG
#define capy_assert(exp) (0)
#else
#define capy_assert(exp) capy_assert_((exp) ? 1 : 0, __FILE__, __LINE__, #exp)
#endif

#endif
