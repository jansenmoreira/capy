#ifndef CAPY_TEST_H
#define CAPY_TEST_H

#include <capy/capy.h>
#include <capy/macros.h>

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

static inline int __expect_ok(capy_err err, const char *file, int line)
{
    if (err.code == 0) return true;
    fprintf(stderr, "%s:%d: expected 'ok' got an error '%d' \"%s\"\n", file, line, err.code, err.msg);
    return false;
}

static inline int __expect_err(capy_err err, const char *file, int line)
{
    if (err.code) return true;
    fprintf(stderr, "%s:%d: expected an error got 'ok'\n", file, line);
    return false;
}

static inline int __expect_null(uintptr_t ptr, const char *file, int line)
{
    if (!ptr) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " == NULL failed\n", file, line, ptr);
    return false;
}

static inline int __expect_notnull(uintptr_t ptr, const char *file, int line)
{
    if (ptr) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " != NULL failed\n", file, line, ptr);
    return false;
}

static inline int __expect_true(int64_t b, const char *file, int line)
{
    if (b) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " != %" PRId64 " failed\n", file, line, b, Cast(int64_t, 0));
    return false;
}

static inline int __expect_false(int64_t b, const char *file, int line)
{
    if (!b) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " == %" PRId64 " failed\n", file, line, b, Cast(int64_t, 0));
    return false;
}

