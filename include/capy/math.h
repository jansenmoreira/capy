#ifndef CAPY_MATH_H
#define CAPY_MATH_H

#include <capy/std.h>
#include <limits.h>
#include <time.h>

size_t align_to(size_t v, size_t n);
size_t next_pow2(size_t v);
int64_t timespec_diff(struct timespec a, struct timespec b);
void nanoseconds_normalize(int64_t *ns, const char **unit);

#endif
