#include <capy/macros.h>
#include <capy/test.h>

static int test_capy_error(void)
{
    capy_err err = {.code = 1, .msg = "test"};

    err = capy_errfmt(2, "wrap1 (%s)", err.msg);
    expect_s_eq(err.code, 2);
    expect_s_eq(strcmp("wrap1 (test)", err.msg), 0);

    err = capy_errfmt(3, "wrap2 (%s)", err.msg);
    expect_s_eq(err.code, 3);
    expect_s_eq(strcmp("wrap2 (wrap1 (test))", err.msg), 0);

    err = capy_errfmt(4, "wrap3 (%s)", err.msg);
    expect_s_eq(err.code, 4);
    expect_s_eq(strcmp("wrap3 (wrap2 (wrap1 (test)))", err.msg), 0);

    return true;
}

static void test_error(testbench *t)
{
    runtest(t, test_capy_error, "capy_error");
}
