#include <arpa/inet.h>
#include <capy/capy.h>
#include <capy/macros.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.c"

#define SIGNAL_EPOLL_EVENT -1ULL

typedef struct http_worker
{
    size_t id;
    pthread_t thread_id;
    int epoll_fd;
    sigset_t *mask;
} http_worker;

typedef enum
{
    STATE_UNKNOWN,
    STATE_INIT,
    STATE_READ_SOCKET,
    STATE_PARSE_REQUEST_LINE,
    STATE_PARSE_HEADERS,
    STATE_PARSE_CONTENT,
    STATE_PARSE_CHUNK_SIZE,
    STATE_PARSE_CHUNK_DATA,
    STATE_PARSE_TRAILERS,
    STATE_PROCESS_REQUEST,
    STATE_WRITE_RESPONSE,
    STATE_BAD_REQUEST,
    STATE_SERVER_FAILURE,
    STATE_CLOSED,
    STATE_BLOCKED,
} http_conn_state;

static const char *http_conn_state_cstr[] = {
    [STATE_UNKNOWN] = "STATE_UNKNOWN",
    [STATE_INIT] = "STATE_INIT",
    [STATE_READ_SOCKET] = "STATE_READ_SOCKET",
    [STATE_PARSE_REQUEST_LINE] = "STATE_PARSE_REQUEST_LINE",
    [STATE_PARSE_HEADERS] = "STATE_PARSE_HEADERS",
    [STATE_PARSE_CONTENT] = "STATE_PARSE_CONTENT",
    [STATE_PARSE_CHUNK_SIZE] = "STATE_PARSE_CHUNK_SIZE",
    [STATE_PARSE_CHUNK_DATA] = "STATE_PARSE_CHUNK_DATA",
    [STATE_PARSE_TRAILERS] = "STATE_PARSE_TRAILERS",
    [STATE_PROCESS_REQUEST] = "STATE_PROCESS_REQUEST",
    [STATE_WRITE_RESPONSE] = "STATE_WRITE_RESPONSE",
    [STATE_BAD_REQUEST] = "STATE_BAD_REQUEST",
    [STATE_SERVER_FAILURE] = "STATE_SERVER_FAILURE",
    [STATE_CLOSED] = "STATE_CLOSED",
    [STATE_BLOCKED] = "STATE_BLOCKED",
};

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

    capy_http_request request;
    capy_http_response response;

    capy_http_router *router;

    capy_http_server_options *options;

    struct timespec timestamp;
} http_conn;

static capy_err http_gaierror(int err)
{
    if (err == 0) return ok;
    return (capy_err){.code = err, .msg = gai_strerror(err)};
}

static capy_err conn_epoll(http_conn *conn, int epoll_fd, int op, uint32_t events)
{
    struct epoll_event client_event = {
        .events = events,
        .data.ptr = conn,
    };

    if (epoll_ctl(epoll_fd, op, conn->socket, &client_event) == -1)
    {
        return errwrap(capy_errno(errno), "Failed to update workers epoll");
    }

    return ok;
}

static capy_err conn_blocked(http_conn *conn, http_worker *worker)
{
    timespec_get(&conn->timestamp, TIME_UTC);

    uint32_t event = EPOLLIN;

    if (conn->after_unblock == STATE_WRITE_RESPONSE)
    {
        event = EPOLLOUT;
    }

    return conn_epoll(conn, worker->epoll_fd, EPOLL_CTL_MOD, event | EPOLLET | EPOLLONESHOT);
}

static capy_err conn_close(http_conn *conn)
{
    close(conn->socket);
    conn->state = STATE_CLOSED;
    conn->socket = 0;
    return ok;
}

static inline int conn_get_line(http_conn *conn, capy_string *line)
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

static inline void conn_consume_bytes(http_conn *conn, size_t size)
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

static inline void conn_consume_line(http_conn *conn)
{
    conn_consume_bytes(conn, conn->line_cursor);
}

