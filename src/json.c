#include <capy/capy.h>
#include <capy/macros.h>

capy_jsonval capy_json_null(void)
{
    return (capy_jsonval){.kind = CAPY_JSON_NULL};
}

capy_jsonval capy_json_string(const char *string)
{
    return (capy_jsonval){.kind = CAPY_JSON_STRING, .string = string};
}

capy_jsonval capy_json_number(double number)
{
    return (capy_jsonval){.kind = CAPY_JSON_NUMBER, .number = number};
}

capy_jsonval capy_json_bool(int boolean)
{
    return (capy_jsonval){.kind = CAPY_JSON_BOOL, .boolean = boolean};
}

capy_jsonval capy_json_object(capy_arena *arena)
{
    size_t capacity = 8;

    capy_jsonobj *obj = capy_arena_alloc(arena, sizeof(capy_jsonobj) + (sizeof(capy_jsonkv) * capacity), 8, true);

    if (obj != NULL)
    {
        obj->size = 0;
        obj->capacity = capacity;
        obj->arena = arena;
        obj->items = ReinterpretCast(capy_jsonkv *, obj + 1);
    }

    return (capy_jsonval){.kind = CAPY_JSON_OBJECT, .object = obj};
}

capy_err capy_json_object_set(capy_jsonobj *object, const char *key, capy_jsonval value)
{
    capy_jsonkv kv = {.key = capy_string_cstr(key), .value = value};

    void *items = capy_strmap_set(object->arena, object->items, sizeof(capy_jsonkv), &object->capacity, &object->size, &kv);

    if (items == NULL)
    {
        return ErrStd(ENOMEM);
    }

    object->items = items;

    return Ok;
}

capy_jsonval *capy_json_object_get(capy_jsonobj *object, const char *key)
{
    return capy_strmap_get(object->items, sizeof(capy_string), object->capacity, capy_string_cstr(key));
}

capy_jsonval capy_json_array(capy_arena *arena)
{
    size_t capacity = 8;

    capy_jsonarr *buf = capy_arena_alloc(arena, sizeof(capy_buffer) + (capacity * sizeof(capy_jsonval)), 8, false);

    if (buf != NULL)
    {
        buf->size = 0;
        buf->capacity = capacity;
        buf->arena = arena;
        buf->data = ReinterpretCast(capy_jsonval *, buf + 1);
    }

    return (capy_jsonval){.kind = CAPY_JSON_ARRAY, .array = buf};
}

capy_err capy_json_array_push(capy_jsonarr *array, capy_jsonval value)
{
    void *tmp = capy_vec_insert(array->arena, array->data, sizeof(capy_jsonval), &array->capacity, &array->size, array->size, 1, &value);

    if (tmp == NULL)
    {
        return ErrStd(ENOMEM);
    }

    array->data = Cast(capy_jsonval *, tmp);

    return Ok;
}

