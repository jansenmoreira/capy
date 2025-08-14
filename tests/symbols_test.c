
#include <assert.h>
#include <capy/capy.h>
#include <stdio.h>

int main()
{
    char txt[100] = {'a', 0, 'b', 0, 'a', 0};

    const char *a1 = "a";
    const char *a2 = &txt[0];
    const char *a3 = &txt[4];
    const char *b1 = "b";
    const char *b2 = &txt[2];

    struct capy_symbols symbols = {NULL};

    struct capy_symbol str_a1 = capy_symbols_add(&symbols, a1);
    struct capy_symbol str_a2 = capy_symbols_add(&symbols, a2);
    struct capy_symbol str_a3 = capy_symbols_add(&symbols, a3);
    struct capy_symbol str_a4 = capy_symbols_add(&symbols, "a");

    assert(str_a1.data == str_a2.data);
    assert(str_a1.data == str_a3.data);
    assert(str_a1.data == str_a4.data);

    struct capy_symbol str_b1 = capy_symbols_add(&symbols, b1);
    struct capy_symbol str_b2 = capy_symbols_add(&symbols, b2);
    struct capy_symbol str_b3 = capy_symbols_add(&symbols, "b");

    assert(str_b1.data == str_b2.data);
    assert(str_b1.data == str_b3.data);

    capy_symbols_add(&symbols, "test1");
    capy_symbols_add(&symbols, "example1");

    capy_symbols_free(&symbols);

    return 0;
}