static inline capy_err conn_write_response(http_conn *conn)
{
    capy_err err;

    ssize_t bytes_written = send(conn->socket,
                                 conn->response_buffer->data,
                                 conn->response_buffer->size,
                                 0);

    if (bytes_written == 0)
    {
        conn->state = STATE_CLOSED;
        return ok;
    }
    else if (bytes_written == -1)
    {
        err = capy_errno(errno);

        if (err.code == EWOULDBLOCK || err.code == EAGAIN)
        {
            conn->after_unblock = STATE_WRITE_RESPONSE;
            conn->state = STATE_BLOCKED;
            return ok;
        }

        if (err.code != ECONNRESET)
        {
            conn->state = STATE_CLOSED;
            return ok;
        }

        return errwrap(err, "Failed to write response");
    }

    capy_buffer_shl(conn->response_buffer, cast(size_t, bytes_written));

    if (conn->response_buffer->size > 0)
    {
        conn->state = STATE_WRITE_RESPONSE;
    }
    else if (conn->request.close)
    {
        conn->state = STATE_CLOSED;
    }
    else
    {
        conn->state = STATE_INIT;
    }

    return ok;
}

static inline capy_err conn_parse_request_line(http_conn *conn)
{
    capy_string line;

    if (conn_get_line(conn, &line))
    {
        conn->after_read = STATE_PARSE_REQUEST_LINE;
        conn->state = STATE_READ_SOCKET;
        return ok;
    }

    capy_err err = capy_http_parse_reqline(conn->arena, &conn->request, line);

    if (err.code == EINVAL)
    {
        conn->state = STATE_BAD_REQUEST;
        return ok;
    }
    else if (err.code)
    {
        return errwrap(err, "Failed to parse request line");
    }

    conn_consume_line(conn);
    conn->state = STATE_PARSE_HEADERS;

    return ok;
}

static inline capy_err conn_parse_headers(http_conn *conn)
{
    capy_err err;

    for (;;)
    {
        capy_string line;

        if (conn_get_line(conn, &line))
        {
            conn->after_read = STATE_PARSE_HEADERS;
            conn->state = STATE_READ_SOCKET;
            return ok;
        }

        if (line.size == 0)
        {
            conn_consume_line(conn);
            break;
        }

        err = capy_http_parse_field(conn->request.headers, line);

        if (err.code == EINVAL)
        {
            conn->state = STATE_BAD_REQUEST;
            return ok;
        }
        else if (err.code)
        {
            return errwrap(err, "Failed to parse headers");
        }

        conn_consume_line(conn);
    }

    err = capy_http_request_validate(conn->arena, &conn->request);

    if (err.code == EINVAL)
    {
        conn->state = STATE_BAD_REQUEST;
        return ok;
    }
    else if (err.code)
    {
        return errwrap(err, "Failed to validate request");
    }

    if (conn->request.content_length)
    {
        conn->chunk_size = conn->request.content_length;
        conn->state = STATE_PARSE_CONTENT;
    }
    else if (conn->request.chunked)
    {
        conn->state = STATE_PARSE_CHUNK_SIZE;
    }
    else
    {
        conn->state = STATE_PROCESS_REQUEST;
    }

    return ok;
}

static inline capy_err conn_parse_trailers(http_conn *conn)
{
    capy_err err;

    for (;;)
    {
        capy_string line;

        if (conn_get_line(conn, &line))
        {
            conn->after_read = STATE_PARSE_TRAILERS;
            conn->state = STATE_READ_SOCKET;
            return ok;
        }

        if (line.size == 0)
        {
            conn_consume_line(conn);
            break;
        }

        err = capy_http_parse_field(conn->request.trailers, line);

        if (err.code == EINVAL)
        {
            conn->state = STATE_BAD_REQUEST;
            return ok;
        }
        else if (err.code)
        {
            return errwrap(err, "Failed to parse trailers");
        }

        conn_consume_line(conn);
    }

    conn->state = STATE_PROCESS_REQUEST;
    return ok;
}

