#include <arpa/inet.h>
#include <capy/capy.h>
#include <capy/macros.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.c"

#define SIGNAL_EPOLL_EVENT -1ULL

typedef enum
{
    STATE_UNKNOWN,
    STATE_RESET,
    STATE_READ_REQUEST,
    STATE_PARSE_REQLINE,
    STATE_PARSE_HEADERS,
    STATE_PARSE_CONTENT,
    STATE_PARSE_CHUNKSIZE,
    STATE_PARSE_CHUNKDATA,
    STATE_PARSE_TRAILERS,
    STATE_ROUTE_REQUEST,
    STATE_WRITE_RESPONSE,
    STATE_BAD_REQUEST,
    STATE_SERVER_FAILURE,
    STATE_SSL_SHUTDOWN,
    STATE_CLOSE,
    STATE_BLOCK,
} http_conn_state;

static const char *http_conn_state_cstr[] = {
    [STATE_UNKNOWN] = "STATE_UNKNOWN",
    [STATE_RESET] = "STATE_RESET",
    [STATE_READ_REQUEST] = "STATE_READ_REQUEST",
    [STATE_PARSE_REQLINE] = "STATE_PARSE_REQLINE",
    [STATE_PARSE_HEADERS] = "STATE_PARSE_HEADERS",
    [STATE_PARSE_CONTENT] = "STATE_PARSE_CONTENT",
    [STATE_PARSE_CHUNKSIZE] = "STATE_PARSE_CHUNKSIZE",
    [STATE_PARSE_CHUNKDATA] = "STATE_PARSE_CHUNKDATA",
    [STATE_PARSE_TRAILERS] = "STATE_PARSE_TRAILERS",
    [STATE_ROUTE_REQUEST] = "STATE_ROUTE_REQUEST",
    [STATE_WRITE_RESPONSE] = "STATE_WRITE_RESPONSE",
    [STATE_BAD_REQUEST] = "STATE_BAD_REQUEST",
    [STATE_SERVER_FAILURE] = "STATE_SERVER_FAILURE",
    [STATE_CLOSE] = "STATE_CLOSE",
    [STATE_BLOCK] = "STATE_BLOCK",
};

typedef struct http_worker
{
    size_t id;
    pthread_t thread_id;
    int epoll_fd;
    sigset_t *mask;
} http_worker;

typedef struct http_conn
{
    int socket;

    pthread_mutex_t mut;

    size_t conn_id;

    capy_arena *arena;
    void *marker_init;

    ssize_t mem_headers;
    ssize_t mem_content;
    ssize_t mem_trailers;
    ssize_t mem_response;

    http_conn_state state;
    http_conn_state after_read;
    http_conn_state after_unblock;

    capy_buffer *line_buffer;

    capy_buffer *content_buffer;
    capy_buffer *response_buffer;

    size_t line_cursor;
    size_t chunk_size;

    capy_httpreq request;
    capy_httpresp response;

    capy_httprouter *router;

    capy_httpserveropt *options;

    struct timespec timestamp;

    int want_write;

    SSL *ssl;
} http_conn;

typedef struct http_server
{
    capy_arena *arena;
    capy_httprouter *router;

    int fd;
    int epoll_fd;
    int workers_epoll_fd;
    int signal_fd;
    sigset_t signals;

    http_worker *workers;
    http_conn **connections;
    size_t active_connections;

    capy_httpserveropt options;

    SSL_CTX *ssl_ctx;
} http_server;

static capy_err http_gaierror(int err)
{
    if (err == 0) return Ok;
    return (capy_err){.code = err, .msg = gai_strerror(err)};
}

static capy_err http_conn_update_epoll(http_conn *conn, int epoll_fd, int op, uint32_t events)
{
    struct epoll_event client_event = {
        .events = events,
        .data.ptr = conn,
    };

    if (epoll_ctl(epoll_fd, op, conn->socket, &client_event) == -1)
    {
        return ErrWrap(ErrStd(errno), "Failed to update workers epoll");
    }

    return Ok;
}

static capy_err http_conn_block(http_conn *conn, http_worker *worker)
{
    timespec_get(&conn->timestamp, TIME_UTC);

    uint32_t event = EPOLLIN;

    if (conn->want_write)
    {
        event = EPOLLOUT;
    }

    return http_conn_update_epoll(conn, worker->epoll_fd, EPOLL_CTL_MOD, event | EPOLLET | EPOLLONESHOT);
}

static capy_err http_conn_close(http_conn *conn)
{
    SSL_free(conn->ssl);
    close(conn->socket);
    conn->state = STATE_CLOSE;
    conn->socket = 0;
    return Ok;
}

