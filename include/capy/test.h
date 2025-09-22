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

static inline int expect_ok_(capy_err err, const char *file, int line)
{
    if (err.code == 0) return true;
    fprintf(stderr, "%s:%d: expected 'ok' got an error '%d' \"%s\"\n", file, line, err.code, err.msg);
    return false;
}

static inline int expect_err_(capy_err err, const char *file, int line)
{
    if (err.code) return true;
    fprintf(stderr, "%s:%d: expected an error got 'ok'\n", file, line);
    return false;
}

static inline int expect_null_(uintptr_t ptr, const char *file, int line)
{
    if (!ptr) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " == NULL failed\n", file, line, ptr);
    return false;
}

static inline int expect_notnull_(uintptr_t ptr, const char *file, int line)
{
    if (ptr) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " != NULL failed\n", file, line, ptr);
    return false;
}

static inline int expect_true_(int64_t b, const char *file, int line)
{
    if (b) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " != %" PRId64 " failed\n", file, line, b, Cast(int64_t, 0));
    return false;
}

static inline int expect_false_(int64_t b, const char *file, int line)
{
    if (!b) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " == %" PRId64 " failed\n", file, line, b, Cast(int64_t, 0));
    return false;
}

static inline int expect_ueq_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " == %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_une_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " != %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_ugt_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " > %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_ugte_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " >= %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_ult_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " < %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_ulte_(uint64_t lhs, uint64_t rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIu64 " <= %" PRIu64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_seq_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " == %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_sne_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " != %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_sgt_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " > %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_sgte_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " >= %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_slt_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " < %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_slte_(int64_t lhs, int64_t rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRId64 " <= %" PRId64 " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_fgt_(double lhs, double rhs, const char *file, int line)
{
    if (lhs > rhs) return true;
    fprintf(stderr, "%s:%d: condition %f > %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_fgte_(double lhs, double rhs, const char *file, int line)
{
    if (lhs >= rhs) return true;
    fprintf(stderr, "%s:%d: condition %f >= %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_flt_(double lhs, double rhs, const char *file, int line)
{
    if (lhs < rhs) return true;
    fprintf(stderr, "%s:%d: condition %f < %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_flte_(double lhs, double rhs, const char *file, int line)
{
    if (lhs <= rhs) return true;
    fprintf(stderr, "%s:%d: condition %f <= %f failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_ptreq_(uintptr_t lhs, uintptr_t rhs, const char *file, int line)
{
    if (lhs == rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " == %" PRIxPTR " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_ptrne_(uintptr_t lhs, uintptr_t rhs, const char *file, int line)
{
    if (lhs != rhs) return true;
    fprintf(stderr, "%s:%d: condition %" PRIxPTR " != %" PRIxPTR " failed\n", file, line, lhs, rhs);
    return false;
}

static inline int expect_streq_(capy_string lhs, capy_string rhs, const char *file, int line)
{
    if (capy_string_eq(lhs, rhs)) return true;
    fprintf(stderr, "%s:%d: condition \"%.*s\" == \"%.*s\" failed\n", file, line, (int)lhs.size, lhs.data, (int)rhs.size, rhs.data);
    return false;
}

static inline int expect_strne_(capy_string lhs, capy_string rhs, const char *file, int line)
{
    if (!capy_string_eq(lhs, rhs)) return true;
    fprintf(stderr, "%s:%d: condition \"%.*s\" != \"%.*s\" failed\n", file, line, (int)lhs.size, lhs.data, (int)rhs.size, rhs.data);
    return false;
}

static inline int expect_memeq_(void *lhs, void *rhs, size_t size, const char *file, int line)
{
    if (memcmp(lhs, rhs, size) == 0) return true;
    fprintf(stderr, "%s:%d: condition memcmp(\"%p\", \"%p\", %zu) == 0 failed\n", file, line, lhs, rhs, size);
    return false;
}

static inline int expect_cstreq_(const char *lhs, const char *rhs, const char *file, int line)
{
    if (strcmp(lhs, rhs) == 0) return true;
    fprintf(stderr, "%s:%d: condition \"%s\" == \"%s\" failed\n", file, line, lhs, rhs);
    return false;
}

#define Expect(op, ...)                                    \
    do                                                     \
    {                                                      \
        if (!expect_##op(__VA_ARGS__, __FILE__, __LINE__)) \
        {                                                  \
            return false;                                  \
        }                                                  \
    } while (0)

#define Assert(op, ...)                                    \
    do                                                     \
    {                                                      \
        if (!expect_##op(__VA_ARGS__, __FILE__, __LINE__)) \
        {                                                  \
            abort();                                       \
        }                                                  \
    } while (0)

#define ExpectOk(err) Expect(ok_, (err))
#define ExpectErr(err) Expect(err_, (err))
#define ExpectNull(ptr) Expect(null_, Cast(uintptr_t, (ptr)))
#define ExpectNotNull(ptr) Expect(notnull_, Cast(uintptr_t, (ptr)))
#define ExpectTrue(v) Expect(true_, (v))
#define ExpectFalse(v) Expect(false_, (v))
#define ExpectEqMem(lhs, rhs, size) Expect(memeq_, (lhs), (rhs), (size))
#define ExpectEqCstr(lhs, rhs) Expect(cstreq_, (lhs), (rhs))
#define ExpectEqU(lhs, rhs) Expect(ueq_, (lhs), (rhs))
#define ExpectNeU(lhs, rhs) Expect(une_, (lhs), (rhs))
#define ExpectGtU(lhs, rhs) Expect(ugt_, (lhs), (rhs))
#define ExpectGteU(lhs, rhs) Expect(ugte_, (lhs), (rhs))
#define ExpectLtU(lhs, rhs) Expect(ult_, (lhs), (rhs))
#define ExpectLteU(lhs, rhs) Expect(ulte_, (lhs), (rhs))
#define ExpectEqS(lhs, rhs) Expect(seq_, (lhs), (rhs))
#define ExpectNeS(lhs, rhs) Expect(sne_, (lhs), (rhs))
#define ExpectGtS(lhs, rhs) Expect(sgt_, (lhs), (rhs))
#define ExpectGteS(lhs, rhs) Expect(sgte_, (lhs), (rhs))
#define ExpectLtS(lhs, rhs) Expect(slt_, (lhs), (rhs))
#define ExpectLteS(lhs, rhs) Expect(slte_, (lhs), (rhs))
#define ExpectEqF(lhs, rhs) Expect(feq_, (lhs), (rhs))
#define ExpectNeF(lhs, rhs) Expect(fne_, (lhs), (rhs))
#define ExpectGtF(lhs, rhs) Expect(fgt_, (lhs), (rhs))
#define ExpectGteF(lhs, rhs) Expect(fgte_, (lhs), (rhs))
#define ExpectLtF(lhs, rhs) Expect(flt_, (lhs), (rhs))
#define ExpectLteF(lhs, rhs) Expect(flte_, (lhs), (rhs))
#define ExpectEqPtr(lhs, rhs) Expect(ptreq_, Cast(uintptr_t, (lhs)), Cast(uintptr_t, (rhs)))
#define ExpectNePtr(lhs, rhs) Expect(ptrne_, Cast(uintptr_t, (lhs)), Cast(uintptr_t, (rhs)))
#define ExpectEqStr(lhs, rhs) Expect(streq_, (lhs), (rhs))
#define ExpectNeStr(lhs, rhs) Expect(strne_, (lhs), (rhs))

#define AssertOk(err) Assert(ok_, (err))
#define AssertErr(err) Assert(err_, (err))
#define AssertNull(ptr) Assert(null_, Cast(uintptr_t, (ptr)))
#define AssertNotNull(ptr) Assert(notnull_, Cast(uintptr_t, (ptr)))
#define AssertTrue(v) Assert(true_, (v))
#define AssertFalse(v) Assert(false_, (v))
#define AssertEqMem(lhs, rhs, size) Assert(memeq_, (lhs), (rhs), (size))
#define AssertEqCstr(lhs, rhs) Assert(cstreq_, (lhs), (rhs))
#define AssertEqU(lhs, rhs) Assert(ueq_, (lhs), (rhs))
#define AssertNeU(lhs, rhs) Assert(une_, (lhs), (rhs))
#define AssertGtU(lhs, rhs) Assert(ugt_, (lhs), (rhs))
#define AssertGteU(lhs, rhs) Assert(ugte_, (lhs), (rhs))
#define AssertLtU(lhs, rhs) Assert(ult_, (lhs), (rhs))
#define AssertLteU(lhs, rhs) Assert(ulte_, (lhs), (rhs))
#define AssertEqS(lhs, rhs) Assert(seq_, (lhs), (rhs))
#define AssertNeS(lhs, rhs) Assert(sne_, (lhs), (rhs))
#define AssertGtS(lhs, rhs) Assert(sgt_, (lhs), (rhs))
#define AssertGteS(lhs, rhs) Assert(sgte_, (lhs), (rhs))
#define AssertLtS(lhs, rhs) Assert(slt_, (lhs), (rhs))
#define AssertLteS(lhs, rhs) Assert(slte_, (lhs), (rhs))
#define AssertEqF(lhs, rhs) Assert(feq_, (lhs), (rhs))
#define AssertNeF(lhs, rhs) Assert(fne_, (lhs), (rhs))
#define AssertGtF(lhs, rhs) Assert(fgt_, (lhs), (rhs))
#define AssertGteF(lhs, rhs) Assert(fgte_, (lhs), (rhs))
#define AssertLtF(lhs, rhs) Assert(flt_, (lhs), (rhs))
#define AssertLteF(lhs, rhs) Assert(flte_, (lhs), (rhs))
#define AssertEqPtr(lhs, rhs) Assert(ptreq_, Cast(uintptr_t, (lhs)), Cast(uintptr_t, (rhs)))
#define AssertNePtr(lhs, rhs) Assert(ptrne_, Cast(uintptr_t, (lhs)), Cast(uintptr_t, (rhs)))
#define AssertEqStr(lhs, rhs) Assert(streq_, (lhs), (rhs))
#define AssertNeStr(lhs, rhs) Assert(strne_, (lhs), (rhs))

#endif