static inline capy_err conn_parse_chunk_size(http_conn *conn)
{
    capy_string line;

    if (conn_get_line(conn, &line))
    {
        conn->after_read = STATE_PARSE_CHUNK_SIZE;
        conn->state = STATE_READ_SOCKET;
        return ok;
    }

    int64_t value;

    // todo: validate chunck_size extensions
    if (capy_string_hex(line, &value) == 0)
    {
        conn->state = STATE_BAD_REQUEST;
        return ok;
    }

    conn->chunk_size = (size_t)(value);

    conn_consume_line(conn);

    if (conn->chunk_size == 0)
    {
        conn->state = STATE_PARSE_TRAILERS;
    }
    else
    {
        conn->state = STATE_PARSE_CHUNK_DATA;
    }

    return ok;
}

static inline capy_err conn_parse_chunk_data(http_conn *conn)
{
    capy_err err;

    size_t msg_size = conn->line_buffer->size;

    if (conn->chunk_size + 2 > msg_size)
    {
        err = capy_buffer_wbytes(conn->content_buffer, msg_size, conn->line_buffer->data);

        if (err.code)
        {
            return errwrap(err, "Failed to read chunk data");
        }

        conn_consume_bytes(conn, msg_size);

        conn->chunk_size -= msg_size;

        conn->after_read = STATE_PARSE_CHUNK_DATA;
        conn->state = STATE_READ_SOCKET;
        return ok;
    }

    const char *end = conn->line_buffer->data + conn->chunk_size;

    if (end[0] != '\r' || end[1] != '\n')
    {
        conn->state = STATE_BAD_REQUEST;
        return ok;
    }

    err = capy_buffer_wbytes(conn->content_buffer, conn->chunk_size, conn->line_buffer->data);

    if (err.code)
    {
        return errwrap(err, "Failed to read chunk data");
    }

    conn_consume_bytes(conn, conn->chunk_size + 2);

    conn->state = STATE_PARSE_CHUNK_SIZE;
    return ok;
}

static capy_err conn_read_socket(http_conn *conn)
{
    capy_err err;

    size_t line_limit = conn->line_buffer->capacity;
    size_t line_size = conn->line_buffer->size;
    size_t bytes_wanted = line_limit - line_size;

    if (bytes_wanted == 0)
    {
        conn->state = STATE_BAD_REQUEST;
        return ok;
    }

    ssize_t bytes_read = recv(conn->socket, conn->line_buffer->data + line_size, bytes_wanted, 0);

    if (bytes_read == 0)
    {
        conn->state = STATE_CLOSED;
        return ok;
    }

    if (bytes_read == -1)
    {
        err = capy_errno(errno);

        if (err.code == EWOULDBLOCK || err.code == EAGAIN)
        {
            conn->after_unblock = STATE_READ_SOCKET;
            conn->state = STATE_BLOCKED;
            return ok;
        }

        if (err.code == ECONNRESET)
        {
            conn->state = STATE_CLOSED;
            return ok;
        }

        return errwrap(err, "Failed to read socket");
    }

    if ((err = capy_buffer_resize(conn->line_buffer, line_size + cast(size_t, bytes_read))).code)
    {
        return err;
    }

    conn->state = conn->after_read;
    return ok;
}

static inline capy_err conn_parse_content(http_conn *conn)
{
    capy_err err;

    size_t message_size = conn->line_buffer->size;

    if (conn->chunk_size > message_size)
    {
        err = capy_buffer_wbytes(conn->content_buffer, message_size, conn->line_buffer->data);

        if (err.code)
        {
            return errwrap(err, "Failed to read content data");
        }

        conn_consume_bytes(conn, message_size);

        conn->chunk_size -= message_size;

        conn->after_read = STATE_PARSE_CONTENT;
        conn->state = STATE_READ_SOCKET;
        return ok;
    }

    err = capy_buffer_wbytes(conn->content_buffer, conn->chunk_size, conn->line_buffer->data);

    if (err.code)
    {
        return errwrap(err, "Failed to read content data");
    }

    conn_consume_bytes(conn, conn->chunk_size);

    conn->state = STATE_PROCESS_REQUEST;
    return ok;
}

static inline capy_err conn_bad_request(http_conn *conn)
{
    capy_err err;

    conn->request.close = true;
    conn->response.status = CAPY_HTTP_BAD_REQUEST;

    if ((err = http_response_status(&conn->response)).code)
    {
        return errwrap(err, "Failed to generate BAD_REQUEST");
    }

    if (capy_http_write_response(conn->response_buffer, &conn->response, true).code)
    {
        return errwrap(err, "Failed to write to response_buffer");
    }

    return ok;
}

