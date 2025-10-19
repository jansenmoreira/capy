#include <capy/test.h>

static int test_capy_error(void)
{
    capy_err err = {.code = 1, .msg = "test"};

    err = capy_err_fmt(2, "wrap1 (%s)", err.msg);
    ExpectEqS(err.code, 2);
    ExpectEqCstr("wrap1 (test)", err.msg);

    err = capy_err_fmt(3, "wrap2 (%s)", err.msg);
    ExpectEqS(err.code, 3);
    ExpectEqCstr("wrap2 (wrap1 (test))", err.msg);

    err = capy_err_fmt(4, "wrap3 (%s)", err.msg);
    ExpectEqS(err.code, 4);
    ExpectEqCstr("wrap3 (wrap2 (wrap1 (test)))", err.msg);

    return true;
}

static void test_error(testbench *t)
{
    runtest(t, test_capy_error, "capy_error");
}