static inline int http_conn_parse_msgline(http_conn *conn, capy_string *line)
{
    while (conn->line_cursor <= conn->line_buffer->size)
    {
        if (conn->line_cursor >= 2 &&
            conn->line_buffer->data[conn->line_cursor - 2] == '\r' &&
            conn->line_buffer->data[conn->line_cursor - 1] == '\n')
        {
            break;
        }

        conn->line_cursor += 1;
    }

    if (conn->line_cursor > conn->line_buffer->size)
    {
        return -1;
    }

    *line = capy_string_bytes(conn->line_cursor - 2, conn->line_buffer->data);

    return 0;
}

static inline void http_conn_shl_msg(http_conn *conn, size_t size)
{
    capy_buffer_shl(conn->line_buffer, size);

    if (size >= conn->line_cursor)
    {
        conn->line_cursor = 2;
    }
    else
    {
        conn->line_cursor -= size;
    }
}

static inline void http_conn_shl_msgline(http_conn *conn)
{
    http_conn_shl_msg(conn, conn->line_cursor);
}

static inline capy_err http_conn_write_response_ssl(http_conn *conn)
{
    size_t bytes_written = 0;

    if (!SSL_write_ex(conn->ssl, conn->response_buffer->data, conn->response_buffer->size, &bytes_written))
    {
        int ssl_err = SSL_get_error(conn->ssl, 0);

        switch (ssl_err)
        {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
            {
                conn->after_unblock = STATE_WRITE_RESPONSE;
                conn->state = STATE_BLOCK;
                conn->want_write = (ssl_err == SSL_ERROR_WANT_WRITE);
                return Ok;
            }

            case SSL_ERROR_ZERO_RETURN:
            {
                conn->state = STATE_CLOSE;
                return Ok;
            }

            default:
            {
                return ErrFmt(ssl_err, "Failed to write to SSL socket (%d)", ssl_err);
            }
        }
    }

    if (bytes_written == 0)
    {
        conn->state = STATE_CLOSE;
        return Ok;
    }

    capy_buffer_shl(conn->response_buffer, Cast(size_t, bytes_written));

    if (conn->response_buffer->size > 0)
    {
        conn->state = STATE_WRITE_RESPONSE;
    }
    else if (conn->request.close)
    {
        SSL_shutdown(conn->ssl);
        conn->line_buffer->size = 0;
        conn->state = STATE_READ_REQUEST;
    }
    else
    {
        conn->state = STATE_RESET;
    }

    return Ok;
}

static inline capy_err http_conn_write_response(http_conn *conn)
{
    capy_err err;

    ssize_t bytes_written = send(conn->socket,
                                 conn->response_buffer->data,
                                 conn->response_buffer->size,
                                 0);

    if (bytes_written == 0)
    {
        conn->state = STATE_CLOSE;
        return Ok;
    }
    else if (bytes_written == -1)
    {
        err = ErrStd(errno);

        if (err.code == EWOULDBLOCK || err.code == EAGAIN)
        {
            conn->after_unblock = STATE_WRITE_RESPONSE;
            conn->state = STATE_BLOCK;
            conn->want_write = true;
            return Ok;
        }

        if (err.code != ECONNRESET)
        {
            conn->state = STATE_CLOSE;
            return Ok;
        }

        return ErrWrap(err, "Failed to write response");
    }

    capy_buffer_shl(conn->response_buffer, Cast(size_t, bytes_written));

    if (conn->response_buffer->size > 0)
    {
        conn->state = STATE_WRITE_RESPONSE;
    }
    else if (conn->request.close)
    {
        conn->state = STATE_CLOSE;
    }
    else
    {
        conn->state = STATE_RESET;
    }

    return Ok;
}

static inline capy_err http_conn_parse_reqline(http_conn *conn)
{
    capy_string line;

    if (http_conn_parse_msgline(conn, &line))
    {
        conn->after_read = STATE_PARSE_REQLINE;
        conn->state = STATE_READ_REQUEST;
        return Ok;
    }

    capy_err err = capy_http_parse_reqline(conn->arena, &conn->request, line);

    if (err.code == EINVAL)
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }
    else if (err.code)
    {
        return ErrWrap(err, "Failed to parse request line");
    }

    http_conn_shl_msgline(conn);
    conn->state = STATE_PARSE_HEADERS;

    return Ok;
}

