#include <capy/macros.h>
#include <capy/test.h>

static int test_capy_string_copy(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string input = strl("The quick brown fox jumps over the lazy dog");
    capy_string output;

    expect_s_eq(capy_string_copy(arena, &output, input), 0);
    expect_p_ne(output.data, input.data);
    expect_str_eq(output, input);

    expect_p_ne(make(arena, char, 4000), NULL);
    expect_s_eq(capy_string_copy(arena, &output, input), ENOMEM);

    input.size = 0;
    expect_s_eq(capy_string_copy(arena, &output, input), 0);
    expect_p_eq(output.data, NULL);
    expect_u_eq(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_lower(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string expected = strl("+azaz09{}");
    capy_string input = strl("+AZaz09{}");
    capy_string output;

    expect_s_eq(capy_string_lower(arena, &output, input), 0);
    expect_str_eq(output, expected);

    expect_p_ne(make(arena, char, 4040), NULL);
    expect_s_eq(capy_string_lower(arena, &output, input), ENOMEM);

    input.size = 0;
    expect_s_eq(capy_string_lower(arena, &output, input), 0);
    expect_p_eq(output.data, NULL);
    expect_u_eq(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_upper(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string expected = strl("+AZAZ09{}");
    capy_string input = strl("+AZaz09{}");
    capy_string output;

    expect_s_eq(capy_string_upper(arena, &output, input), 0);
    expect_str_eq(output, expected);

    expect_p_ne(make(arena, char, 4040), NULL);
    expect_s_eq(capy_string_upper(arena, &output, input), ENOMEM);

    input.size = 0;
    expect_s_eq(capy_string_upper(arena, &output, input), 0);
    expect_p_eq(output.data, NULL);
    expect_u_eq(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_join(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string expected = strl("one two three");
    capy_string input[] = {strli("one"), strli("two"), strli("three")};
    capy_string output;

    expect_s_eq(capy_string_join(arena, &output, " ", 3, input), 0);
    expect_str_eq(output, expected);

    expect_p_ne(make(arena, char, 4040), NULL);
    expect_s_eq(capy_string_join(arena, &output, " ", 3, input), ENOMEM);

    expect_s_eq(capy_string_join(arena, &output, " ", 0, input), 0);
    expect_p_eq(output.data, NULL);
    expect_u_eq(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_trim(void)
{
    expect_str_eq(capy_string_trim(strl(""), " "), strl(""));
    expect_str_eq(capy_string_trim(strl("  "), " "), strl(""));
    expect_str_eq(capy_string_trim(strl("  a "), " "), strl("a"));
    expect_str_eq(capy_string_trim(strl("a   "), " "), strl("a"));
    return true;
}

static int test_capy_string_prefix(void)
{
    expect_str_eq(capy_string_prefix(strl("foobar"), strl("foo")), strl("foo"));
    expect_str_eq(capy_string_prefix(strl("foo"), strl("foobar")), strl("foo"));
    expect_str_eq(capy_string_prefix(strl("foobar"), strl("bar")), strl(""));
    return true;
}

static int test_capy_string_hex(void)
{
    size_t bytes;
    int64_t value;

    bytes = capy_string_hex(strl("a0b1c2d3e4f5"), &value);
    expect_u_eq(bytes, 12);
    expect_s_eq(value, 0xa0b1c2d3e4f5);

    bytes = capy_string_hex(strl("-A9B8C7D6E5F4 "), &value);
    expect_u_eq(bytes, 13);
    expect_s_eq(value, -0xA9B8C7D6E5F4);

    bytes = capy_string_hex(strl(""), &value);
    expect_u_eq(bytes, 0);
    expect_s_eq(value, -0xA9B8C7D6E5F4);

    bytes = capy_string_hex(strl("-"), &value);
    expect_u_eq(bytes, 0);
    expect_s_eq(value, -0xA9B8C7D6E5F4);

    bytes = capy_string_hex(strl("-G"), &value);
    expect_u_eq(bytes, 0);
    expect_s_eq(value, -0xA9B8C7D6E5F4);

    return true;
}

static int test_capy_string_eq(void)
{
    char a1[] = "foo";
    char a2[] = {'f', 'o', 'o'};
    const char *a3 = "bar";
    const char *a4 = "";

    capy_string s1 = {.data = a1, .size = 3};
    capy_string s2 = {.data = a2, .size = 3};
    capy_string s3 = {.data = a3, .size = 3};
    capy_string s4 = {.data = a4, .size = 0};

    expect_s_eq(capy_string_eq(s1, s2), true);
    expect_s_eq(capy_string_eq(s1, s3), false);
    expect_s_eq(capy_string_eq(s1, s4), false);

    return true;
}

static int test_capy_string_cstr(void)
{
    const char *cstr = "foo";
    capy_string s1 = capy_string_cstr(cstr);
    expect_p_eq(s1.data, cstr);
    expect_u_eq(s1.size, 3);
    return true;
}

static int test_capy_string_slice(void)
{
    capy_string input = strl("foobar");
    expect_str_eq(capy_string_slice(input, 1, 3), strl("oo"));
    expect_str_eq(capy_string_shl(input, 3), strl("bar"));
    expect_str_eq(capy_string_shr(input, 3), strl("foo"));
    return true;
}

static void test_string(testbench *t)
{
    runtest(t, test_capy_string_cstr, "capy_string_cstr");
    runtest(t, test_capy_string_eq, "capy_string_eq");
    runtest(t, test_capy_string_slice, "capy_string_(slice|shl|shr)");
    runtest(t, test_capy_string_copy, "capy_string_copy");
    runtest(t, test_capy_string_lower, "capy_string_lower");
    runtest(t, test_capy_string_upper, "capy_string_upper");
    runtest(t, test_capy_string_join, "capy_string_Join");
    runtest(t, test_capy_string_trim, "capy_string_(trim|ltrim|rtrim)");
    runtest(t, test_capy_string_prefix, "capy_string_prefix");
    runtest(t, test_capy_string_hex, "capy_string_hex");
}