static inline capy_err conn_process_request(http_conn *conn)
{
    capy_err err;

    conn->request.content = conn->content_buffer->data;
    conn->request.content_length = conn->content_buffer->size;

    if ((err = capy_http_router_handle(conn->arena, conn->router, &conn->request, &conn->response)).code)
    {
        return errwrap(err, "Failed to handle request");
    }

    if ((err = capy_http_write_response(conn->response_buffer, &conn->response, conn->request.close)).code)
    {
        return errwrap(err, "Failed to write to response_buffer");
    }

    conn->state = STATE_WRITE_RESPONSE;
    return ok;
}

static capy_err conn_message_init(http_conn *conn)
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

    conn->request = (capy_http_request){
        .headers = capy_strkvmmap_init(conn->arena, 16),
        .trailers = capy_strkvmmap_init(conn->arena, 4),
        .params = capy_strkvmmap_init(conn->arena, 8),
        .query = capy_strkvmmap_init(conn->arena, 8),
    };

    conn->response = (capy_http_response){
        .headers = capy_strkvmmap_init(conn->arena, 16),
        .body = capy_buffer_init(conn->arena, 256),
    };

    conn->content_buffer = capy_buffer_init(conn->arena, 256);
    conn->response_buffer = capy_buffer_init(conn->arena, 512);

    conn->mem_headers = 0;
    conn->mem_content = 0;
    conn->mem_trailers = 0;
    conn->mem_response = 0;

    conn->state = STATE_PARSE_REQUEST_LINE;

    return ok;
}

static inline void conn_trace(http_conn *conn, http_worker *worker)
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

    capy_logdbg("worker: %-24s | WK:%-4zu SK:%-4d MH:%-8zu MC:%-8zu MT:%-8zu MR:%-8zu MA:%-8zu RS:%-8zu WS:%-8zu | %3" PRIi64 " %s",
                http_conn_state_cstr[conn->state], worker->id, conn->socket,
                conn->mem_headers, conn->mem_content, conn->mem_trailers, conn->mem_response, mem_total,
                to_read, to_write, elapsed, elapsed_unit);
}