static inline capy_err http_conn_parse_headers(http_conn *conn)
{
    capy_err err;

    for (;;)
    {
        capy_string line;

        if (http_conn_parse_msgline(conn, &line))
        {
            conn->after_read = STATE_PARSE_HEADERS;
            conn->state = STATE_READ_REQUEST;
            return Ok;
        }

        if (line.size == 0)
        {
            http_conn_shl_msgline(conn);
            break;
        }

        err = capy_http_parse_field(conn->request.headers, line);

        if (err.code == EINVAL)
        {
            conn->state = STATE_BAD_REQUEST;
            return Ok;
        }
        else if (err.code)
        {
            return ErrWrap(err, "Failed to parse headers");
        }

        http_conn_shl_msgline(conn);
    }

    err = capy_http_validate_request(conn->arena, &conn->request);

    if (err.code == EINVAL)
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }
    else if (err.code)
    {
        return ErrWrap(err, "Failed to validate request");
    }

    if (conn->request.content_length)
    {
        conn->chunk_size = conn->request.content_length;
        conn->state = STATE_PARSE_CONTENT;
    }
    else if (conn->request.chunked)
    {
        conn->state = STATE_PARSE_CHUNKSIZE;
    }
    else
    {
        conn->state = STATE_ROUTE_REQUEST;
    }

    return Ok;
}

static inline capy_err http_conn_parse_trailers(http_conn *conn)
{
    capy_err err;

    for (;;)
    {
        capy_string line;

        if (http_conn_parse_msgline(conn, &line))
        {
            conn->after_read = STATE_PARSE_TRAILERS;
            conn->state = STATE_READ_REQUEST;
            return Ok;
        }

        if (line.size == 0)
        {
            http_conn_shl_msgline(conn);
            break;
        }

        err = capy_http_parse_field(conn->request.trailers, line);

        if (err.code == EINVAL)
        {
            conn->state = STATE_BAD_REQUEST;
            return Ok;
        }
        else if (err.code)
        {
            return ErrWrap(err, "Failed to parse trailers");
        }

        http_conn_shl_msgline(conn);
    }

    conn->state = STATE_ROUTE_REQUEST;
    return Ok;
}

static inline capy_err http_conn_parse_chunksize(http_conn *conn)
{
    capy_string line;

    if (http_conn_parse_msgline(conn, &line))
    {
        conn->after_read = STATE_PARSE_CHUNKSIZE;
        conn->state = STATE_READ_REQUEST;
        return Ok;
    }

    uint64_t value;

    // todo: validate chunck_size extensions
    if (capy_string_parse_hexdigits(&value, line) == 0)
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }

    conn->chunk_size = (size_t)(value);

    http_conn_shl_msgline(conn);

    if (conn->chunk_size == 0)
    {
        conn->state = STATE_PARSE_TRAILERS;
    }
    else
    {
        conn->state = STATE_PARSE_CHUNKDATA;
    }

    return Ok;
}

static inline capy_err http_conn_parse_chunkdata(http_conn *conn)
{
    capy_err err;

    size_t msg_size = conn->line_buffer->size;

    if (conn->chunk_size + 2 > msg_size)
    {
        err = capy_buffer_write_bytes(conn->content_buffer, msg_size, conn->line_buffer->data);

        if (err.code)
        {
            return ErrWrap(err, "Failed to read chunk data");
        }

        http_conn_shl_msg(conn, msg_size);

        conn->chunk_size -= msg_size;

        conn->after_read = STATE_PARSE_CHUNKDATA;
        conn->state = STATE_READ_REQUEST;
        return Ok;
    }

    const char *end = conn->line_buffer->data + conn->chunk_size;

    if (end[0] != '\r' || end[1] != '\n')
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }

    err = capy_buffer_write_bytes(conn->content_buffer, conn->chunk_size, conn->line_buffer->data);

    if (err.code)
    {
        return ErrWrap(err, "Failed to read chunk data");
    }

    http_conn_shl_msg(conn, conn->chunk_size + 2);

    conn->state = STATE_PARSE_CHUNKSIZE;
    return Ok;
}

