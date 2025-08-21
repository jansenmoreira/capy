#include <capy/capy.h>
#include <stdlib.h>

const char *capy_symbols_add(const char ***pool, const char *str)
{
    const char *entry = capy_sset_get(pool, str);

    if (entry != NULL)
    {
        return entry;
    }

    ptrdiff_t length = strlen(str) + 1;

    void *buffer = malloc(length);

    if (buffer == NULL)
    {
        return NULL;
    }

    memcpy(buffer, str, length);

    int err = capy_sset_set(pool, buffer);

    if (err)
    {
        return NULL;
    }

    return buffer;
}

void capy_symbols_free(const char ***pool)
{
    for (const char **it = capy_sset_next(*pool, NULL);
         it != NULL;
         it = capy_sset_next(*pool, it))
    {
        free((void *)it[0]);
    }

    free(capy_smap_head(*pool));

    *pool = NULL;
}
