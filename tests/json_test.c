#include <capy/test.h>

static int test_capy_json_deserialize(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_jsonval value;

    ExpectOk(capy_json_deserialize(arena, &value, "null"));
    ExpectEqS(value.kind, CAPY_JSON_NULL);

    ExpectOk(capy_json_deserialize(arena, &value, "true"));
    ExpectEqS(value.kind, CAPY_JSON_BOOL);
    ExpectTrue(value.boolean);

    ExpectOk(capy_json_deserialize(arena, &value, "false"));
    ExpectEqS(value.kind, CAPY_JSON_BOOL);
    ExpectFalse(value.boolean);

    ExpectOk(capy_json_deserialize(arena, &value, "[\"abcd𐐷\\uD801\\uDC37\",true,null,{},[]]"));
    ExpectEqS(value.kind, CAPY_JSON_ARRAY);
    ExpectEqU(value.array->size, 5);
    ExpectEqS(value.array->data[0].kind, CAPY_JSON_STRING);
    ExpectEqCstr(value.array->data[0].string, "abcd𐐷𐐷");
    ExpectEqS(value.array->data[1].kind, CAPY_JSON_BOOL);
    ExpectTrue(value.array->data[1].boolean);
    ExpectEqS(value.array->data[2].kind, CAPY_JSON_NULL);

    capy_buffer *buffer = capy_buffer_init(arena, KiB(1));
    capy_json_serialize(buffer, value, 3);
    // printf("%s\n", buffer->data);

    ExpectOk(capy_json_deserialize(arena, &value, "300 "));

    return true;
}

static int test_capy_json_serialize(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));
    capy_buffer *buffer = capy_buffer_init(arena, KiB(1));

    capy_jsonval arr = capy_json_array(arena);
    ExpectOk(capy_json_array_push(arr.array, capy_json_bool(true)));
    ExpectOk(capy_json_array_push(arr.array, capy_json_bool(false)));
    ExpectOk(capy_json_array_push(arr.array, capy_json_null()));

    capy_jsonval obj = capy_json_object(arena);
    ExpectOk(capy_json_object_set(obj.object, "a", capy_json_string("teste")));
    ExpectOk(capy_json_object_set(obj.object, "b", capy_json_number(10)));
    ExpectOk(capy_json_object_set(obj.object, "c", capy_json_number(16.32)));
    ExpectOk(capy_json_object_set(obj.object, "d", arr));

    capy_json_serialize(buffer, obj, 0);
    // printf("%s\n", buffer->data);
    buffer->size = 0;

    capy_json_serialize(buffer, obj, 3);
    // printf("%s\n", buffer->data);

    return true;
}

static void test_json(testbench *t)
{
    runtest(t, test_capy_json_serialize, "capy_json_serialize");
    runtest(t, test_capy_json_deserialize, "capy_json_deserialize");
}
