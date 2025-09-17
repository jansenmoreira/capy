#ifndef CAPY_ERROR_H
#define CAPY_ERROR_H

#include <capy/std.h>

typedef struct capy_error_t
{
    int code;
    const char *msg;
} capy_err;

capy_err capy_errfmt(int code, const char *fmt, ...);

#endif
