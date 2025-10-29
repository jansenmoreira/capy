#ifndef CAPY_MACROS_H
#define CAPY_MACROS_H

#include <capy/capy.h>

#ifdef __GNUC__
#define Format(i) __attribute__((format(printf, (i), (i) + 1)))
#define MustCheck __attribute__((warn_unused_result))
#define Unused __attribute__((unused))
#define Ignore (void)!
#define InOut
#define Out
#else
#define Format(i)
#define MustCheck
#define Unused
#define Ignore
#define InOut
#define Out
#endif

#define ArrLen(v) (sizeof(v) / sizeof(v[0]))
#define Arr(T, ...) ((T[]){__VA_ARGS__})

#define ArrCmp2(m, c0, c1) \
    ((m)[0] == (c0) && (m)[1] == (c1))

#define ArrCmp3(m, c0, c1, c2) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2))

#define ArrCmp4(m, c0, c1, c2, c3) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3))

#define ArrCmp5(m, c0, c1, c2, c3, c4) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3) && (m)[4] == (c4))

#define ArrCmp6(m, c0, c1, c2, c3, c4, c5) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3) && (m)[4] == (c4) && (m)[5] == (c5))

#define ArrCmp7(m, c0, c1, c2, c3, c4, c5, c6) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3) && (m)[4] == (c4) && (m)[5] == (c5) && (m)[6] == (c6))

#define StrIni(s) {.data = (s), .size = sizeof(s) - 1}
#define Str(s) ((capy_string)StrIni(s))

#define KiB(v) ((v) * 1024)
#define MiB(v) ((v) * 1024 * 1024)
#define GiB(v) ((v) * 1024 * 1024 * 1024)
#define TiB(v) ((v) * 1024 * 1024 * 1024 * 1024)

#define Seconds(v) ((v) * 1000)
#define Minutes(v) ((v) * 1000 * 60)
#define Hours(v) ((v) * 1000 * 60 * 60)
#define Days(v) ((v) * 1000 * 60 * 60 * 24)
#define Years(v) ((v) * 1000 * 60 * 60 * 24 * 365)

#define MicrosecondsNano(v) ((v) * 1000)
#define MillisecondsNano(v) ((v) * MicrosecondsNano(1000))
#define SecondsNano(v) ((v) * MillisecondsNano(1000))

#define Make(arena, T, size) \
    (capy_arena_alloc((arena), sizeof(T) * (size), alignof(T), true))

#define MakeNZ(arena, T, size) \
    (capy_arena_alloc((arena), sizeof(T) * (size), alignof(T), false))

#define Cast(T, v) ((T)(v))
#define ReinterpretCast(T, v) ((T)((char *)(v)))

#define ErrStd capy_err_errno
#define ErrWrap capy_err_wrap
#define ErrFmt capy_err_fmt
#define Ok ((capy_err){.code = 0})

#define LogMem(...) capy_log(CAPY_LOG_MEM, __VA_ARGS__)
#define LogDbg(...) capy_log(CAPY_LOG_DEBUG, __VA_ARGS__)
#define LogInf(...) capy_log(CAPY_LOG_INFO, __VA_ARGS__)
#define LogWrn(...) capy_log(CAPY_LOG_WARNING, __VA_ARGS__)
#define LogErr(...) capy_log(CAPY_LOG_ERROR, __VA_ARGS__)

#define Linux
#define LinuxAmd64
#define Platform

#endif