static capy_err capy_json_serialize_(capy_buffer *buffer, capy_jsonval value, int tabsize, int tabs)
{
    switch (value.kind)
    {
        case CAPY_JSON_BOOL:
        {
            if (value.boolean)
            {
                return capy_buffer_write_cstr(buffer, "true");
            }
            else
            {
                return capy_buffer_write_cstr(buffer, "false");
            }
        }
        break;

        case CAPY_JSON_NULL:
        {
            return capy_buffer_write_cstr(buffer, "null");
        }
        break;

        case CAPY_JSON_NUMBER:
        {
            return capy_buffer_write_fmt(buffer, 0, "%g", value.number);
        }
        break;

        case CAPY_JSON_STRING:
        {
            return capy_buffer_write_fmt(buffer, 0, "\"%s\"", value.string);
        }
        break;

        case CAPY_JSON_OBJECT:
        {
            capy_err err = capy_buffer_write_bytes(buffer, 1, "{");

            if (err.code)
            {
                return err;
            }

            tabs += 1;

            bool first = true;

            for (size_t i = 0; value.object != NULL && i < value.object->capacity; i++)
            {
                capy_jsonkv keyval = value.object->items[i];

                if (keyval.key.size == 0)
                {
                    continue;
                }

                if (!first)
                {
                    err = capy_buffer_write_bytes(buffer, 1, ",");

                    if (err.code)
                    {
                        return err;
                    }
                }

                if (tabsize > 0)
                {
                    err = capy_buffer_write_fmt(buffer, 0, "\n%*s", tabs * tabsize, "");

                    if (err.code)
                    {
                        return err;
                    }
                }

                err = capy_buffer_write_fmt(buffer, 0, "\"%.*s\":", (int)keyval.key.size, keyval.key.data);

                if (err.code)
                {
                    return err;
                }

                if (tabsize > 0)
                {
                    err = capy_buffer_write_bytes(buffer, 1, " ");

                    if (err.code)
                    {
                        return err;
                    }
                }

                err = capy_json_serialize_(buffer, keyval.value, tabsize, tabs);

                if (err.code)
                {
                    return err;
                }

                first = false;
            }

            tabs -= 1;

            if (tabsize > 0 && value.object != NULL)
            {
                err = capy_buffer_write_fmt(buffer, 0, "\n%*s", tabs * tabsize, "");

                if (err.code)
                {
                    return err;
                }
            }

            err = capy_buffer_write_bytes(buffer, 1, "}");

            if (err.code)
            {
                return err;
            }
        }
        break;

        case CAPY_JSON_ARRAY:
        {
            capy_err err;

            err = capy_buffer_write_bytes(buffer, 1, "[");

            if (err.code)
            {
                return err;
            }

            tabs += 1;

            for (size_t i = 0; value.array != NULL && i < value.array->size; i++)
            {
                capy_jsonval el = value.array->data[i];

                if (i != 0)
                {
                    err = capy_buffer_write_bytes(buffer, 1, ",");

                    if (err.code)
                    {
                        return err;
                    }
                }

                if (tabsize > 0)
                {
                    err = capy_buffer_write_fmt(buffer, 0, "\n%*s", tabs * tabsize, "");

                    if (err.code)
                    {
                        return err;
                    }
                }

                err = capy_json_serialize_(buffer, el, tabsize, tabs);

                if (err.code)
                {
                    return err;
                }
            }

            tabs -= 1;

            if (tabsize > 0 && value.array != NULL)
            {
                err = capy_buffer_write_fmt(buffer, 0, "\n%*s", tabs * tabsize, "");

                if (err.code)
                {
                    return err;
                }
            }

            err = capy_buffer_write_bytes(buffer, 1, "]");

            if (err.code)
            {
                return err;
            }
        }
        break;
    }

    return Ok;
}

capy_err capy_json_serialize(capy_buffer *buffer, capy_jsonval value, int tabsize)
{
    return capy_json_serialize_(buffer, value, tabsize, 0);
}

static capy_err capy_json_parse_number(double *number, capy_string *input)
{
    // always starts with '-' or a digit

    capy_string content = *input;

    if (input->data[0] == '-')
    {
        *input = capy_string_shl(*input, 1);

        if (input->size == 0)
        {
            return ErrFmt(EINVAL, "no number after minus sign");
        }
    }

    size_t digits = 0;

    while (input->size)
    {
        char c = input->data[0];

        if (!capy_char_isdigit(c))
        {
            break;
        }

        *input = capy_string_shl(*input, 1);
        digits += 1;

        if (digits == 0 && c == '0')
        {
            break;
        }
    }

    if (digits == 0)
    {
        return ErrFmt(EINVAL, "unexpected non-digit");
    }

    if (input->size > 0)
    {
        if (input->data[0] == '.')
        {
            *input = capy_string_shl(*input, 1);

            digits = 0;

            while (input->size)
            {
                char c = input->data[0];

                if (!capy_char_isdigit(c))
                {
                    break;
                }

                *input = capy_string_shl(*input, 1);
                digits += 1;
            }

            if (digits == 0)
            {
                return ErrFmt(EINVAL, "unterminated fractional number");
            }
        }
    }

    if (input->size > 0)
    {
        if (capy_char_is(input->data[0], "eE"))
        {
            *input = capy_string_shl(*input, 1);

            if (capy_char_is(input->data[0], "+-"))
            {
                *input = capy_string_shl(*input, 1);
            }

            digits = 0;

            while (input->size)
            {
                char c = input->data[0];

                if (!capy_char_isdigit(c))
                {
                    break;
                }

                *input = capy_string_shl(*input, 1);
                digits += 1;
            }

            if (digits == 0)
            {
                return ErrFmt(EINVAL, "exponent part is missing a number");
            }
        }
    }

    content = capy_string_slice(content, 0, content.size - input->size);

    *number = strtod(content.data, NULL);

    return Ok;
}