static inline capy_err conn_state_machine(http_conn *conn, http_worker *worker)
{
    capy_assert(conn != NULL);

    capy_err err;

    if (conn->state == STATE_BLOCKED)
    {
        conn->state = conn->after_unblock;
    }

    for (;;)
    {
        conn_trace(conn, worker);

        ssize_t begin = (ssize_t)capy_arena_size(conn->arena);

        switch (conn->state)
        {
            case STATE_INIT:
            {
                err = conn_message_init(conn);
            }
            break;

            case STATE_READ_SOCKET:
            {
                err = conn_read_socket(conn);
            }
            break;

            case STATE_PARSE_REQUEST_LINE:
            {
                err = conn_parse_request_line(conn);
                conn->mem_headers += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_HEADERS:
            {
                err = conn_parse_headers(conn);
                conn->mem_headers += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_CONTENT:
            {
                err = conn_parse_content(conn);
                conn->mem_content += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_CHUNK_SIZE:
            {
                err = conn_parse_chunk_size(conn);
                conn->mem_content += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_CHUNK_DATA:
            {
                err = conn_parse_chunk_data(conn);
                conn->mem_content += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PARSE_TRAILERS:
            {
                err = conn_parse_trailers(conn);
                conn->mem_trailers += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_PROCESS_REQUEST:
            {
                err = conn_process_request(conn);
                conn->mem_response += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_BAD_REQUEST:
            {
                err = conn_bad_request(conn);
                conn->mem_response += (ssize_t)capy_arena_size(conn->arena) - begin;
            }
            break;

            case STATE_WRITE_RESPONSE:
            {
                err = conn_write_response(conn);
            }
            break;

            case STATE_BLOCKED:
            {
                return conn_blocked(conn, worker);
            }
            break;

            case STATE_CLOSED:
            {
                return conn_close(conn);
            }

            default:
            {
                capy_assert(false);
            }
        }

        if (err.code)
        {
            logerr("While processing request: %s", err.msg);
            return conn_close(conn);
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
            err = capy_errno(errno);

            if (err.code == EINTR)
            {
                continue;
            }

            capy_logerr("Failed to receive events from epoll (worker): %s", err.msg);
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
                if ((err = conn_state_machine(conn, worker)).code)
                {
                    capy_logerr("While processing request: %s", err.msg);
                    abort();
                }
            }

            pthread_mutex_unlock(&conn->mut);
        }
    }
}

static capy_http_server_options http_server_options_with_defaults(capy_http_server_options options)
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

typedef struct http_server
{
    capy_arena *arena;
    capy_http_router *router;

    int fd;
    int epoll_fd;
    int workers_epoll_fd;
    int signal_fd;
    sigset_t signals;

    http_worker *workers;
    http_conn **connections;
    size_t active_connections;

    capy_http_server_options options;
} http_server;

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
        err = ok;

        int fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

        if (fd == -1)
        {
            err = capy_errno(errno);
            continue;
        }

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) == -1)
        {
            err = capy_errno(errno);
            break;
        }

        if (bind(fd, address->ai_addr, address->ai_addrlen) == -1)
        {
            err = capy_errno(errno);
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
        return capy_errno(errno);
    }

    return ok;
}

static capy_err http_server_create_epoll(http_server *server)
{
    struct epoll_event event;

    server->epoll_fd = epoll_create1(0);

    if (server->epoll_fd == -1)
    {
        return capy_errno(errno);
    }

    event = (struct epoll_event){
        .events = EPOLLIN,
        .data.u64 = SIGNAL_EPOLL_EVENT,
    };

    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->signal_fd, &event) == -1)
    {
        return capy_errno(errno);
    }

    event = (struct epoll_event){
        .events = EPOLLIN,
        .data.u64 = 0,
    };

    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->fd, &event) == -1)
    {
        return capy_errno(errno);
    }

    return ok;
}

static capy_err http_server_create_workers_epoll(http_server *server)
{
    struct epoll_event event;

    server->workers_epoll_fd = epoll_create1(0);

    if (server->workers_epoll_fd == -1)
    {
        return capy_errno(errno);
    }

    event = (struct epoll_event){
        .events = EPOLLIN,
        .data.u64 = SIGNAL_EPOLL_EVENT,
    };

    if (epoll_ctl(server->workers_epoll_fd, EPOLL_CTL_ADD, server->signal_fd, &event) == -1)
    {
        return capy_errno(errno);
    }

    return ok;
}

static capy_err http_server_create_connections(http_server *server)
{
    server->connections = make(server->arena, http_conn *, server->options.connections);

    if (server->connections == NULL)
    {
        return capy_errno(ENOMEM);
    }

    for (size_t i = 0; i < server->options.connections; i++)
    {
        capy_arena *arena = capy_arena_init(server->options.line_buffer_size + KiB(4),
                                            server->options.mem_connection_max);

        if (arena == NULL)
        {
            return capy_errno(ENOMEM);
        }

        http_conn *conn = make(arena, http_conn, 1);

        conn->arena = arena;
        conn->router = server->router;
        conn->state = STATE_CLOSED;
        conn->options = &server->options;
        conn->line_buffer = capy_buffer_init(arena, server->options.line_buffer_size);
        conn->line_buffer->arena = NULL;
        conn->marker_init = capy_arena_end(arena);

        pthread_mutex_init(&conn->mut, NULL);

        server->connections[i] = conn;
    }

    return ok;
}