static inline int __expect_ueq(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " == %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_une(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " != %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_ugt(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " > %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_ugte(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " >= %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_ult(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " < %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_ulte(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " <= %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_seq(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " == %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_sne(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " != %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_sgt(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " > %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_sgte(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " >= %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_slt(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " < %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_slte(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " <= %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_fgt(double lhs, double rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %f > %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_fgte(double lhs, double rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %f >= %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_flt(double lhs, double rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %f < %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_flte(double lhs, double rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %f <= %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_ptreq(uintptr_t lhs, uintptr_t rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " == %" PRIxPTR " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_ptrne(uintptr_t lhs, uintptr_t rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " != %" PRIxPTR " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int __expect_streq(capy_string lhs, capy_string rhs, const char *file, int line)
{
    if (capy_string_eq(lhs, rhs)) return true;
    fprintf(stderr, "%s:%d: condition \"%.*s\" == \"%.*s\" failed\n", file, line, (int)lhs.size, lhs.data, (int)rhs.size, rhs.data);
    return false;
}

static inline int __expect_strne(capy_string lhs, capy_string rhs, const char *file, int line)
{
    if (!capy_string_eq(lhs, rhs)) return true;
    fprintf(stderr, "%s:%d: condition \"%.*s\" != \"%.*s\" failed\n", file, line, (int)lhs.size, lhs.data, (int)rhs.size, rhs.data);
    return false;
}

static inline int __expect_memeq(void *lhs, void *rhs, size_t size, const char *file, int line)
{
    if (memcmp(lhs, rhs, size) == 0) return true;
    fprintf(stderr, "%s:%d: condition memcmp(\"%p\", \"%p\", %zu) == 0 failed\n", file, line, lhs, rhs, size);
    return false;
}

static inline int __expect_cstreq(const char *lhs, const char *rhs, const char *file, int line)
{
    if (strcmp(lhs, rhs) == 0) return true;
    fprintf(stderr, "%s:%d: condition \"%s\" == \"%s\" failed\n", file, line, lhs, rhs);
    return false;
}

#define Expect(op, ...)                                      \
    do                                                       \
    {                                                        \
        if (!__expect_##op(__VA_ARGS__, __FILE__, __LINE__)) \
        {                                                    \
            return false;                                    \
        }                                                    \
    } while (0)

#define Assert(op, ...)                                      \
    do                                                       \
    {                                                        \
        if (!__expect_##op(__VA_ARGS__, __FILE__, __LINE__)) \
        {                                                    \
            abort();                                         \
        }                                                    \
    } while (0)

#define ExpectOk(v) Expect(ok, (v))
#define ExpectErr(v) Expect(err, (v))
#define ExpectNull(v) Expect(null, Cast(uintptr_t, (v)))
#define ExpectNotNull(v) Expect(notnull, Cast(uintptr_t, (v)))
#define ExpectTrue(v) Expect(true, (v))
#define ExpectFalse(v) Expect(false, (v))
#define ExpectEqMem(lhs, rhs, size) Expect(memeq, (lhs), (rhs), (size))
#define ExpectEqCstr(lhs, rhs) Expect(cstreq, (lhs), (rhs))
#define ExpectEqU(lhs, rhs) Expect(ueq, (lhs), (rhs))
#define ExpectNeU(lhs, rhs) Expect(une, (lhs), (rhs))
#define ExpectGtU(lhs, rhs) Expect(ugt, (lhs), (rhs))
#define ExpectGteU(lhs, rhs) Expect(ugte, (lhs), (rhs))
#define ExpectLtU(lhs, rhs) Expect(ult, (lhs), (rhs))
#define ExpectLteU(lhs, rhs) Expect(ulte, (lhs), (rhs))
#define ExpectEqS(lhs, rhs) Expect(seq, (lhs), (rhs))
#define ExpectNeS(lhs, rhs) Expect(sne, (lhs), (rhs))
#define ExpectGtS(lhs, rhs) Expect(sgt, (lhs), (rhs))
#define ExpectGteS(lhs, rhs) Expect(sgte, (lhs), (rhs))
#define ExpectLtS(lhs, rhs) Expect(slt, (lhs), (rhs))
#define ExpectLteS(lhs, rhs) Expect(slte, (lhs), (rhs))
#define ExpectEqF(lhs, rhs) Expect(feq, (lhs), (rhs))
#define ExpectNeF(lhs, rhs) Expect(fne, (lhs), (rhs))
#define ExpectGtF(lhs, rhs) Expect(fgt, (lhs), (rhs))
#define ExpectGteF(lhs, rhs) Expect(fgte, (lhs), (rhs))
#define ExpectLtF(lhs, rhs) Expect(flt, (lhs), (rhs))
#define ExpectLteF(lhs, rhs) Expect(flte, (lhs), (rhs))
#define ExpectEqPtr(lhs, rhs) Expect(ptreq, Cast(uintptr_t, (lhs)), Cast(uintptr_t, (rhs)))
#define ExpectNePtr(lhs, rhs) Expect(ptrne, Cast(uintptr_t, (lhs)), Cast(uintptr_t, (rhs)))
#define ExpectEqStr(lhs, rhs) Expect(streq, (lhs), (rhs))
#define ExpectNeStr(lhs, rhs) Expect(strne, (lhs), (rhs))

#define AssertOk(v) Assert(ok, (v))
#define AssertErr(v) Assert(err, (v))
#define AssertNull(v) Assert(null, Cast(uintptr_t, (v)))
#define AssertNotNull(v) Assert(notnull, Cast(uintptr_t, (v)))
#define AssertTrue(v) Assert(true, (v))
#define AssertFalse(v) Assert(false, (v))
#define AssertEqMem(lhs, rhs, size) Assert(memeq, (lhs), (rhs), (size))
#define AssertEqCstr(lhs, rhs) Assert(cstreq, (lhs), (rhs))
#define AssertEqU(lhs, rhs) Assert(ueq, (lhs), (rhs))
#define AssertNeU(lhs, rhs) Assert(une, (lhs), (rhs))
#define AssertGtU(lhs, rhs) Assert(ugt, (lhs), (rhs))
#define AssertGteU(lhs, rhs) Assert(ugte, (lhs), (rhs))
#define AssertLtU(lhs, rhs) Assert(ult, (lhs), (rhs))
#define AssertLteU(lhs, rhs) Assert(ulte, (lhs), (rhs))
#define AssertEqS(lhs, rhs) Assert(seq, (lhs), (rhs))
#define AssertNeS(lhs, rhs) Assert(sne, (lhs), (rhs))
#define AssertGtS(lhs, rhs) Assert(sgt, (lhs), (rhs))
#define AssertGteS(lhs, rhs) Assert(sgte, (lhs), (rhs))
#define AssertLtS(lhs, rhs) Assert(slt, (lhs), (rhs))
#define AssertLteS(lhs, rhs) Assert(slte, (lhs), (rhs))
#define AssertEqF(lhs, rhs) Assert(feq, (lhs), (rhs))
#define AssertNeF(lhs, rhs) Assert(fne, (lhs), (rhs))
#define AssertGtF(lhs, rhs) Assert(fgt, (lhs), (rhs))
#define AssertGteF(lhs, rhs) Assert(fgte, (lhs), (rhs))
#define AssertLtF(lhs, rhs) Assert(flt, (lhs), (rhs))
#define AssertLteF(lhs, rhs) Assert(flte, (lhs), (rhs))
#define AssertEqPtr(lhs, rhs) Assert(ptreq, Cast(uintptr_t, (lhs)), Cast(uintptr_t, (rhs)))
#define AssertNePtr(lhs, rhs) Assert(ptrne, Cast(uintptr_t, (lhs)), Cast(uintptr_t, (rhs)))
#define AssertEqStr(lhs, rhs) Assert(streq, (lhs), (rhs))
#define AssertNeStr(lhs, rhs) Assert(strne, (lhs), (rhs))

#endif