static capy_err capy_json_parse_string(capy_arena *arena, const char **cstr, capy_string *input)
{
    // always starts with '"'

    *input = capy_string_shl(*input, 1);

    capy_string content = *input;

    size_t size = 0;

    while (input->size)
    {
        char c = input->data[0];

        if (Cast(unsigned char, c) < 0x20)
        {
            return ErrFmt(EINVAL, "bad control character in string literal");
        }

        if (c == '"')
        {
            break;
        }
        else if (c == '\\')
        {
            if (input->size < 2)
            {
                return ErrFmt(EINVAL, "unterminated string literal");
            }

            switch (input->data[1])
            {
                case '"':
                case '\\':
                case '/':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                {
                    *input = capy_string_shl(*input, 2);
                    size += 1;
                }
                break;

                case 'u':
                {
                    uint64_t v = 0;

                    if (input->size < 6 || capy_string_parse_hexdigits(&v, capy_string_slice(*input, 2, 6)) != 4)
                    {
                        return ErrFmt(EINVAL, "bad Unicode escape \"%.*s\"", (int)input->size, input->data);
                    }

                    *input = capy_string_shl(*input, 6);

                    size += 3;
                }
                break;

                default:
                    return ErrFmt(EINVAL, "bad escaped character \"%.*s\"", 2, input->data);
            }
        }
        else
        {
            *input = capy_string_shl(*input, 1);
            size += 1;
        }
    }

    if (input->size == 0)
    {
        return ErrFmt(EINVAL, "unterminated string literal");
    }

    char *buffer = Make(arena, char, size + 1);

    if (buffer == NULL)
    {
        return ErrStd(ENOMEM);
    }

    content = capy_string_slice(content, 0, content.size - input->size);

    *input = capy_string_shl(*input, 1);

    size_t i = 0;

    while (content.size)
    {
        char c = content.data[0];

        if (c == '\\')
        {
            switch (content.data[1])
            {
                case '"':
                    buffer[i++] = '"';
                    content = capy_string_shl(content, 1);
                    break;

                case '\\':
                    buffer[i++] = '\\';
                    content = capy_string_shl(content, 1);
                    break;

                case '/':
                    buffer[i++] = '/';
                    content = capy_string_shl(content, 1);
                    break;

                case 'b':
                    buffer[i++] = '\b';
                    content = capy_string_shl(content, 1);
                    break;

                case 'f':
                    buffer[i++] = '\f';
                    content = capy_string_shl(content, 1);
                    break;

                case 'n':
                    buffer[i++] = '\n';
                    content = capy_string_shl(content, 1);
                    break;

                case 'r':
                    buffer[i++] = '\r';
                    content = capy_string_shl(content, 1);
                    break;

                case 't':
                    buffer[i++] = '\t';
                    content = capy_string_shl(content, 1);
                    break;

                case 'u':
                {
                    uint64_t high = 0;
                    uint64_t low = 0;

                    capy_string_parse_hexdigits(&high, capy_string_slice(content, 2, 6));
                    content = capy_string_shl(content, 6);

                    if (high >= 0xD800 && high <= 0xDFFF)
                    {
                        capy_string_parse_hexdigits(&low, capy_string_slice(content, 2, 6));
                        content = capy_string_shl(content, 6);
                    }

                    uint32_t code = capy_unicode_utf16(Cast(uint16_t, high), Cast(uint16_t, low));
                    i += capy_unicode_utf8encode(buffer + i, code);
                }
                break;
            }
        }
        else
        {
            buffer[i++] = c;
            content = capy_string_shl(content, 1);
        }
    }

    buffer[i] = 0;

    *cstr = buffer;

    return Ok;
}

