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

    capy_string lower = capy_string_tolower(arena, str("+AZaz12"));
    capy_string upper = capy_string_toupper(arena, str("+AZaz12"));
    capy_string empty = (capy_string){NULL};

    expect_str_eq(str("+azaz12"), lower);
    expect_str_eq(str("+AZAZ12"), upper);
    expect_str_eq(capy_string_copy(arena, str("")), empty);

    for (size_t i = 0; i < arrlen(slices); i++)
    {
        capy_string result = capy_string_slice(slices[i].input, slices[i].begin, slices[i].end);
        expect_str_eq(result, slices[i].expected);
    }

    for (size_t i = 0; i < arrlen(trims); i++)
    {
        capy_string result = capy_string_trim(trims[i].input);
        expect_str_eq(result, trims[i].expected);
    }

    return 0;
}
