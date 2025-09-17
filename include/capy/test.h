#ifndef CAPY_TEST_H
#define CAPY_TEST_H

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

static inline int expect_u_eq_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " == %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_ne_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " != %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_gt_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " > %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_gte_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " >= %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_lt_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " < %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_u_lte_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " <= %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_eq_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " == %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_ne_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " != %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_gt_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " > %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_gte_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " >= %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_lt_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " < %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_s_lte_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " <= %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_f_gt_(double lhs, double rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %f > %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_f_gte_(double lhs, double rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %f >= %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_f_lt_(double lhs, double rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %f < %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_f_lte_(double lhs, double rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %f <= %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_p_eq_(uintptr_t lhs, uintptr_t rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " == %" PRIxPTR " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_p_ne_(uintptr_t lhs, uintptr_t rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " != %" PRIxPTR " failed\n", file, line, lhs, rhs);
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
#define expect_p_eq(lhs, rhs) { if (!expect_p_eq_(cast(uintptr_t, (lhs)), cast(uintptr_t, (rhs)), __FILE__, __LINE__)) return false; }
#define expect_p_ne(lhs, rhs) { if (!expect_p_ne_(cast(uintptr_t, (lhs)), cast(uintptr_t, (rhs)), __FILE__, __LINE__)) return false; }
#define expect_str_eq(lhs, rhs) { if (!expect_str_eq_((lhs), (rhs), __FILE__, __LINE__)) return false; }
#define expect_str_ne(lhs, rhs) { if (!expect_str_ne_((lhs), (rhs), __FILE__, __LINE__)) return false; }

#define assert_u_eq(lhs, rhs) { if (!expect_u_eq_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_u_ne(lhs, rhs) { if (!expect_u_ne_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_u_gt(lhs, rhs) { if (!expect_u_gt_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_u_gte(lhs, rhs) { if (!expect_u_gte_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_u_lt(lhs, rhs) { if (!expect_u_lt_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_u_lte(lhs, rhs) { if (!expect_u_lte_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_s_eq(lhs, rhs) { if (!expect_s_eq_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_s_ne(lhs, rhs) { if (!expect_s_ne_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_s_gt(lhs, rhs) { if (!expect_s_gt_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_s_gte(lhs, rhs) { if (!expect_s_gte_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_s_lt(lhs, rhs) { if (!expect_s_lt_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_s_lte(lhs, rhs) { if (!expect_s_lte_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_f_eq(lhs, rhs) { if (!expect_f_eq_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_f_ne(lhs, rhs) { if (!expect_f_ne_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_f_gt(lhs, rhs) { if (!expect_f_gt_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_f_gte(lhs, rhs) { if (!expect_f_gte_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_f_lt(lhs, rhs) { if (!expect_f_lt_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_f_lte(lhs, rhs) { if (!expect_f_lte_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_p_eq(lhs, rhs) { if (!expect_p_eq_(cast(void*, (lhs)), cast(void*, (rhs)), __FILE__, __LINE__)) abort(); }
#define assert_p_ne(lhs, rhs) { if (!expect_p_ne_(cast(void*, (lhs)), cast(void*, (rhs)), __FILE__, __LINE__)) abort(); }
#define assert_str_eq(lhs, rhs) { if (!expect_str_eq_((lhs), (rhs), __FILE__, __LINE__)) abort(); }
#define assert_str_ne(lhs, rhs) { if (!expect_str_ne_((lhs), (rhs), __FILE__, __LINE__)) abort(); }

// clang-format on

#endif