static capy_err http_conn_read_request_ssl(http_conn *conn)
{
    capy_err err;

    size_t line_limit = conn->line_buffer->capacity;
    size_t line_size = conn->line_buffer->size;
    size_t bytes_wanted = line_limit - line_size;

    if (bytes_wanted == 0)
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }

    size_t bytes_read = 0;

    if (!SSL_read_ex(conn->ssl, conn->line_buffer->data + line_size, bytes_wanted, &bytes_read))
    {
        int ssl_err = SSL_get_error(conn->ssl, 0);

        switch (ssl_err)
        {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
            {
                conn->after_unblock = STATE_READ_REQUEST;
                conn->state = STATE_BLOCK;
                conn->want_write = (ssl_err == SSL_ERROR_WANT_WRITE);
                return Ok;
            }

            case SSL_ERROR_ZERO_RETURN:
            {
                conn->state = STATE_CLOSE;
                return Ok;
            }

            default:
            {
                return ErrFmt(ssl_err, "Failed to read from SSL socket (%d)", ssl_err);
            }
        }
    }

    if (bytes_read == 0)
    {
        conn->state = STATE_CLOSE;
        return Ok;
    }

    if ((err = capy_buffer_resize(conn->line_buffer, line_size + Cast(size_t, bytes_read))).code)
    {
        return err;
    }

    conn->state = conn->after_read;
    return Ok;
}

static capy_err http_conn_read_request(http_conn *conn)
{
    capy_err err;

    size_t line_limit = conn->line_buffer->capacity;
    size_t line_size = conn->line_buffer->size;
    size_t bytes_wanted = line_limit - line_size;

    if (bytes_wanted == 0)
    {
        conn->state = STATE_BAD_REQUEST;
        return Ok;
    }

    ssize_t bytes_read = recv(conn->socket, conn->line_buffer->data + line_size, bytes_wanted, 0);

    if (bytes_read == 0)
    {
        conn->state = STATE_CLOSE;
        return Ok;
    }

    if (bytes_read == -1)
    {
        err = ErrStd(errno);

        if (err.code == EWOULDBLOCK || err.code == EAGAIN)
        {
            conn->after_unblock = STATE_READ_REQUEST;
            conn->state = STATE_BLOCK;
            conn->want_write = false;
            return Ok;
        }

        if (err.code == ECONNRESET)
        {
            conn->state = STATE_CLOSE;
            return Ok;
        }

        return ErrWrap(err, "Failed to read socket");
    }

    if ((err = capy_buffer_resize(conn->line_buffer, line_size + Cast(size_t, bytes_read))).code)
    {
        return err;
    }

    conn->state = conn->after_read;
    return Ok;
}

static inline capy_err http_conn_parse_reqbody(http_conn *conn)
{
    capy_err err;

    size_t message_size = conn->line_buffer->size;

    if (conn->chunk_size > message_size)
    {
        err = capy_buffer_write_bytes(conn->content_buffer, message_size, conn->line_buffer->data);

        if (err.code)
        {
            return ErrWrap(err, "Failed to read content data");
        }

        http_conn_shl_msg(conn, message_size);

        conn->chunk_size -= message_size;

        conn->after_read = STATE_PARSE_CONTENT;
        conn->state = STATE_READ_REQUEST;
        return Ok;
    }

    err = capy_buffer_write_bytes(conn->content_buffer, conn->chunk_size, conn->line_buffer->data);

    if (err.code)
    {
        return ErrWrap(err, "Failed to read content data");
    }

    http_conn_shl_msg(conn, conn->chunk_size);

    conn->state = STATE_ROUTE_REQUEST;
    return Ok;
}

static inline capy_err http_conn_prepare_badrequest(http_conn *conn)
{
    capy_err err;

    conn->request.close = true;
    conn->response.status = CAPY_HTTP_BAD_REQUEST;

    if ((err = http_response_status(&conn->response)).code)
    {
        return ErrWrap(err, "Failed to generate BAD_REQUEST");
    }

    if (capy_http_write_response(conn->response_buffer, &conn->response, true).code)
    {
        return ErrWrap(err, "Failed to write to response_buffer");
    }

    return Ok;
}

static inline capy_err http_conn_route_request(http_conn *conn)
{
    capy_err err;

    if ((err = capy_buffer_write_null(conn->content_buffer)).code)
    {
        return ErrWrap(err, "Failed to write null terminator");
    }

    conn->request.content = capy_string_bytes(conn->content_buffer->size, conn->content_buffer->data);

    if ((err = capy_http_router_handle(conn->arena, conn->router, &conn->request, &conn->response)).code)
    {
        return ErrWrap(err, "Failed to handle request");
    }

    if ((err = capy_http_write_response(conn->response_buffer, &conn->response, conn->request.close)).code)
    {
        return ErrWrap(err, "Failed to write to response_buffer");
    }

    conn->state = STATE_WRITE_RESPONSE;
    return Ok;
}

