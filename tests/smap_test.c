
#include <assert.h>
#include <capy/capy.h>
#include <stdio.h>

struct point
{
    float x;
    float y;
    float z;
};

typedef struct point point;
capy_smap_define(point);

struct capy_smap_pair_point tests[] = {
    {"a", {1.0f}},
    {"b", {2.0f}},
    {"c", {3.0f}},
    {"d", {4.0f}},
    {"e", {5.0f}},
    {"f", {6.0f}},
    {"g", {7.0f}},
    {"h", {8.0f}},
    {"i", {9.0f}},
    {"j", {10.0f}},
    {"k", {11.0f}},
    {"l", {12.0f}},
    {"m", {13.0f}},
    {"n", {14.0f}},
    {"o", {15.0f}},
    {"p", {16.0f}},
    {"q", {17.0f}},
    {"r", {18.0f}},
    {"s", {19.0f}},
    {"t", {20.0f}},
    {"u", {21.0f}},
    {"v", {22.0f}},
    {"w", {23.0f}},
    {"x", {24.0f}},
    {"y", {25.0f}},
    {"z", {26.0f}},
};

int main()
{
    struct capy_smap_pair_point pair;

    union capy_smap_point smap1 = {NULL};
    union capy_sset sset1 = {NULL};

    for (int i = 0; i < 11; i++)
    {
        capy_smap_set_point(&smap1, tests[i].key, tests[i].value);
        capy_sset_set(&sset1, tests[i].key);
    }

    for (int i = 0; i < 11; i++)
    {
        pair = capy_smap_get_point(&smap1, tests[i].key);
        assert(strcmp(pair.key, tests[i].key) == 0);
        assert(pair.value.x == tests[i].value.x);
    }

    for (int i = 0; i < 11; i++)
    {
        const char *value = capy_sset_get(&sset1, tests[i].key);
        assert(strcmp(value, tests[i].key) == 0);
    }

    pair = capy_smap_get_point(&smap1, "l");
    assert(pair.key == NULL);
    assert(capy_sset_get(&sset1, "l") == NULL);

    capy_smap_delete_point(&smap1, "k");
    capy_sset_delete(&sset1, "k");
    capy_smap_delete_point(&smap1, "0");
    capy_sset_delete(&sset1, "0");

    for (int i = 11; i < 26; i++)
    {
        capy_smap_set_point(&smap1, tests[i].key, tests[i].value);
        capy_sset_set(&sset1, tests[i].key);
    }

    pair = capy_smap_get_point(&smap1, "k");
    assert(pair.key == NULL);
    assert(capy_sset_get(&sset1, "k") == NULL);

    for (struct capy_smap_pair_point *it = capy_smap_next_point(&smap1, NULL);
         it != NULL;
         it = capy_smap_next_point(&smap1, it))
    {
        printf("%s -> %f, ", it->key, it->value.x);
    }

    puts("");

    for (const char **it = capy_sset_next(&sset1, NULL);
         it != NULL;
         it = capy_sset_next(&sset1, it))
    {
        printf("%s, ", *it);
    }

    puts("");

    printf("%d, %d, %d\n", sizeof(const char *), sizeof(point), sizeof(struct capy_smap_pair_point));

    return 0;
}
