#include <capy/assert.h>

int capy_log_errno_(int err, const char *file, int line, const char *msg)
{
    fprintf(stderr, "[%s:%d] => %s: %s\n", file, line, msg, strerror(err));
    return err;
}

void capy_assert_(int condition, const char *file, int line, const char *expression)
{
    if (!condition)
    {
        fprintf(stderr, "[%s:%d] => expected condition `%s` failed\n", file, line, expression);
        abort();
    }
}