static capy_err http_server_create_workers(http_server *server)
{
    server->workers = make(server->arena, http_worker, server->options.workers);

    if (server->workers == NULL)
    {
        return capy_errno(ENOMEM);
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
            return capy_errno(err);
        }
    }

    return ok;
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

        if (conn->state == STATE_BLOCKED)
        {
            if (difftime(ts.tv_sec, conn->timestamp.tv_sec) > 15)
            {
                capy_loginf("server: connection (%d) closed due to inactivity", conn->socket);
                conn_close(conn);
            }
        }

        pthread_mutex_unlock(&conn->mut);
    }

    size_t active = 0;

    for (size_t i = 0; i < total; i++)
    {
        http_conn *conn = server->connections[i];

        if (conn->state == STATE_CLOSED)
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
            close(conn->socket);
        }

        pthread_mutex_destroy(&conn->mut);
        capy_arena_destroy(conn->arena);
    }

    close(server->signal_fd);
    close(server->fd);
    close(server->epoll_fd);
    close(server->workers_epoll_fd);

    capy_arena_destroy(server->arena);
}

capy_err capy_http_serve(capy_http_server_options options)
{
    capy_err err;

    capy_arena *arena = capy_arena_init(0, MiB(1));

    if (arena == NULL)
    {
        return errfmt(ENOMEM, "Failed to create server arena");
    }

    capy_http_router *router = capy_http_router_init(arena, options.routes_size, options.routes);

    if (router == NULL)
    {
        return errfmt(ENOMEM, "Failed to create router");
    }

    http_server server = {
        .router = router,
        .arena = arena,
        .options = http_server_options_with_defaults(options),
    };

    if ((err = http_server_bind(&server)).code)
    {
        return errwrap(err, "Failed to bind server");
    }

    if ((err = http_server_create_signal_fd(&server)).code)
    {
        return errwrap(err, "Failed to create signal file descriptors");
    }

    if ((err = http_server_create_epoll(&server)).code)
    {
        return errwrap(err, "Failed to create listen epoll");
    }

    if ((err = http_server_create_workers_epoll(&server)).code)
    {
        return errwrap(err, "Failed to create listen epoll");
    }

    if ((err = http_server_create_connections(&server)).code)
    {
        return errwrap(err, "Failed to create connection pool");
    }

    if ((err = http_server_create_workers(&server)).code)
    {
        return errwrap(err, "Failed to create workers pool");
    }

    if (listen(server.fd, 10) == -1)
    {
        return errwrap(capy_errno(errno), "Failed to listen");
    }

    capy_loginf("server: listening at %s:%s (workers = %lu, connections = %lu)",
                server.options.host, server.options.port,
                server.options.workers, server.options.connections);

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    int keepcnt = 4;
    int keepidle = 60;
    int keepintvl = 15;

    unsigned int timeout = cast(unsigned int, (keepidle + (keepcnt * keepintvl)) * 1000);

    struct epoll_event events[10];

    for (int stop = false; !stop;)
    {
        int events_count = epoll_pwait(server.epoll_fd, events, 10, 5000, &server.signals);

        if (events_count == -1)
        {
            err = capy_errno(errno);

            if (err.code == EINTR)
            {
                continue;
            }

            return errwrap(err, "Failed to receive events from epoll");
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
                return errwrap(capy_errno(errno), "Failed to accept connection");
            }

            if (server.active_connections >= server.options.connections)
            {
                close(conn_fd);
                continue;
            }

            if (setsockopt(conn_fd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int)) == -1)
            {
                return errwrap(capy_errno(errno), "Failed to set SO_KEEPALIVE value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt)) == -1)
            {
                return errwrap(capy_errno(errno), "Failed to set TCP_KEEPCNT value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)) == -1)
            {
                return errwrap(capy_errno(errno), "Failed to set TCP_KEEPIDLE value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl)) == -1)
            {
                return errwrap(capy_errno(errno), "Failed to set TCP_KEEPINTVL value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &timeout, sizeof(timeout)) == -1)
            {
                return errwrap(capy_errno(errno), "Failed to set TCP_USER_TIMEOUT value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int)) == -1)
            {
                return errwrap(capy_errno(errno), "Failed to set TCP_NODELAY value");
            }

            http_conn *conn = server.connections[server.active_connections++];

            conn->socket = conn_fd;
            conn->state = STATE_INIT;
            timespec_get(&conn->timestamp, TIME_UTC);

            if ((err = conn_epoll(conn, server.workers_epoll_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET | EPOLLONESHOT)).code)
            {
                return err;
            }
        }
    }

    http_server_destroy(&server);

    capy_loginf("server: shutting down");

    return ok;
}
