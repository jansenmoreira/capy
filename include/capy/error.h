#ifndef CAPY_ERROR_H
#define CAPY_ERROR_H

#include <capy/std.h>

typedef struct capy_err
{
    int code;
    const char *msg;
} capy_err;

capy_err capy_errfmt(int code, const char *fmt, ...);
capy_err capy_errno(int err);
capy_err capy_errwrap(capy_err err, const char *msg);

#endif
