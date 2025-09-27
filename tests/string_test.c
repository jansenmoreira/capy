#include <capy/macros.h>
#include <capy/test.h>

static int test_capy_string_copy(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string input = Str("The quick brown fox jumps over the lazy dog");
    capy_string output;

    ExpectOk(capy_string_copy(arena, &output, input));
    ExpectNePtr(output.data, input.data);
    ExpectEqStr(output, input);

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_string_copy(arena, &output, input));

    input.size = 0;
    ExpectOk(capy_string_copy(arena, &output, input));
    ExpectNull(output.data);
    ExpectEqU(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_lower(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string expected = Str("+azaz09{}");
    capy_string input = Str("+AZaz09{}");
    capy_string output;

    ExpectOk(capy_string_lower(arena, &output, input));
    ExpectEqStr(output, expected);

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_string_lower(arena, &output, input));

    input.size = 0;
    ExpectOk(capy_string_lower(arena, &output, input));
    ExpectNull(output.data);
    ExpectEqU(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_upper(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string expected = Str("+AZAZ09{}");
    capy_string input = Str("+AZaz09{}");
    capy_string output;

    ExpectOk(capy_string_upper(arena, &output, input));
    ExpectEqStr(output, expected);

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_string_upper(arena, &output, input));

    input.size = 0;
    ExpectOk(capy_string_upper(arena, &output, input));
    ExpectNull(output.data);
    ExpectEqU(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_join(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_string expected = Str("one two three");
    capy_string input[] = {StrIni("one"), StrIni("two"), StrIni("three")};
    capy_string output;

    ExpectOk(capy_string_join(arena, &output, " ", 3, input));
    ExpectEqStr(output, expected);

    ExpectNotNull(Make(arena, char, capy_arena_available(arena)));
    ExpectErr(capy_string_join(arena, &output, " ", 3, input));

    ExpectOk(capy_string_join(arena, &output, " ", 0, input));
    ExpectNull(output.data);
    ExpectEqU(output.size, 0);

    capy_arena_destroy(arena);
    return true;
}

static int test_capy_string_trim(void)
{
    ExpectEqStr(capy_string_trim(Str(""), " "), Str(""));
    ExpectEqStr(capy_string_trim(Str("  "), " "), Str(""));
    ExpectEqStr(capy_string_trim(Str("  a "), " "), Str("a"));
    ExpectEqStr(capy_string_trim(Str("a   "), " "), Str("a"));
    return true;
}

static int test_capy_string_prefix(void)
{
    ExpectEqStr(capy_string_prefix(Str("foobar"), Str("foo")), Str("foo"));
    ExpectEqStr(capy_string_prefix(Str("foo"), Str("foobar")), Str("foo"));
    ExpectEqStr(capy_string_prefix(Str("foobar"), Str("bar")), Str(""));
    return true;
}

static int test_capy_string_hex(void)
{
    size_t bytes;
    uint64_t value;

    bytes = capy_string_parse_hexdigits(&value, Str("a0b1c2d3e4f5"));
    ExpectEqU(bytes, 12);
    ExpectEqU(value, 0xa0b1c2d3e4f5);

    bytes = capy_string_parse_hexdigits(&value, Str("A9B8C7D6E5F4 "));
    ExpectEqU(bytes, 12);
    ExpectEqU(value, 0xA9B8C7D6E5F4);

    bytes = capy_string_parse_hexdigits(&value, Str(""));
    ExpectEqU(bytes, 0);

    bytes = capy_string_parse_hexdigits(&value, Str("-"));
    ExpectEqU(bytes, 0);

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

    ExpectTrue(capy_string_eq(s1, s2));
    ExpectFalse(capy_string_eq(s1, s3));
    ExpectFalse(capy_string_eq(s1, s4));

    return true;
}

static int test_capy_string_cstr(void)
{
    const char *cstr = "foo";
    capy_string s1 = capy_string_cstr(cstr);
    ExpectEqPtr(s1.data, cstr);
    ExpectEqU(s1.size, 3);
    return true;
}

static int test_capy_string_slice(void)
{
    capy_string input = Str("foobar");
    ExpectEqStr(capy_string_slice(input, 1, 3), Str("oo"));
    ExpectEqStr(capy_string_shl(input, 3), Str("bar"));
    ExpectEqStr(capy_string_shr(input, 3), Str("foo"));
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
