#include "capy.h"

// PUBLIC DEFINITIONS

void capy_assert_(int condition, const char *file, int line, const char *expression)
{
    if (!condition)
    {
        fprintf(stderr, "[%s:%d] => expected condition `%s` failed\n", file, line, expression);
        abort();
    }
}
