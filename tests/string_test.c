#include "test.h"

static struct
{
    capy_string input;
    size_t begin;
    size_t end;
    capy_string expected;
} slices[] = {
    {str("+AZaz12"), 0, 7, str("+AZaz12")},
    {str("+AZaz12"), 1, 5, str("AZaz")},
    {str("+AZaz12"), 0, 0, str("")},
};

static struct
{
    capy_string input;
    capy_string expected;
} trims[] = {
    {str(""), str("")},
    {str("  "), str("")},
    {str("  a "), str("a")},
    {str("a   "), str("a")},
};

static int test_string(void)
{
    int err;

    capy_arena *arena = capy_arena_init(MiB(1));

    const char *buffer = "012345";

    capy_string cstr = capy_string_cstr(buffer);
    expect_str_eq(str("012345"), cstr);

    capy_string bytes = capy_string_bytes(buffer, 4);
    expect_str_eq(str("0123"), bytes);

    capy_string lower;
    err = capy_string_tolower(arena, str("+AZaz12"), &lower);
    expect_s_eq(err, 0);

    capy_string upper;
    err = capy_string_toupper(arena, str("+AZaz12"), &upper);
    expect_s_eq(err, 0);

    expect_str_eq(str("+azaz12"), lower);
    expect_str_eq(str("+AZAZ12"), upper);

    capy_string empty = (capy_string){NULL};
    capy_string empty2;

    err = capy_string_copy(arena, str(""), &empty2);
    expect_s_eq(err, 0);
    expect_str_eq(empty, empty2);

    for (size_t i = 0; i < arrlen(slices); i++)
    {
        capy_string result = capy_string_slice(slices[i].input, slices[i].begin, slices[i].end);
        expect_str_eq(result, slices[i].expected);
    }

    for (size_t i = 0; i < arrlen(trims); i++)
    {
        capy_string result = capy_string_trim(trims[i].input, " ");
        expect_str_eq(result, trims[i].expected);
    }

    expect_s_eq(capy_string_sw(lower, str("+")), 1);
    expect_s_eq(capy_string_sw(str("+"), lower), 0);

    capy_string joined;
    err = capy_string_join(arena, str(", "), 0, NULL, &joined);
    expect_s_eq(err, 0);
    expect_str_eq(joined, str(""));

    err = capy_string_join(arena, str(", "), 3, (capy_string[3]){lower, empty, upper}, &joined);
    expect_s_eq(err, 0);
    expect_str_eq(joined, str("+azaz12, , +AZAZ12"));

    int64_t value;
    size_t bytes_read = capy_string_hex(str("a0b1c2d3e4f5"), &value);
    expect_u_eq(bytes_read, 12);
    expect_s_eq(value, 0xa0b1c2d3e4f5);

    bytes_read = capy_string_hex(str("-A9B8C7D6E5F4 "), &value);
    expect_u_eq(bytes_read, 13);
    expect_s_eq(value, -0xA9B8C7D6E5F4);

    return 0;
}
