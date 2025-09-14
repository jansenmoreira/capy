#ifndef CAPY_HELPERS_H
#define CAPY_HELPERS_H

#include <capy/capy.h>

#define arrlen(v) (sizeof(v) / sizeof(v[0]))
#define arr(T, ...) ((T[]){__VA_ARGS__})

#define arrcmp2(m, c0, c1) \
    ((m)[0] == (c0) && (m)[1] == (c1))

#define arrcmp3(m, c0, c1, c2) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2))

#define arrcmp4(m, c0, c1, c2, c3) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3))

#define arrcmp5(m, c0, c1, c2, c3, c4) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3) && (m)[4] == (c4))

#define arrcmp6(m, c0, c1, c2, c3, c4, c5) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3) && (m)[4] == (c4) && (m)[5] == (c5))

#define arrcmp7(m, c0, c1, c2, c3, c4, c5, c6) \
    ((m)[0] == (c0) && (m)[1] == (c1) && (m)[2] == (c2) && (m)[3] == (c3) && (m)[4] == (c4) && (m)[5] == (c5) && (m)[6] == (c6))

#define strli(s) {.data = (s), .size = sizeof(s) - 1}
#define strl(s) ((capy_string)strli(s))

#define KiB(v) ((v) * 1024)
#define MiB(v) (KiB(v) * 1024)
#define GiB(v) (MiB(v) * 1024)
#define TiB(v) (GiB(v) * 1024)

#define make(arena, T, size) \
    (capy_arena_alloc((arena), sizeof(T) * (size), alignof(T), true))

#define umake(arena, T, size) \
    (capy_arena_alloc((arena), sizeof(T) * (size), alignof(T), false))

#define cast(T, v) ((T)(v))
#define recast(T, v) ((T)((void *)(v)))

#endif
