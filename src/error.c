#include <capy/assert.h>
#include <capy/error.h>
#include <stdarg.h>
#include <threads.h>

#define CAPY_EBUFSIZE 256

capy_err capy_errfmt(int code, const char *fmt, ...)
{
    thread_local static char buffer1[CAPY_EBUFSIZE];
    thread_local static char buffer2[CAPY_EBUFSIZE];
    thread_local static int active_buffer = 1;

    char *buffer;

    if (active_buffer == 1)
    {
        active_buffer = 2;
        buffer = buffer1;
    }
    else
    {
        active_buffer = 1;
        buffer = buffer2;
    }

    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buffer, CAPY_EBUFSIZE, fmt, args);
    va_end(args);
    capy_assert(n >= 0);

    return (capy_err){.code = code, .msg = buffer};
}
