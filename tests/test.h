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

#define Ref(T, v) (&(T[]){(v)})

struct point
{
    float x;
    float y;
    float z;
};

typedef struct string_pair
{
    capy_string key;
    capy_string value;
} string_pair;

static inline void expect_u_eq_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs == rhs) return;
    fprintf(stderr, "%s:%d: condition %llu == %llu failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_u_ne_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs != rhs) return;
    fprintf(stderr, "%s:%d: condition %llu != %llu failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_u_gt_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs > rhs) return;
    fprintf(stderr, "%s:%d: condition %llu > %llu failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_u_gte_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs >= rhs) return;
    fprintf(stderr, "%s:%d: condition %llu >= %llu failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_u_lt_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs < rhs) return;
    fprintf(stderr, "%s:%d: condition %llu < %llu failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_u_lte_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs <= rhs) return;
    fprintf(stderr, "%s:%d: condition %llu <= %llu failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_s_eq_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs == rhs) return;
    fprintf(stderr, "%s:%d: condition %lld == %lld failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_s_ne_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs != rhs) return;
    fprintf(stderr, "%s:%d: condition %lld != %lld failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_s_gt_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs > rhs) return;
    fprintf(stderr, "%s:%d: condition %lld > %lld failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_s_gte_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs >= rhs) return;
    fprintf(stderr, "%s:%d: condition %lld >= %lld failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_s_lt_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs < rhs) return;
    fprintf(stderr, "%s:%d: condition %lld < %lld failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_s_lte_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs <= rhs) return;
    fprintf(stderr, "%s:%d: condition %lld <= %lld failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_f_gt_(double lhs, double rhs, const char *file, int line)
{
    if (lhs > rhs) return;
    fprintf(stderr, "%s:%d: condition %lf > %lf failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_f_gte_(double lhs, double rhs, const char *file, int line)
{
    if (lhs >= rhs) return;
    fprintf(stderr, "%s:%d: condition %lf >= %lf failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_f_lt_(double lhs, double rhs, const char *file, int line)
{
    if (lhs < rhs) return;
    fprintf(stderr, "%s:%d: condition %lf < %lf failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_f_lte_(double lhs, double rhs, const char *file, int line)
{
    if (lhs <= rhs) return;
    fprintf(stderr, "%s:%d: condition %lf <= %lf failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_p_eq_(void *lhs, void *rhs, const char *file, int line)
{
    if (lhs == rhs) return;
    fprintf(stderr, "%s:%d: condition %p == %p failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_p_ne_(void *lhs, void *rhs, const char *file, int line)
{
    if (lhs != rhs) return;
    fprintf(stderr, "%s:%d: condition %p != %p failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_p_gt_(void *lhs, void *rhs, const char *file, int line)
{
    if (lhs > rhs) return;
    fprintf(stderr, "%s:%d: condition %p > %p failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_p_gte_(void *lhs, void *rhs, const char *file, int line)
{
    if (lhs >= rhs) return;
    fprintf(stderr, "%s:%d: condition %p >= %p failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_p_lt_(void *lhs, void *rhs, const char *file, int line)
{
    if (lhs < rhs) return;
    fprintf(stderr, "%s:%d: condition %p < %p failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_p_lte_(void *lhs, void *rhs, const char *file, int line)
{
    if (lhs <= rhs) return;
    fprintf(stderr, "%s:%d: condition %p <= %p failed\n", file, line, lhs, rhs);
    abort();
}

static inline void expect_str_eq_(capy_string lhs, capy_string rhs, const char *file, int line)
{
    if (capy_string_eq(lhs, rhs)) return;
    fprintf(stderr, "%s:%d: condition %.*s == %.*s failed\n", file, line, (int)lhs.size, lhs.data, (int)rhs.size, rhs.data);
    abort();
}

static inline void expect_str_ne_(capy_string lhs, capy_string rhs, const char *file, int line)
{
    if (!capy_string_eq(lhs, rhs)) return;
    fprintf(stderr, "%s:%d: condition %.*s != %.*s failed\n", file, line, (int)lhs.size, lhs.data, (int)rhs.size, rhs.data);
    abort();
}

#define expect_u_eq(lhs, rhs) expect_u_eq_((lhs), (rhs), __FILE__, __LINE__)
#define expect_u_ne(lhs, rhs) expect_u_ne_((lhs), (rhs), __FILE__, __LINE__)
#define expect_u_gt(lhs, rhs) expect_u_gt_((lhs), (rhs), __FILE__, __LINE__)
#define expect_u_gte(lhs, rhs) expect_u_gte_((lhs), (rhs), __FILE__, __LINE__)
#define expect_u_lt(lhs, rhs) expect_u_lt_((lhs), (rhs), __FILE__, __LINE__)
#define expect_u_lte(lhs, rhs) expect_u_lte_((lhs), (rhs), __FILE__, __LINE__)
#define expect_s_eq(lhs, rhs) expect_s_eq_((lhs), (rhs), __FILE__, __LINE__)
#define expect_s_ne(lhs, rhs) expect_s_ne_((lhs), (rhs), __FILE__, __LINE__)
#define expect_s_gt(lhs, rhs) expect_s_gt_((lhs), (rhs), __FILE__, __LINE__)
#define expect_s_gte(lhs, rhs) expect_s_gte_((lhs), (rhs), __FILE__, __LINE__)
#define expect_s_lt(lhs, rhs) expect_s_lt_((lhs), (rhs), __FILE__, __LINE__)
#define expect_s_lte(lhs, rhs) expect_s_lte_((lhs), (rhs), __FILE__, __LINE__)
#define expect_f_eq(lhs, rhs) expect_f_eq_((lhs), (rhs), __FILE__, __LINE__)
#define expect_f_ne(lhs, rhs) expect_f_ne_((lhs), (rhs), __FILE__, __LINE__)
#define expect_f_gt(lhs, rhs) expect_f_gt_((lhs), (rhs), __FILE__, __LINE__)
#define expect_f_gte(lhs, rhs) expect_f_gte_((lhs), (rhs), __FILE__, __LINE__)
#define expect_f_lt(lhs, rhs) expect_f_lt_((lhs), (rhs), __FILE__, __LINE__)
#define expect_f_lte(lhs, rhs) expect_f_lte_((lhs), (rhs), __FILE__, __LINE__)
#define expect_p_eq(lhs, rhs) expect_p_eq_((lhs), (rhs), __FILE__, __LINE__)
#define expect_p_ne(lhs, rhs) expect_p_ne_((lhs), (rhs), __FILE__, __LINE__)
#define expect_p_gt(lhs, rhs) expect_p_gt_((lhs), (rhs), __FILE__, __LINE__)
#define expect_p_gte(lhs, rhs) expect_p_gte_((lhs), (rhs), __FILE__, __LINE__)
#define expect_p_lt(lhs, rhs) expect_p_lt_((lhs), (rhs), __FILE__, __LINE__)
#define expect_p_lte(lhs, rhs) expect_p_lte_((lhs), (rhs), __FILE__, __LINE__)
#define expect_str_eq(lhs, rhs) expect_str_eq_((lhs), (rhs), __FILE__, __LINE__)
#define expect_str_ne(lhs, rhs) expect_str_ne_((lhs), (rhs), __FILE__, __LINE__)

#endif
