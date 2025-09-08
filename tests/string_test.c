#include <capy/test.h>

static int test_string(void)
{
    capy_arena *arena = capy_arena_init(MiB(10));

    const char *numbers = "012345";

    capy_string cstr = capy_string_cstr(numbers);
    expect_str_eq(str("012345"), cstr);

    capy_string bytes = capy_string_bytes(4, numbers);
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

    capy_buffer *buffer = capy_buffer_init(arena, 1024);
    expect_u_eq(buffer->capacity, 1024);
    capy_buffer_write(buffer, str("foobar\n"));
    capy_buffer_write_cstr(buffer, "baz");
    capy_buffer_write_bytes(buffer, 2, "baz");
    capy_buffer_format(buffer, 50, " %d %.1f", 5, 1.3f);
    expect_s_eq(memcmp(buffer->data, "foobar\nbazba 5 1.3", buffer->size), 0);
    capy_buffer_shl(buffer, 7);
    expect_s_eq(memcmp(buffer->data, "bazba 5 1.3", buffer->size), 0);
    capy_buffer_resize(buffer, 5);
    expect_s_eq(memcmp(buffer->data, "bazba", buffer->size), 0);

    capy_buffer_resize(buffer, 0);
    int s = capy_buffer_format(buffer, 4, "%d", 123456);
    expect_s_eq(s, 6);
    expect_s_eq(memcmp(buffer->data, "1234", buffer->size), 0);

    capy_arena_free(arena);

    return 0;
}