static capy_err http_conn_reset(http_conn *conn)
{
    capy_err err;

    if ((err = capy_arena_free(conn->arena, conn->marker_init)).code)
    {
        return err;
    }

    conn->line_buffer->size = 0;
    conn->line_cursor = 2;
    conn->after_read = STATE_UNKNOWN;
    conn->after_unblock = STATE_UNKNOWN;

    conn->request = (capy_httpreq){
        .headers = capy_strkvnmap_init(conn->arena, 16),
        .trailers = capy_strkvnmap_init(conn->arena, 4),
        .params = capy_strkvnmap_init(conn->arena, 8),
        .query = capy_strkvnmap_init(conn->arena, 8),
    };

    conn->response = (capy_httpresp){
        .headers = capy_strkvnmap_init(conn->arena, 16),
        .body = capy_buffer_init(conn->arena, 256),
    };

    conn->content_buffer = capy_buffer_init(conn->arena, 256);
    conn->response_buffer = capy_buffer_init(conn->arena, 512);

    conn->mem_headers = 0;
    conn->mem_content = 0;
    conn->mem_trailers = 0;
    conn->mem_response = 0;

    conn->state = STATE_PARSE_REQLINE;

    return Ok;
}

static inline void http_conn_trace(http_conn *conn, http_worker *worker)
{
    struct timespec timestamp;
    timespec_get(&timestamp, TIME_UTC);

    int64_t elapsed = timespec_diff(timestamp, conn->timestamp);
    const char *elapsed_unit;
    nanoseconds_normalize(&elapsed, &elapsed_unit);

    conn->timestamp = timestamp;

    size_t mem_total = capy_arena_size(conn->arena);
    size_t to_read = (conn->line_buffer) ? conn->line_buffer->size : 0;
    size_t to_write = (conn->response_buffer) ? conn->response_buffer->size : 0;

    LogDbg("worker: %-24s | WK:%-4zu SK:%-4d MH:%-8zu MC:%-8zu MT:%-8zu MR:%-8zu MA:%-8zu RS:%-8zu WS:%-8zu | %3" PRIi64 " %s",
           http_conn_state_cstr[conn->state], worker->id, conn->socket,
           conn->mem_headers, conn->mem_content, conn->mem_trailers, conn->mem_response, mem_total,
           to_read, to_write, elapsed, elapsed_unit);
}

