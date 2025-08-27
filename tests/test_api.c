#include "test.h"

static int http_handler(capy_arena *arena, capy_http_request *request, capy_http_response *response)
{
    if (request->method != CAPY_HTTP_GET)
    {
        response->status = CAPY_HTTP_METHOD_NOT_ALLOWED;
        response->content = capy_string_copy(arena, str("405 Method Not Allowed\n"));
        return 0;
    }

    if (!capy_string_eq(request->uri.path, capy_string_literal("")) &&
        !capy_string_eq(request->uri.path, capy_string_literal("/")))
    {
        response->status = CAPY_HTTP_NOT_FOUND;
        response->content = capy_string_copy(arena, str("404 Not Found\n"));
        return 0;
    }

    response->status = CAPY_HTTP_OK;
    response->content = capy_string_copy(arena, str("200 OK\n"));

    return 0;
}

int main(void)
{
    capy_http_serve("0.0.0.0", "8080", http_handler);

    return 0;
}
