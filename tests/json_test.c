#include <capy/macros.h>
#include <capy/test.h>

static int test_capy_json_deserialize(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_json_value value;

    expect_ok(capy_json_deserialize(arena, &value, "null"));
    expect_s_eq(value.kind, CAPY_JSON_NULL);

    expect_ok(capy_json_deserialize(arena, &value, "true"));
    expect_s_eq(value.kind, CAPY_JSON_BOOL);
    expect_true(value.boolean);

    expect_ok(capy_json_deserialize(arena, &value, "false"));
    expect_s_eq(value.kind, CAPY_JSON_BOOL);
    expect_false(value.boolean);

    expect_ok(capy_json_deserialize(arena, &value, "[\"abcd𐐷\\uD801\\uDC37\",true,null,{},[]]"));
    expect_s_eq(value.kind, CAPY_JSON_ARRAY);
    expect_u_eq(value.array->size, 5);
    expect_s_eq(value.array->data[0].kind, CAPY_JSON_STRING);
    expect_s_eq(strcmp(value.array->data[0].string, "abcd𐐷𐐷"), 0);
    expect_s_eq(value.array->data[1].kind, CAPY_JSON_BOOL);
    expect_true(value.array->data[1].boolean);
    expect_s_eq(value.array->data[2].kind, CAPY_JSON_NULL);

    capy_buffer *buffer = capy_buffer_init(arena, KiB(1));
    capy_json_serialize(buffer, value, 3);
    printf("%s\n", buffer->data);

    expect_ok(capy_json_deserialize(arena, &value, "300 "));

    return true;
}

static int test_capy_json_serialize(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));
    capy_buffer *buffer = capy_buffer_init(arena, KiB(1));

    capy_json_value arr = capy_json_array(arena);
    expect_ok(capy_json_array_push(arr.array, capy_json_bool(true)));
    expect_ok(capy_json_array_push(arr.array, capy_json_bool(false)));
    expect_ok(capy_json_array_push(arr.array, capy_json_null()));

    capy_json_value obj = capy_json_object(arena);
    expect_ok(capy_json_object_set(obj.object, "a", capy_json_string("teste")));
    expect_ok(capy_json_object_set(obj.object, "b", capy_json_number(10)));
    expect_ok(capy_json_object_set(obj.object, "c", capy_json_number(16.32)));
    expect_ok(capy_json_object_set(obj.object, "d", arr));

    capy_json_serialize(buffer, obj, 0);
    printf("%s\n", buffer->data);
    buffer->size = 0;

    capy_json_serialize(buffer, obj, 3);
    printf("%s\n", buffer->data);

    return true;
}

static void test_json(testbench *t)
{
    runtest(t, test_capy_json_serialize, "capy_json_serialize");
    runtest(t, test_capy_json_deserialize, "capy_json_deserialize");
}
