#include <capy/test.h>

static int test_string(void)
{
    capy_arena *arena;
    capy_string cstr, bytes, lower, upper, empty, joined;
    int64_t value;
    size_t bytes_read;

    arena = capy_arena_init(KiB(4));
    expect_p_ne(capy_arena_make(arena, char, 4040), NULL);

    capy_string text = str("The quick brown fox jumps over the lazy dog");

    expect_s_eq(capy_string_copy(arena, &bytes, text), ENOMEM);
    expect_s_eq(capy_string_lower(arena, &bytes, text), ENOMEM);
    expect_s_eq(capy_string_upper(arena, &bytes, text), ENOMEM);
    expect_s_eq(capy_string_join(arena, &bytes, " ", 1, (capy_string[]){text}), ENOMEM);

    capy_arena_destroy(arena);

    arena = capy_arena_init(MiB(10));

    const char *numbers = "012345";

    cstr = capy_string_cstr(numbers);
    expect_str_eq(str("012345"), cstr);

    bytes = capy_string_bytes(4, numbers);
    expect_str_eq(str("0123"), bytes);

    expect_s_eq(capy_string_lower(arena, &lower, str("+AZaz12{}")), 0);

    expect_s_eq(capy_string_upper(arena, &upper, str("+AZaz12{}")), 0);

    expect_str_eq(str("+azaz12{}"), lower);
    expect_str_eq(str("+AZAZ12{}"), upper);

    empty = lower;
    expect_s_eq(capy_string_copy(arena, &empty, str("")), 0);
    expect_str_eq(str(""), empty);

    expect_str_eq(capy_string_slice(str("+AZaz12"), 0, 7), str("+AZaz12"));
    expect_str_eq(capy_string_slice(str("+AZaz12"), 1, 5), str("AZaz"));
    expect_str_eq(capy_string_slice(str("+AZaz12"), 0, 0), str(""));

    expect_str_eq(capy_string_trim(str(""), " "), str(""));
    expect_str_eq(capy_string_trim(str("  "), " "), str(""));
    expect_str_eq(capy_string_trim(str("  a "), " "), str("a"));
    expect_str_eq(capy_string_trim(str("a   "), " "), str("a"));

    expect_str_eq(capy_string_prefix(str("foobar"), str("foo")), str("foo"));
    expect_str_eq(capy_string_prefix(str("foo"), str("foobar")), str("foo"));
    expect_str_eq(capy_string_prefix(str("foobar"), str("bar")), str(""));

    expect_s_eq(capy_string_sw(lower, str("+")), 1);
    expect_s_eq(capy_string_sw(str("+"), lower), 0);

    expect_s_eq(capy_string_join(arena, &joined, ", ", 0, NULL), 0);
    expect_str_eq(joined, str(""));

    expect_s_eq(capy_string_join(arena, &joined, ", ", 3, (capy_string[]){lower, empty, upper}), 0);
    expect_str_eq(joined, str("+azaz12{}, , +AZAZ12{}"));

    bytes_read = capy_string_hex(str("a0b1c2d3e4f5"), &value);
    expect_u_eq(bytes_read, 12);
    expect_s_eq(value, 0xa0b1c2d3e4f5);

    bytes_read = capy_string_hex(str("-A9B8C7D6E5F4 "), &value);
    expect_u_eq(bytes_read, 13);
    expect_s_eq(value, -0xA9B8C7D6E5F4);

    bytes_read = capy_string_hex(str(""), &value);
    expect_u_eq(bytes_read, 0);
    expect_s_eq(value, -0xA9B8C7D6E5F4);

    bytes_read = capy_string_hex(str("-"), &value);
    expect_u_eq(bytes_read, 0);
    expect_s_eq(value, -0xA9B8C7D6E5F4);

    bytes_read = capy_string_hex(str("-G"), &value);
    expect_u_eq(bytes_read, 0);
    expect_s_eq(value, -0xA9B8C7D6E5F4);

    capy_arena_destroy(arena);

    return 0;
}
