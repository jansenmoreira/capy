#include "test.h"

int test_vec(void)
{
    capy_arena *arena = capy_arena_init(GiB(8ULL));

    uint16_t *vec1 = capy_vec_init_u16(arena, 8, CAPY_VEC_REALLOC);
    float *vec2 = capy_vec_init_f32(arena, 8, CAPY_VEC_REALLOC);
    uint16_t *vec3 = capy_vec_init_u16(arena, 8, CAPY_VEC_REALLOC);

    int a = capy_vec_reserve_u16(&vec1, 40);
    capy_vec_resize_u16(&vec1, 11);
    capy_vec_resize_u16(&vec3, 11);

    vec1[0] = 0;
    vec3[0] = 0;

    for (uint16_t i = 1; i <= 10; i++)
    {
        vec1[i] = i * 10;
        vec3[i] = i;
        capy_vec_push_f32(&vec2, i * i);
    }

    capy_vec_resize_f32(&vec2, capy_vec_size(vec2) - 1);

    int limit = capy_vec_size(vec2);
    for (int i = 0; i < limit; i++)
    {
        float last = vec2[capy_vec_size(vec2) - 1];
        capy_vec_pop_f32(&vec2);
        printf("%.2f\n", last);
    }

    for (int i = 0; i < capy_vec_size(vec1); i++)
    {
        printf("%d\n", vec1[i]);
    }

    capy_vec_insertvals_u16(&vec1, 5, 3, NULL);
    capy_vec_insertvals_u16(&vec1, 14, 2, NULL);

    for (int i = 5; i < 5 + 3; i++)
    {
        vec1[i] = i * 1000;
    }

    capy_vec_insertvals_u16(&vec3, 5, capy_vec_size(vec1), vec1);

    uint16_t t = 1234;
    capy_vec_insertvals_u16(&vec3, 0, 1, &t);
    capy_vec_insert_u16(&vec3, 20, 888);
    capy_vec_insert_u16(&vec3, 20, 999);

    capy_vec_delete_u16(&vec3, 6, 18);

    return 0;
}
