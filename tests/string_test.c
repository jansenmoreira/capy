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
    capy_arena *arena = capy_arena_init(MiB(1));

    const char *buffer = "012345";

    capy_string cstr = capy_string_cstr(buffer);
    expect_str_eq(str("012345"), cstr);

    capy_string bytes = capy_string_bytes(buffer, 4);
    expect_str_eq(str("0123"), bytes);

    capy_string lower = capy_string_lowercase(arena, str("+AZaz12{}"));
    capy_string upper = capy_string_uppercase(arena, str("+AZaz12{}"));

    expect_str_eq(str("+azaz12{}"), lower);
    expect_str_eq(str("+AZAZ12{}"), upper);

    capy_string empty = (capy_string){.size = 0};
    expect_str_eq(capy_string_copy(arena, str("")), empty);

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

    joined = capy_string_join(arena, ", ", 0, NULL);
    expect_str_eq(joined, str(""));

    joined = capy_string_join(arena, ", ", 3, (capy_string[]){lower, empty, upper});
    expect_str_eq(joined, str("+azaz12{}, , +AZAZ12{}"));

    int64_t value;
    size_t bytes_read = capy_string_hex(str("a0b1c2d3e4f5"), &value);
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

    capy_arena_free(arena);
    arena = capy_arena_init(64);

    return 0;
}
