#ifndef CAPY_JSON_H
#define CAPY_JSON_H

#include <capy/buffer.h>
#include <capy/string.h>
#include <capy/strmap.h>
#include <capy/vec.h>

typedef enum capy_json_kind
{
    CAPY_JSON_BOOL,
    CAPY_JSON_NULL,
    CAPY_JSON_NUMBER,
    CAPY_JSON_STRING,
    CAPY_JSON_OBJECT,
    CAPY_JSON_ARRAY,
} capy_json_kind;

typedef struct capy_json_value
{
    capy_json_kind kind;
    union
    {
        double number;
        bool boolean;
        const char *string;
        struct capy_json_object *object;
        struct capy_json_array *array;
    };
} capy_json_value;

typedef struct capy_json_keyval
{
    capy_string key;
    capy_json_value value;
} capy_json_keyval;

typedef struct capy_json_object
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_json_keyval *items;
} capy_json_object_t;

typedef struct capy_json_array
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_json_value *data;
} capy_json_array_t;

capy_json_value capy_json_null(void);
capy_json_value capy_json_string(const char *string);
capy_json_value capy_json_number(double number);
capy_json_value capy_json_bool(int boolean);

must_check capy_json_value capy_json_object(capy_arena *arena);
must_check capy_err capy_json_object_set(capy_json_object_t *object, const char *key, capy_json_value value);
capy_json_value *capy_json_object_get(capy_json_object_t *object, const char *key);

must_check capy_json_value capy_json_array(capy_arena *arena);
must_check capy_err capy_json_array_push(capy_json_array_t *array, capy_json_value value);

capy_err capy_json_deserialize(capy_arena *arena, capy_json_value *value, const char *input);
capy_err capy_json_serialize(capy_buffer *buffer, capy_json_value value, int tabsize);

#endif
