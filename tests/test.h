#ifndef TEST_H
#define TEST_H

#include <capy/capy.h>
#include <errno.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define expect(exp) \
    ((exp) ? 0 : (fprintf(stderr, "%s:%d: expected condition `%s` failed\n", __FILE__, __LINE__, #exp), abort(), 0))

#define KiB(v) ((v) * 1024)
#define MiB(v) (KiB(v) * 1024)
#define GiB(v) (MiB(v) * 1024)
#define TiB(v) (TiB(v) * 1024)

capy_vec_define(u16, uint16_t);
capy_vec_define(f32, float);

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

#endif
