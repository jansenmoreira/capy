
#include <capy/capy.h>
#include <stdio.h>

capy_vec_define(uint16_t);
capy_vec_define(float);

int main()
{
    union capy_vec_uint16_t vec1 = {NULL};
    union capy_vec_float vec2 = {NULL};
    union capy_vec_uint16_t vec3 = {NULL};

    capy_vec_reserve_uint16_t(&vec3, 40);

    capy_vec_resize_uint16_t(&vec1, 11);
    capy_vec_resize_uint16_t(&vec3, 11);
    vec1.data[0] = 0;
    vec3.data[0] = 0;

    for (uint16_t i = 1; i <= 10; i++)
    {
        vec1.data[i] = i * 10;
        vec3.data[i] = i;
        capy_vec_push_float(&vec2, i * i);
    }

    capy_vec_resize_float(&vec2, vec2.size - 1);

    int limit = vec2.size;
    for (int i = 0; i < limit; i++)
    {
        float last = vec2.data[vec2.size - 1];
        capy_vec_pop_float(&vec2);
        printf("%.2f\n", last);
    }

    for (int i = 0; i < vec1.size; i++)
    {
        printf("%d\n", vec1.data[i]);
    }

    capy_vec_insert_at_uint16_t(&vec1, 5, 3, NULL);
    capy_vec_insert_at_uint16_t(&vec1, 14, 2, NULL);

    for (int i = 5; i < 5 + 3; i++)
    {
        vec1.data[i] = i * 1000;
    }

    capy_vec_insert_at_uint16_t(&vec3, 5, vec1.size, vec1.data);

    uint16_t t = 1234;
    capy_vec_insert_at_uint16_t(&vec3, 0, 1, &t);
    capy_vec_insert_value_at_uint16_t(&vec3, 20, 888);
    capy_vec_insert_value_at_uint16_t(&vec3, 20, 999);

    capy_vec_delete_at_uint16_t(&vec3, 6, 18);

    free(vec1.data);
    free(vec2.data);

    return 0;
}
