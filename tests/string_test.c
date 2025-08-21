#include "test.h"

int test_string(void)
{
    capy_string s = capy_string_lit("+AZaz12");

    capy_arena *arena = capy_arena_init(1024);

    capy_string lower = capy_string_tolower(arena, s);
    capy_string upper = capy_string_toupper(arena, s);
    capy_string slice = capy_string_slice(s, 1, 3);

    expect(capy_string_equals(capy_string_lit("+azaz12"), lower));
    expect(capy_string_equals(capy_string_lit("+AZAZ12"), upper));
    expect(capy_string_equals(capy_string_lit("AZ"), slice));

    expect(capy_string_slice((capy_string){NULL}, 0, 5).size == 0);
    expect(capy_string_slice(s, 0, 9).size == 0);
    expect(capy_string_slice(s, 3, 2).size == 0);
    expect(capy_string_slice(s, 9, 15).size == 0);

    return 0;
}