static capy_err capy_json_deserialize_(capy_arena *arena, capy_jsonval *value, capy_string *input)
{
    capy_err err;

    *input = capy_string_ltrim(*input, " \t\r\n");

    if (input->size == 0)
    {
        return ErrFmt(EINVAL, "unexpected end of data");
    }

    switch (input->data[0])
    {
        case 'n':
        {
            if (input->size < 4 && !ArrCmp4(input->data, 'n', 'u', 'l', 'l'))
            {
                return ErrFmt(EINVAL, "unexpected keyword");
            }

            *value = capy_json_null();
            *input = capy_string_shl(*input, 4);
        }
        break;

        case 't':
        {
            if (input->size < 4 && !ArrCmp4(input->data, 't', 'r', 'u', 'e'))
            {
                return ErrFmt(EINVAL, "unexpected keyword");
            }

            *value = capy_json_bool(true);
            *input = capy_string_shl(*input, 4);
        }
        break;

        case 'f':
        {
            if (input->size < 5 && !ArrCmp5(input->data, 'f', 'a', 'l', 's', 'e'))
            {
                return ErrFmt(EINVAL, "unexpected keyword");
            }

            *value = capy_json_bool(false);
            *input = capy_string_shl(*input, 5);
        }
        break;

        case '"':
        {
            const char *string;

            err = capy_json_parse_string(arena, &string, input);

            if (err.code)
            {
                return err;
            }

            *value = capy_json_string(string);
        }
        break;

        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            double number = 0;

            err = capy_json_parse_number(&number, input);

            if (err.code)
            {
                return err;
            }

            *value = capy_json_number(number);
        }
        break;

        case '[':
        {
            *input = capy_string_shl(*input, 1);
            *input = capy_string_ltrim(*input, " \t\r\n");

            if (input->size > 0 && input->data[0] == ']')
            {
                *input = capy_string_shl(*input, 1);
                *value = (capy_jsonval){.kind = CAPY_JSON_ARRAY, .object = NULL};
            }
            else
            {
                capy_jsonval arr = capy_json_array(arena);

                if (arr.array == NULL)
                {
                    return ErrStd(ENOMEM);
                }

                for (;;)
                {
                    capy_jsonval element;

                    err = capy_json_deserialize_(arena, &element, input);

                    if (err.code)
                    {
                        return err;
                    }

                    err = capy_json_array_push(arr.array, element);

                    if (err.code)
                    {
                        return err;
                    }

                    *input = capy_string_ltrim(*input, " \t\r\n");

                    if (input->size == 0)
                    {
                        return ErrFmt(EINVAL, "end of data when ',' or ']' was expected");
                    }

                    if (input->data[0] == ',')
                    {
                        *input = capy_string_shl(*input, 1);
                    }
                    else if (input->data[0] == ']')
                    {
                        *input = capy_string_shl(*input, 1);
                        break;
                    }
                    else
                    {
                        return ErrFmt(EINVAL, "expected ',' or ']' after array element");
                    }
                }

                *value = arr;
            }
        }
        break;

        case '{':
        {
            *input = capy_string_shl(*input, 1);
            *input = capy_string_ltrim(*input, " \t\r\n");

            if (input->size == 0)
            {
                return ErrFmt(EINVAL, "end of data while reading object contents");
            }

            if (input->data[0] == '}')
            {
                *input = capy_string_shl(*input, 1);
                *value = (capy_jsonval){.kind = CAPY_JSON_OBJECT, .object = NULL};
            }
            else
            {
                capy_jsonval obj = capy_json_object(arena);

                if (obj.object == NULL)
                {
                    return ErrStd(ENOMEM);
                }

                for (;;)
                {
                    *input = capy_string_ltrim(*input, " \t\r\n");

                    if (input->size == 0)
                    {
                        return ErrFmt(EINVAL, "end of data when property name was expected");
                    }

                    if (input->data[0] != '"')
                    {
                        return ErrFmt(EINVAL, "expected property name or '}'");
                    }

                    const char *key;

                    err = capy_json_parse_string(arena, &key, input);

                    if (err.code)
                    {
                        return err;
                    }

                    *input = capy_string_ltrim(*input, " \t\r\n");

                    if (input->size == 0)
                    {
                        return ErrFmt(EINVAL, "end of data after property name when ':' was expected");
                    }

                    if (input->data[0] != ':')
                    {
                        return ErrFmt(EINVAL, "expected ':' after property name in object");
                    }

                    *input = capy_string_shl(*input, 1);
                    *input = capy_string_ltrim(*input, " \t\r\n");

                    capy_jsonval element;

                    err = capy_json_deserialize_(arena, &element, input);

                    if (err.code)
                    {
                        return err;
                    }

                    err = capy_json_object_set(obj.object, key, element);

                    if (err.code)
                    {
                        return err;
                    }

                    *input = capy_string_ltrim(*input, " \t\r\n");

                    if (input->size == 0)
                    {
                        return ErrFmt(EINVAL, "end of data after property value in object");
                    }

                    if (input->data[0] == ',')
                    {
                        *input = capy_string_shl(*input, 1);
                    }
                    else if (input->data[0] == '}')
                    {
                        *input = capy_string_shl(*input, 1);
                        break;
                    }
                    else
                    {
                        return ErrFmt(EINVAL, "expected double-quoted property name");
                    }
                }

                *value = obj;
            }
        }
        break;

        default:
            return ErrFmt(EINVAL, "unexpected character");
            break;
    }

    return Ok;
}

capy_err capy_json_deserialize(capy_arena *arena, capy_jsonval *value, const char *in)
{
    capy_string input = capy_string_cstr(in);
    capy_string cursor = input;

    capy_err err = capy_json_deserialize_(arena, value, &cursor);

    if (err.code)
    {
        size_t index = input.size - cursor.size;
        size_t line = 1;
        size_t column = 1;

        for (size_t i = 0; i < index; i++)
        {
            if (input.data[i] == '\n')
            {
                line += 1;
                column = 1;
            }
            else
            {
                column += 1;
            }
        }

        return ErrFmt(err.code, "%s at line %zu column %zu", err.msg, line, column);
    }

    return Ok;
}