static inline capy_err http_conn_step(http_conn *conn, http_worker *worker)
{
    capy_assert(conn != NULL);

    capy_err err;

    if (conn->state == STATE_BLOCK)
    {
        conn->state = conn->after_unblock;
    }

    for (;;)
    {
        http_conn_trace(conn, worker);

        ssize_t begin = (ssize_t)capy_arena_size(conn->arena);

        switch (conn->state)
        {
            case STATE_RESET:
            {
                err = http_conn_reset(conn);
            }
            break;

            case STATE_READ_REQUEST:
            {
                if (conn->options->protocol == CAPY_HTTPS)
                {
                    err = http_conn_read_request_ssl(conn);
                }
                else
                {
                    err = http_conn_read_request(conn);
                }
            }
            break;

            case STATE_PARSE_REQLINE:
            {
                err = http_conn_parse_reqline(conn);
                conn->mem_headers += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_HEADERS:
            {
                err = http_conn_parse_headers(conn);
                conn->mem_headers += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_CONTENT:
            {
                err = http_conn_parse_reqbody(conn);
                conn->mem_content += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_CHUNKSIZE:
            {
                err = http_conn_parse_chunksize(conn);
                conn->mem_content += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_CHUNKDATA:
            {
                err = http_conn_parse_chunkdata(conn);
                conn->mem_content += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_TRAILERS:
            {
                err = http_conn_parse_trailers(conn);
                conn->mem_trailers += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_ROUTE_REQUEST:
            {
                err = http_conn_route_request(conn);
                conn->mem_response += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_BAD_REQUEST:
            {
                err = http_conn_prepare_badrequest(conn);
                conn->mem_response += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_WRITE_RESPONSE:
            {
                if (conn->options->protocol == CAPY_HTTPS)
                {
                    err = http_conn_write_response_ssl(conn);
                }
                else
                {
                    err = http_conn_write_response(conn);
                }
            }
            break;

            case STATE_BLOCK:
            {
                return http_conn_block(conn, worker);
            }
            break;

            case STATE_CLOSE:
            {
                return http_conn_close(conn);
            }

            default:
            {
                capy_assert(false);
            }
        }

        if (err.code)
        {
            LogErr("While processing request: %s", err.msg);
            return http_conn_close(conn);
        }
    }
}

static void *conn_worker(void *data)
{
    capy_err err;

    http_worker *worker = data;

    struct epoll_event events[1];

    for (;;)
    {
        int events_count = epoll_pwait(worker->epoll_fd, events, 1, -1, worker->mask);

        if (events_count == -1)
        {
            err = ErrStd(errno);

            if (err.code == EINTR)
            {
                continue;
            }

            LogErr("Failed to receive events from epoll (worker): %s", err.msg);
            abort();
        }

        for (int i = 0; i < events_count; i++)
        {
            if (events[i].data.u64 == SIGNAL_EPOLL_EVENT)
            {
                return 0;
            }

            http_conn *conn = events[i].data.ptr;

            pthread_mutex_lock(&conn->mut);

            if (conn->socket)
            {
                if ((err = http_conn_step(conn, worker)).code)
                {
                    LogErr("While processing request: %s", err.msg);
                    abort();
                }
            }

            pthread_mutex_unlock(&conn->mut);
        }
    }
}

static capy_httpserveropt http_server_options_with_defaults(capy_httpserveropt options)
{
    if (!options.workers)
    {
        options.workers = (size_t)(sysconf(_SC_NPROCESSORS_CONF)) * 2;
    }

    if (!options.connections)
    {
        options.connections = 256;
    }

    if (options.host == NULL)
    {
        options.host = "127.0.0.1";
    }

    if (options.port == NULL)
    {
        options.port = "8080";
    }

    if (!options.line_buffer_size)
    {
        options.line_buffer_size = KiB(8);
    }

    if (!options.mem_connection_max)
    {
        options.mem_connection_max = MiB(2);
    }

    return options;
}

static capy_err http_server_bind(http_server *server)
{
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    struct addrinfo *addresses;

    capy_err err = http_gaierror(getaddrinfo(server->options.host, server->options.port, &hints, &addresses));

    if (err.code)
    {
        return err;
    }

    for (struct addrinfo *address = addresses; address != NULL; address = address->ai_next)
    {
        err = Ok;

        int fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

        if (fd == -1)
        {
            err = ErrStd(errno);
            continue;
        }

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) == -1)
        {
            err = ErrStd(errno);
            break;
        }

        if (bind(fd, address->ai_addr, address->ai_addrlen) == -1)
        {
            err = ErrStd(errno);
            continue;
        }

        server->fd = fd;
        break;
    }

    freeaddrinfo(addresses);

    return err;
}

static capy_err http_server_create_signal_fd(http_server *server)
{
    sigemptyset(&server->signals);
    sigaddset(&server->signals, SIGINT);
    sigaddset(&server->signals, SIGQUIT);
    sigprocmask(SIG_BLOCK, &server->signals, NULL);

    server->signal_fd = signalfd(-1, &server->signals, 0);

    if (server->signal_fd == -1)
    {
        return ErrStd(errno);
    }

    return Ok;
}

static capy_err http_server_create_epoll(http_server *server)
{
    struct epoll_event event;

    server->epoll_fd = epoll_create1(0);

    if (server->epoll_fd == -1)
    {
        return ErrStd(errno);
    }

    event = (struct epoll_event){
        .events = EPOLLIN,
        .data.u64 = SIGNAL_EPOLL_EVENT,
    };

    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->signal_fd, &event) == -1)
    {
        return ErrStd(errno);
    }

    event = (struct epoll_event){
        .events = EPOLLIN,
        .data.u64 = 0,
    };

    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->fd, &event) == -1)
    {
        return ErrStd(errno);
    }

    return Ok;
}

static capy_err http_server_create_workers_epoll(http_server *server)
{
    struct epoll_event event;

    server->workers_epoll_fd = epoll_create1(0);

    if (server->workers_epoll_fd == -1)
    {
        return ErrStd(errno);
    }

    event = (struct epoll_event){
        .events = EPOLLIN,
        .data.u64 = SIGNAL_EPOLL_EVENT,
    };

    if (epoll_ctl(server->workers_epoll_fd, EPOLL_CTL_ADD, server->signal_fd, &event) == -1)
    {
        return ErrStd(errno);
    }

    return Ok;
}

static capy_err http_server_create_connections(http_server *server)
{
    server->connections = Make(server->arena, http_conn *, server->options.connections);

    if (server->connections == NULL)
    {
        return ErrStd(ENOMEM);
    }

    for (size_t i = 0; i < server->options.connections; i++)
    {
        capy_arena *arena = capy_arena_init(server->options.line_buffer_size + KiB(4),
                                            server->options.mem_connection_max);

        if (arena == NULL)
        {
            return ErrStd(ENOMEM);
        }

        http_conn *conn = Make(arena, http_conn, 1);

        conn->arena = arena;
        conn->router = server->router;
        conn->state = STATE_CLOSE;
        conn->options = &server->options;
        conn->line_buffer = capy_buffer_init(arena, server->options.line_buffer_size);
        conn->line_buffer->arena = NULL;
        conn->marker_init = capy_arena_end(arena);

        pthread_mutex_init(&conn->mut, NULL);

        server->connections[i] = conn;
    }

    return Ok;
}

static capy_err http_server_create_workers(http_server *server)
{
    server->workers = Make(server->arena, http_worker, server->options.workers);

    if (server->workers == NULL)
    {
        return ErrStd(ENOMEM);
    }

    for (size_t i = 0; i < server->options.workers; i++)
    {
        server->workers[i] = (http_worker){
            .id = i,
            .epoll_fd = server->workers_epoll_fd,
            .mask = &server->signals,
        };

        int err = pthread_create(&server->workers[i].thread_id, NULL, conn_worker, &server->workers[i]);

        if (err)
        {
            return ErrStd(err);
        }
    }

    return Ok;
}

static capy_err http_server_init_ssl(http_server *server)
{
    static const char cache_id[] = "capy-https-server";

    server->ssl_ctx = SSL_CTX_new(TLS_server_method());

    if (server->ssl_ctx == NULL)
    {
        return ErrFmt(EINVAL, "Failed to create server SSL_CTX");
    }

    if (!SSL_CTX_set_min_proto_version(server->ssl_ctx, TLS1_2_VERSION))
    {
        SSL_CTX_free(server->ssl_ctx);
        return ErrFmt(EINVAL, "Failed to set the minimum TLS protocol version");
    }

    uint64_t opts = SSL_OP_IGNORE_UNEXPECTED_EOF | SSL_OP_NO_RENEGOTIATION | SSL_OP_CIPHER_SERVER_PREFERENCE;

    SSL_CTX_set_options(server->ssl_ctx, opts);

    if (SSL_CTX_use_certificate_chain_file(server->ssl_ctx, server->options.certificate_chain) <= 0)
    {
        SSL_CTX_free(server->ssl_ctx);
        return ErrFmt(EINVAL, "Failed to load the server certificate chain file");
    }

    if (SSL_CTX_use_PrivateKey_file(server->ssl_ctx, server->options.certificate_key, SSL_FILETYPE_PEM) <= 0)
    {
        SSL_CTX_free(server->ssl_ctx);
        return ErrFmt(EINVAL, "Failed to load the server private key file");
    }

    SSL_CTX_set_session_id_context(server->ssl_ctx, (void *)cache_id, sizeof(cache_id));
    SSL_CTX_set_session_cache_mode(server->ssl_ctx, SSL_SESS_CACHE_SERVER);

    SSL_CTX_sess_set_cache_size(server->ssl_ctx, 1024);
    SSL_CTX_set_timeout(server->ssl_ctx, 3600);
    SSL_CTX_set_verify(server->ssl_ctx, SSL_VERIFY_NONE, NULL);

    return Ok;
}

static void http_server_update_connections(http_server *server)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    size_t total = server->active_connections;

    for (size_t i = 0; i < total; i++)
    {
        http_conn *conn = server->connections[i];

        if (pthread_mutex_trylock(&conn->mut) == EBUSY)
        {
            continue;
        }

        if (conn->state == STATE_BLOCK)
        {
            if (difftime(ts.tv_sec, conn->timestamp.tv_sec) > 15)
            {
                LogInf("server: connection (%d) closed due to inactivity", conn->socket);
                http_conn_close(conn);
            }
        }

        pthread_mutex_unlock(&conn->mut);
    }

    size_t active = 0;

    for (size_t i = 0; i < total; i++)
    {
        http_conn *conn = server->connections[i];

        if (conn->socket == 0)
        {
            continue;
        }

        server->connections[i] = server->connections[active];
        server->connections[active] = conn;

        active += 1;
    }

    server->active_connections = active;
}

static void http_server_destroy(http_server *server)
{
    for (size_t i = 0; i < server->options.workers; i++)
    {
        pthread_join(server->workers[i].thread_id, NULL);
    }

    for (size_t i = 0; i < server->options.connections; i++)
    {
        http_conn *conn = server->connections[i];

        if (conn->socket != 0)
        {
            http_conn_close(conn);
        }

        pthread_mutex_destroy(&conn->mut);
        capy_arena_destroy(conn->arena);
    }

    SSL_CTX_free(server->ssl_ctx);
    close(server->signal_fd);
    close(server->fd);
    close(server->epoll_fd);
    close(server->workers_epoll_fd);

    capy_arena_destroy(server->arena);
}

capy_err capy_http_serve(capy_httpserveropt options)
{
    capy_err err;

    capy_arena *arena = capy_arena_init(0, MiB(1));

    if (arena == NULL)
    {
        return ErrFmt(ENOMEM, "Failed to create server arena");
    }

    capy_httprouter *router = capy_http_router_init(arena, options.routes_size, options.routes);

    if (router == NULL)
    {
        return ErrFmt(ENOMEM, "Failed to create router");
    }

    http_server server = {
        .router = router,
        .arena = arena,
        .options = http_server_options_with_defaults(options),
    };

    if ((err = http_server_bind(&server)).code)
    {
        return ErrWrap(err, "Failed to bind server");
    }

    if ((err = http_server_create_signal_fd(&server)).code)
    {
        return ErrWrap(err, "Failed to create signal file descriptors");
    }

    if ((err = http_server_create_epoll(&server)).code)
    {
        return ErrWrap(err, "Failed to create listen epoll");
    }

    if ((err = http_server_create_workers_epoll(&server)).code)
    {
        return ErrWrap(err, "Failed to create listen epoll");
    }

    if ((err = http_server_create_connections(&server)).code)
    {
        return ErrWrap(err, "Failed to create connection pool");
    }

    if ((err = http_server_create_workers(&server)).code)
    {
        return ErrWrap(err, "Failed to create workers pool");
    }

    if (options.protocol == CAPY_HTTPS)
    {
        if ((err = http_server_init_ssl(&server)).code)
        {
            return ErrWrap(err, "Failed to initialize SSL context");
        }
    }

    if (listen(server.fd, 10) == -1)
    {
        return ErrWrap(ErrStd(errno), "Failed to listen");
    }

    LogInf("server: listening at %s:%s (workers = %lu, connections = %lu)",
           server.options.host, server.options.port,
           server.options.workers, server.options.connections);

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    int keepcnt = 4;
    int keepidle = 60;
    int keepintvl = 15;

    unsigned int timeout = Cast(unsigned int, (keepidle + (keepcnt * keepintvl)) * 1000);

    struct epoll_event events[10];

    for (int stop = false; !stop;)
    {
        int events_count = epoll_pwait(server.epoll_fd, events, 10, 5000, &server.signals);

        if (events_count == -1)
        {
            err = ErrStd(errno);

            if (err.code == EINTR)
            {
                continue;
            }

            return ErrWrap(err, "Failed to receive events from epoll");
        }

        http_server_update_connections(&server);

        for (int i = 0; i < events_count; i++)
        {
            if (events[i].data.u64 == SIGNAL_EPOLL_EVENT)
            {
                stop = true;
                break;
            }

            int conn_fd = accept4(server.fd, address_buffer, &address_buffer_size, SOCK_NONBLOCK);

            if (conn_fd == -1)
            {
                return ErrWrap(ErrStd(errno), "Failed to accept connection");
            }

            if (server.active_connections >= server.options.connections)
            {
                close(conn_fd);
                continue;
            }

            if (setsockopt(conn_fd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int)) == -1)
            {
                return ErrWrap(ErrStd(errno), "Failed to set SO_KEEPALIVE value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt)) == -1)
            {
                return ErrWrap(ErrStd(errno), "Failed to set TCP_KEEPCNT value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)) == -1)
            {
                return ErrWrap(ErrStd(errno), "Failed to set TCP_KEEPIDLE value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl)) == -1)
            {
                return ErrWrap(ErrStd(errno), "Failed to set TCP_KEEPINTVL value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &timeout, sizeof(timeout)) == -1)
            {
                return ErrWrap(ErrStd(errno), "Failed to set TCP_USER_TIMEOUT value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int)) == -1)
            {
                return ErrWrap(ErrStd(errno), "Failed to set TCP_NODELAY value");
            }

            http_conn *conn = server.connections[server.active_connections++];

            conn->socket = conn_fd;
            conn->state = STATE_RESET;
            timespec_get(&conn->timestamp, TIME_UTC);

            if (options.protocol == CAPY_HTTPS)
            {
                SSL *ssl = SSL_new(server.ssl_ctx);
                SSL_set_fd(ssl, conn->socket);
                SSL_set_accept_state(ssl);
                conn->ssl = ssl;
            }

            if ((err = http_conn_update_epoll(conn, server.workers_epoll_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET | EPOLLONESHOT)).code)
            {
                return err;
            }
        }
    }

    http_server_destroy(&server);

    LogInf("server: shutting down");

    return Ok;
}
