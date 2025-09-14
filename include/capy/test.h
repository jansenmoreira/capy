#ifndef TEST_H
#define TEST_H

#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct testbench
{
    int succeded;
    int failed;
} testbench;

typedef int(test_definition)(void);

static inline void runtest(testbench *t, test_definition *test, const char *msg)
{
    if (test())
    {
        t->succeded += 1;
        printf("\x1b[32m\xE2\x9C\x93  %s\x1b[0m\n", msg);
    }
    else
    {
        t->failed += 1;
        printf("\x1b[31m\xE2\x9C\x97  %s\x1b[0m\n", msg);
    }
}

// STATIC INLINE DEFINITIONS

static inline int expect_u_eq_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %llu == %llu failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_ne_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %llu != %llu failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_gt_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %llu > %llu failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_gte_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %llu >= %llu failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_lt_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %llu < %llu failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_lte_(unsigned long long lhs, unsigned long long rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %llu <= %llu failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_eq_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %lld == %lld failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_ne_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %lld != %lld failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_gt_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %lld > %lld failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_gte_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %lld >= %lld failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_lt_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %lld < %lld failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_lte_(long long lhs, long long rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %lld <= %lld failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_f_gt_(double lhs, double rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %lf > %lf failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_f_gte_(double lhs, double rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %lf >= %lf failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_f_lt_(double lhs, double rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %lf < %lf failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_f_lte_(double lhs, double rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %lf <= %lf failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_p_eq_(const void *lhs, const void *rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %p == %p failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_p_ne_(const void *lhs, const void *rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %p != %p failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_p_gt_(const void *lhs, const void *rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %p > %p failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_p_gte_(const void *lhs, const void *rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %p >= %p failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_p_lt_(const void *lhs, const void *rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %p < %p failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_p_lte_(const void *lhs, const void *rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %p <= %p failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_str_eq_(capy_string lhs, capy_string rhs, const char *file, int line)
{
    if (capy_string_eq(lhs, rhs)) return true;
    fprintf(stderr, "%s:%d: condition \"%.*s\" == \"%.*s\" failed\n", file, line, (int)lhs.size, lhs.data, (int)rhs.size, rhs.data);
    return false;
}

static inline int expect_str_ne_(capy_string lhs, capy_string rhs, const char *file, int line)
{
    if (!capy_string_eq(lhs, rhs)) return true;
    fprintf(stderr, "%s:%d: condition \"%.*s\" != \"%.*s\" failed\n", file, line, (int)lhs.size, lhs.data, (int)rhs.size, rhs.data);
    return false;
}

// MACROS

// clang-format off

#define expect_u_eq(lhs, rhs) { if (!expect_u_eq_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_u_ne(lhs, rhs) { if (!expect_u_ne_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_u_gt(lhs, rhs) { if (!expect_u_gt_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_u_gte(lhs, rhs) { if (!expect_u_gte_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_u_lt(lhs, rhs) { if (!expect_u_lt_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_u_lte(lhs, rhs) { if (!expect_u_lte_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_s_eq(lhs, rhs) { if (!expect_s_eq_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_s_ne(lhs, rhs) { if (!expect_s_ne_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_s_gt(lhs, rhs) { if (!expect_s_gt_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_s_gte(lhs, rhs) { if (!expect_s_gte_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_s_lt(lhs, rhs) { if (!expect_s_lt_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_s_lte(lhs, rhs) { if (!expect_s_lte_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_f_eq(lhs, rhs) { if (!expect_f_eq_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_f_ne(lhs, rhs) { if (!expect_f_ne_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_f_gt(lhs, rhs) { if (!expect_f_gt_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_f_gte(lhs, rhs) { if (!expect_f_gte_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_f_lt(lhs, rhs) { if (!expect_f_lt_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_f_lte(lhs, rhs) { if (!expect_f_lte_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_p_eq(lhs, rhs) { if (!expect_p_eq_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_p_ne(lhs, rhs) { if (!expect_p_ne_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_p_gt(lhs, rhs) { if (!expect_p_gt_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_p_gte(lhs, rhs) { if (!expect_p_gte_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_p_lt(lhs, rhs) { if (!expect_p_lt_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_p_lte(lhs, rhs) { if (!expect_p_lte_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_str_eq(lhs, rhs) { if (!expect_str_eq_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_str_ne(lhs, rhs) { if (!expect_str_ne_((lhs), (rhs), __FILE__, __LINE__)) return false; }

// clang-format on

#endif
