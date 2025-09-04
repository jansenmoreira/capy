#include <capy/test.h>

static int test_string(void)
{
    capy_arena *arena = capy_arena_init(MiB(10));

    const char *buffer = "012345";

    capy_string cstr = capy_string_cstr(buffer);
    expect_str_eq(str("012345"), cstr);

    capy_string bytes = capy_string_bytes(4, buffer);
    expect_str_eq(str("0123"), bytes);

    capy_string lower = capy_string_lower(arena, str("+AZaz12{}"));
    capy_string upper = capy_string_upper(arena, str("+AZaz12{}"));

    expect_str_eq(str("+azaz12{}"), lower);
    expect_str_eq(str("+AZAZ12{}"), upper);

    capy_string empty = (capy_string){.size = 0};
    expect_str_eq(capy_string_copy(arena, str("")), empty);

    expect_str_eq(capy_string_slice(str("+AZaz12"), 0, 7), str("+AZaz12"));
    expect_str_eq(capy_string_slice(str("+AZaz12"), 1, 5), str("AZaz"));
    expect_str_eq(capy_string_slice(str("+AZaz12"), 0, 0), str(""));

    expect_str_eq(capy_string_trim(str(""), " "), str(""));
    expect_str_eq(capy_string_trim(str("  "), " "), str(""));
    expect_str_eq(capy_string_trim(str("  a "), " "), str("a"));
    expect_str_eq(capy_string_trim(str("a   "), " "), str("a"));

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

    capy_strbuf *strbuf = capy_strbuf_init(arena, 1024);
    expect_u_eq(strbuf->capacity, 1024);
    capy_strbuf_write(strbuf, str("foobar\n"));
    capy_strbuf_write_cstr(strbuf, "baz");
    capy_strbuf_write_bytes(strbuf, 2, "baz");
    capy_strbuf_snprintf(strbuf, 50, " %d %.1f", 5, 1.3f);
    expect_s_eq(memcmp(strbuf->data, "foobar\nbazba 5 1.3", strbuf->size), 0);
    capy_strbuf_shl(strbuf, 7);
    expect_s_eq(memcmp(strbuf->data, "bazba 5 1.3", strbuf->size), 0);
    capy_strbuf_resize(strbuf, 5);
    expect_s_eq(memcmp(strbuf->data, "bazba", strbuf->size), 0);

    capy_arena_free(arena);

    return 0;
}
