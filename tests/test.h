#ifndef TEST_H
#define TEST_H

#include <capy/capy.h>
#include <errno.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define arrlen(v) (sizeof(v) / sizeof(v[0]))

#define KiB(v) ((v) * 1024)
#define MiB(v) (KiB(v) * 1024)
#define GiB(v) (MiB(v) * 1024)
#define TiB(v) (GiB(v) * 1024)

#define str(s) capy_string_literal(s)

struct capy_arena
{
    ptrdiff_t size;
    ptrdiff_t capacity;
    ptrdiff_t limit;
    ptrdiff_t page_size;
};

struct point
{
    float x;
    float y;
    float z;
};

struct smap_pair_point
{
    const char *key;
    struct point value;
};

capy_smap_define(point, struct smap_pair_point)

#define expect(exp, format, file, line, lhs, rhs) \
    ((exp) ? 0 : (fprintf(stderr, (format), (file), (line), (lhs), (rhs)), abort(), 0))

#define expect_u_eq(lhs, rhs) expect((unsigned long long)(lhs) == (unsigned long long)(rhs), "%s:%d: condition %llu == %llu failed\n", __FILE__, __LINE__, (unsigned long long)(lhs), (unsigned long long)(rhs))
#define expect_u_ne(lhs, rhs) expect((unsigned long long)(lhs) != (unsigned long long)(rhs), "%s:%d: condition %llu != %llu failed\n", __FILE__, __LINE__, (unsigned long long)(lhs), (unsigned long long)(rhs))
#define expect_u_gt(lhs, rhs) expect((unsigned long long)(lhs) > (unsigned long long)(rhs), "%s:%d: condition %llu > %llu failed\n", __FILE__, __LINE__, (unsigned long long)(lhs), (unsigned long long)(rhs))
#define expect_u_gte(lhs, rhs) expect((unsigned long long)(lhs) >= (unsigned long long)(rhs), "%s:%d: condition %llu >= %llu failed\n", __FILE__, __LINE__, (unsigned long long)(lhs), (unsigned long long)(rhs))
#define expect_u_lt(lhs, rhs) expect((unsigned long long)(lhs) < (unsigned long long)(rhs), "%s:%d: condition %llu < %llu failed\n", __FILE__, __LINE__, (unsigned long long)(lhs), (unsigned long long)(rhs))
#define expect_u_lte(lhs, rhs) expect((unsigned long long)(lhs) <= (unsigned long long)(rhs), "%s:%d: condition %llu <= %llu failed\n", __FILE__, __LINE__, (unsigned long long)(lhs), (unsigned long long)(rhs))

#define expect_s_eq(lhs, rhs) expect((long long)(lhs) == (long long)(rhs), "%s:%d: condition %lld == %lld failed\n", __FILE__, __LINE__, (long long)(lhs), (long long)(rhs))
#define expect_s_ne(lhs, rhs) expect((long long)(lhs) != (long long)(rhs), "%s:%d: condition %lld != %lld failed\n", __FILE__, __LINE__, (long long)(lhs), (long long)(rhs))
#define expect_s_gt(lhs, rhs) expect((long long)(lhs) > (long long)(rhs), "%s:%d: condition %lld > %lld failed\n", __FILE__, __LINE__, (long long)(lhs), (long long)(rhs))
#define expect_s_gte(lhs, rhs) expect((long long)(lhs) >= (long long)(rhs), "%s:%d: condition %lld >= %lld failed\n", __FILE__, __LINE__, (long long)(lhs), (long long)(rhs))
#define expect_s_lt(lhs, rhs) expect((long long)(lhs) < (long long)(rhs), "%s:%d: condition %lld < %lld failed\n", __FILE__, __LINE__, (long long)(lhs), (long long)(rhs))
#define expect_s_lte(lhs, rhs) expect((long long)(lhs) <= (long long)(rhs), "%s:%d: condition %lld <= %lld failed\n", __FILE__, __LINE__, (long long)(lhs), (long long)(rhs))

#define expect_f_eq(lhs, rhs) expect((double)(lhs) == (double)(rhs), "%s:%d: condition %f == %f failed\n", __FILE__, __LINE__, (double)(lhs), (double)(rhs))
#define expect_f_ne(lhs, rhs) expect((double)(lhs) != (double)(rhs), "%s:%d: condition %f != %f failed\n", __FILE__, __LINE__, (double)(lhs), (double)(rhs))
#define expect_f_gt(lhs, rhs) expect((double)(lhs) > (double)(rhs), "%s:%d: condition %f > %f failed\n", __FILE__, __LINE__, (double)(lhs), (double)(rhs))
#define expect_f_gte(lhs, rhs) expect((double)(lhs) >= (double)(rhs), "%s:%d: condition %f >= %f failed\n", __FILE__, __LINE__, (double)(lhs), (double)(rhs))
#define expect_f_lt(lhs, rhs) expect((double)(lhs) < (double)(rhs), "%s:%d: condition %f < %f failed\n", __FILE__, __LINE__, (double)(lhs), (double)(rhs))
#define expect_f_lte(lhs, rhs) expect((double)(lhs) <= (double)(rhs), "%s:%d: condition %f <= %f failed\n", __FILE__, __LINE__, (double)(lhs), (double)(rhs))

#define expect_p_eq(lhs, rhs) expect((void *)(lhs) == (void *)(rhs), "%s:%d: condition %p == %p failed\n", __FILE__, __LINE__, (void *)(lhs), (void *)(rhs))
#define expect_p_ne(lhs, rhs) expect((void *)(lhs) != (void *)(rhs), "%s:%d: condition %p != %p failed\n", __FILE__, __LINE__, (void *)(lhs), (void *)(rhs))
#define expect_p_gt(lhs, rhs) expect((void *)(lhs) > (void *)(rhs), "%s:%d: condition %p > %p failed\n", __FILE__, __LINE__, (void *)(lhs), (void *)(rhs))
#define expect_p_gte(lhs, rhs) expect((void *)(lhs) >= (void *)(rhs), "%s:%d: condition %p >= %p failed\n", __FILE__, __LINE__, (void *)(lhs), (void *)(rhs))
#define expect_p_lt(lhs, rhs) expect((void *)(lhs) < (void *)(rhs), "%s:%d: condition %p < %p failed\n", __FILE__, __LINE__, (void *)(lhs), (void *)(rhs))
#define expect_p_lte(lhs, rhs) expect((void *)(lhs) <= (void *)(rhs), "%s:%d: condition %p <= %p failed\n", __FILE__, __LINE__, (void *)(lhs), (void *)(rhs))

#define expect_str_eq(lhs, rhs) \
    ((capy_string_eq((lhs), (rhs))) ? 0 : (fprintf(stderr, "%s:%d: condition \"%.*s\" == \"%.*s\" failed\n", __FILE__, __LINE__, (int)(lhs).size, (lhs).data, (int)(rhs).size, (rhs).data), abort(), 0))

#define expect_str_ne(lhs, rhs) \
    ((!capy_string_eq((lhs), (rhs))) ? 0 : (fprintf(stderr, "%s:%d: condition \"%.*s\" != \"%.*s\" failed\n", __FILE__, __LINE__, (int)(lhs).size, (lhs).data, (int)(rhs).size, (rhs).data), abort(), 0))

#endif
