#include <capy/capy.h>
#include <capy/macros.h>

#define CAPY_EBUFSIZE 512

static char *capy_err_buf(void)
{
    thread_local static char buffer1[CAPY_EBUFSIZE];
    thread_local static char buffer2[CAPY_EBUFSIZE];
    thread_local static int active_buffer = 1;

    if (active_buffer == 1)
    {
        active_buffer = 2;
        return buffer1;
    }
    else
    {
        active_buffer = 1;
        return buffer2;
    }
}

capy_err capy_err_fmt(int code, const char *fmt, ...)
{
    char *buffer = capy_err_buf();

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, CAPY_EBUFSIZE, fmt, args);
    va_end(args);

    return (capy_err){.code = code, .msg = buffer};
}

capy_err capy_err_wrap(capy_err err, const char *msg)
{
    return capy_err_fmt(err.code, "%s: %s", msg, err.msg);
}
