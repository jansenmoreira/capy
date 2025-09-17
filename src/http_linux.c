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

    capy_strkvmmap *headers;
    capy_buffer *body;

    capy_http_router *router;

    capy_http_server_options *options;

    struct timespec timestamp;
} http_conn;

static inline int log_gaierr(int err, const char *file, int line, const char *msg)
{
    capy_logerr("[%s:%d] => %s: %s\n", file, line, msg, gai_strerror(err));
    return err;
}

static int conn_epoll(http_conn *conn, int epoll_fd, int op, uint32_t events)
{
    struct epoll_event client_event = {
        .events = events,
        .data.ptr = conn,
    };

    return epoll_ctl(epoll_fd, op, conn->socket, &client_event);
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

static inline http_conn_state conn_write_response(http_conn *conn)
{
    ssize_t bytes_written = send(conn->socket,
                                 conn->response_buffer->data,
                                 conn->response_buffer->size,
                                 0);

    if (bytes_written == 0)
    {
        return STATE_CLOSED;
    }
    else if (bytes_written == -1)
    {
        int err = errno;

        if (err == EWOULDBLOCK || err == EAGAIN)
        {
            conn->after_unblock = STATE_WRITE_RESPONSE;

            return STATE_BLOCKED;
        }

        if (err != ECONNRESET)
        {
            capy_log_errno(errno, "Failed to write message");
        }

        return STATE_CLOSED;
    }

    capy_buffer_shl(conn->response_buffer, cast(size_t, bytes_written));

    if (conn->response_buffer->size == 0)
    {
        if (conn->request.close)
        {
            return STATE_CLOSED;
        }

        return STATE_INIT;
    }

    return STATE_WRITE_RESPONSE;
}

static inline http_conn_state conn_parse_request_line(http_conn *conn)
{
    capy_string line;

    if (conn_get_line(conn, &line))
    {
        conn->after_read = STATE_PARSE_REQUEST_LINE;
        return STATE_READ_SOCKET;
    }

    switch (capy_http_parse_reqline(conn->arena, &conn->request, line))
    {
        case 0:
            break;
        case EINVAL:
            return STATE_BAD_REQUEST;
        default:
            return STATE_SERVER_FAILURE;
    }

    conn_consume_line(conn);

    return STATE_PARSE_HEADERS;
}

static inline http_conn_state conn_parse_headers(http_conn *conn)
{
    for (;;)
    {
        capy_string line;

        if (conn_get_line(conn, &line))
        {
            conn->after_read = STATE_PARSE_HEADERS;
            return STATE_READ_SOCKET;
        }

        if (line.size == 0)
        {
            conn_consume_line(conn);
            break;
        }

        switch (capy_http_parse_field(conn->request.headers, line))
        {
            case 0:
                break;
            case EINVAL:
                return STATE_BAD_REQUEST;
            default:
                return STATE_SERVER_FAILURE;
        }

        conn_consume_line(conn);
    }

    switch (capy_http_request_validate(conn->arena, &conn->request))
    {
        case 0:
            break;
        case EINVAL:
            return STATE_BAD_REQUEST;
        default:
            return STATE_SERVER_FAILURE;
    }

    if (conn->request.content_length)
    {
        conn->chunk_size = conn->request.content_length;
        return STATE_PARSE_CONTENT;
    }
    else if (conn->request.chunked)
    {
        return STATE_PARSE_CHUNK_SIZE;
    }

    return STATE_PROCESS_REQUEST;
}

static inline http_conn_state conn_parse_trailers(http_conn *conn)
{
    for (;;)
    {
        capy_string line;

        if (conn_get_line(conn, &line))
        {
            conn->after_read = STATE_PARSE_TRAILERS;
            return STATE_READ_SOCKET;
        }

        if (line.size == 0)
        {
            conn_consume_line(conn);
            break;
        }

        switch (capy_http_parse_field(conn->request.trailers, line))
        {
            case 0:
                break;
            case EINVAL:
                return STATE_BAD_REQUEST;
            default:
                return STATE_SERVER_FAILURE;
        }

        conn_consume_line(conn);
    }

    return STATE_PROCESS_REQUEST;
}

static inline http_conn_state conn_parse_chunk_size(http_conn *conn)
{
    capy_string line;

    if (conn_get_line(conn, &line))
    {
        conn->after_read = STATE_PARSE_CHUNK_SIZE;
        return STATE_READ_SOCKET;
    }

    int64_t value;

    // todo: validate chunck_size extensions
    if (capy_string_hex(line, &value) == 0)
    {
        return STATE_BAD_REQUEST;
    }

    conn->chunk_size = (size_t)(value);

    conn_consume_line(conn);

    if (conn->chunk_size == 0)
    {
        return STATE_PARSE_TRAILERS;
    }

    return STATE_PARSE_CHUNK_DATA;
}

static inline http_conn_state conn_parse_chunk_data(http_conn *conn)
{
    size_t msg_size = conn->line_buffer->size;

    if (conn->chunk_size + 2 > msg_size)
    {
        if (capy_buffer_wbytes(conn->content_buffer, msg_size, conn->line_buffer->data))
        {
            return STATE_SERVER_FAILURE;
        }

        conn_consume_bytes(conn, msg_size);

        conn->chunk_size -= msg_size;

        conn->after_read = STATE_PARSE_CHUNK_DATA;
        return STATE_READ_SOCKET;
    }

    const char *end = conn->line_buffer->data + conn->chunk_size;

    if (end[0] != '\r' || end[1] != '\n')
    {
        return STATE_BAD_REQUEST;
    }

    if (capy_buffer_wbytes(conn->content_buffer, conn->chunk_size, conn->line_buffer->data))
    {
        return STATE_SERVER_FAILURE;
    }

    conn_consume_bytes(conn, conn->chunk_size + 2);

    return STATE_PARSE_CHUNK_SIZE;
}

static inline http_conn_state conn_read_socket(http_conn *conn)
{
    size_t line_limit = conn->line_buffer->capacity;
    size_t line_size = conn->line_buffer->size;
    size_t bytes_wanted = line_limit - line_size;

    if (bytes_wanted == 0)
    {
        return STATE_BAD_REQUEST;
    }

    ssize_t bytes_read = recv(conn->socket, conn->line_buffer->data + line_size, bytes_wanted, 0);

    if (bytes_read == 0)
    {
        return STATE_CLOSED;
    }

    if (bytes_read == -1)
    {
        int err = errno;

        if (err == EWOULDBLOCK || err == EAGAIN)
        {
            conn->after_unblock = STATE_READ_SOCKET;
            return STATE_BLOCKED;
        }

        if (err != ECONNRESET)
        {
            capy_log_errno(errno, "Failed to read message");
        }

        return STATE_CLOSED;
    }

    if (capy_buffer_resize(conn->line_buffer, line_size + cast(size_t, bytes_read)))
    {
        return STATE_SERVER_FAILURE;
    }

    return conn->after_read;
}

static inline http_conn_state conn_parse_content(http_conn *conn)
{
    size_t message_size = conn->line_buffer->size;

    if (conn->chunk_size > message_size)
    {
        if (capy_buffer_wbytes(conn->content_buffer, message_size, conn->line_buffer->data))
        {
            return STATE_SERVER_FAILURE;
        }

        conn_consume_bytes(conn, message_size);

        conn->chunk_size -= message_size;

        conn->after_read = STATE_PARSE_CONTENT;
        return STATE_READ_SOCKET;
    }

    if (capy_buffer_wbytes(conn->content_buffer, conn->chunk_size, conn->line_buffer->data))
    {
        return STATE_SERVER_FAILURE;
    }

    conn_consume_bytes(conn, conn->chunk_size);

    return STATE_PROCESS_REQUEST;
}

static inline http_conn_state conn_bad_request(http_conn *conn)
{
    conn->request.close = true;

    if (http_status_text(CAPY_HTTP_BAD_REQUEST, conn->headers, conn->body))
    {
        return STATE_SERVER_FAILURE;
    }

    if (capy_http_write_response(conn->response_buffer, CAPY_HTTP_BAD_REQUEST, conn->headers, conn->body, true))
    {
        return STATE_SERVER_FAILURE;
    }

    return STATE_WRITE_RESPONSE;
}

static inline http_conn_state conn_process_request(http_conn *conn)
{
    conn->request.content = conn->content_buffer->data;
    conn->request.content_length = conn->content_buffer->size;

    capy_http_status status = capy_http_router_handle(conn->arena, conn->router, &conn->request, conn->headers, conn->body);

    if (capy_http_write_response(conn->response_buffer, status, conn->headers, conn->body, conn->request.close))
    {
        return STATE_SERVER_FAILURE;
    }

    return STATE_WRITE_RESPONSE;
}

static http_conn_state conn_message_init(http_conn *conn)
{
    if (capy_arena_free(conn->arena, conn->marker_init))
    {
        return STATE_SERVER_FAILURE;
    }

    conn->line_buffer->size = 0;
    conn->line_cursor = 2;
    conn->after_read = STATE_UNKNOWN;
    conn->after_unblock = STATE_UNKNOWN;

    conn->request = (capy_http_request){
        .headers = capy_strkvmmap_init(conn->arena, 16),
        .trailers = capy_strkvmmap_init(conn->arena, 4),
        .query = capy_strkvmmap_init(conn->arena, 8),
    };

    conn->headers = capy_strkvmmap_init(conn->arena, 16);
    conn->body = capy_buffer_init(conn->arena, 256);

    conn->content_buffer = capy_buffer_init(conn->arena, 256);
    conn->response_buffer = capy_buffer_init(conn->arena, 512);

    conn->mem_headers = 0;
    conn->mem_content = 0;
    conn->mem_trailers = 0;
    conn->mem_response = 0;

    return STATE_PARSE_REQUEST_LINE;
}

static int conn_blocked(http_conn *conn, http_worker *worker)
{
    timespec_get(&conn->timestamp, TIME_UTC);

    uint32_t event = (conn->after_unblock == STATE_WRITE_RESPONSE) ? EPOLLOUT : EPOLLIN;

    if (conn_epoll(conn, worker->epoll_fd, EPOLL_CTL_MOD, event | EPOLLET | EPOLLONESHOT) == -1)
    {
        return capy_log_errno(errno, "Failed to update client epoll");
    }

    return 0;
}

static int conn_close(http_conn *conn)
{
    if (close(conn->socket) == -1)
    {
        return capy_log_errno(errno, "Failed to close connection");
    }

    conn->state = STATE_CLOSED;
    conn->socket = 0;

    return 0;
}

static int64_t difftimespec_us(struct timespec a, struct timespec b)
{
    return ((a.tv_sec - b.tv_sec) * (int64_t)(1000000000) + a.tv_nsec - b.tv_nsec) / 1000;
}

static void normalize_time(int64_t *us, const char **unit)
{
    if (*us > 1000000)
    {
        *unit = "s";
        *us /= 1000000;
    }
    else if (*us > 1000)
    {
        *unit = "ms";
        *us /= 1000;
    }
    else
    {
        *unit = "μs";
    }
}

static inline void conn_trace(http_conn *conn, http_worker *worker)
{
    struct timespec timestamp;
    timespec_get(&timestamp, TIME_UTC);

    int64_t elapsed = difftimespec_us(timestamp, conn->timestamp);
    const char *elapsed_unit = "us";

    normalize_time(&elapsed, &elapsed_unit);

    conn->timestamp = timestamp;

    size_t mem_total = capy_arena_size(conn->arena);
    size_t to_read = (conn->line_buffer) ? conn->line_buffer->size : 0;
    size_t to_write = (conn->response_buffer) ? conn->response_buffer->size : 0;

    capy_logdbg("worker: %-24s | WK:%-4zu SK:%-4d MH:%-8zu MC:%-8zu MT:%-8zu MR:%-8zu MA:%-8zu RS:%-8zu WS:%-8zu | %3" PRIi64 " %s",
                http_conn_state_cstr[conn->state], worker->id, conn->socket,
                conn->mem_headers, conn->mem_content, conn->mem_trailers, conn->mem_response, mem_total,
                to_read, to_write, elapsed, elapsed_unit);
}

static inline int conn_state_machine(http_conn *conn, http_worker *worker)
{
    capy_assert(conn != NULL);

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
                conn->state = conn_message_init(conn);
                break;

            case STATE_READ_SOCKET:
                conn->state = conn_read_socket(conn);
                break;

            case STATE_PARSE_REQUEST_LINE:
                conn->state = conn_parse_request_line(conn);
                conn->mem_headers += (ssize_t)capy_arena_size(conn->arena) - begin;
                break;

            case STATE_PARSE_HEADERS:
                conn->state = conn_parse_headers(conn);
                conn->mem_headers += (ssize_t)capy_arena_size(conn->arena) - begin;
                break;

            case STATE_PARSE_CONTENT:
                conn->state = conn_parse_content(conn);
                conn->mem_content += (ssize_t)capy_arena_size(conn->arena) - begin;
                break;

            case STATE_PARSE_CHUNK_SIZE:
                conn->state = conn_parse_chunk_size(conn);
                conn->mem_content += (ssize_t)capy_arena_size(conn->arena) - begin;
                break;

            case STATE_PARSE_CHUNK_DATA:
                conn->state = conn_parse_chunk_data(conn);
                conn->mem_content += (ssize_t)capy_arena_size(conn->arena) - begin;
                break;

            case STATE_PARSE_TRAILERS:
                conn->state = conn_parse_trailers(conn);
                conn->mem_trailers += (ssize_t)capy_arena_size(conn->arena) - begin;
                break;

            case STATE_PROCESS_REQUEST:
                conn->state = conn_process_request(conn);
                conn->mem_response += (ssize_t)capy_arena_size(conn->arena) - begin;
                break;

            case STATE_BAD_REQUEST:
                conn->state = conn_bad_request(conn);
                conn->mem_response += (ssize_t)capy_arena_size(conn->arena) - begin;
                break;

            case STATE_WRITE_RESPONSE:
                conn->state = conn_write_response(conn);
                break;

            case STATE_BLOCKED:
                return conn_blocked(conn, worker);

            case STATE_CLOSED:
                return conn_close(conn);

            case STATE_SERVER_FAILURE:
            case STATE_UNKNOWN:
            default:
                capy_assert(false);
        }
    }
}

static void *conn_worker(void *data)
{
    http_worker *worker = data;

    struct epoll_event events[1];

    for (;;)
    {
        int events_count = epoll_pwait(worker->epoll_fd, events, 1, -1, worker->mask);

        if (events_count == -1)
        {
            int err = errno;

            if (err == EINTR)
            {
                continue;
            }

            capy_log_errno(err, "Failed to receive events from epoll");
            return NULL;
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
                if (conn_state_machine(conn, worker))
                {
                    abort();
                }
            }

            pthread_mutex_unlock(&conn->mut);
        }
    }

    return 0;
}

static capy_http_server_options http_get_default_options(capy_http_server_options options)
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

static int http_bind_server(const char *host, const char *port)
{
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    struct addrinfo *addresses;

    int err = getaddrinfo(host, port, &hints, &addresses);

    if (err)
    {
        log_gaierr(err, __FILE__, __LINE__, "Failed to get server address");
        return -1;
    }

    int server_fd = -1;

    for (struct addrinfo *address = addresses; address != NULL; address = address->ai_next)
    {
        int fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

        if (fd == -1)
        {
            continue;
        }

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) == -1)
        {
            capy_log_errno(errno, "Failed to set SO_REUSEADDR value");
            break;
        }

        if (bind(fd, address->ai_addr, address->ai_addrlen) == -1)
        {
            continue;
        }

        server_fd = fd;
        break;
    }

    freeaddrinfo(addresses);

    return server_fd;
}

static http_conn **http_create_connections(capy_arena *arena, capy_http_router *router, capy_http_server_options *options)
{
    int err;

    http_conn **conns = make(arena, http_conn *, options->connections);

    if (conns == NULL)
    {
        capy_log_errno(errno, "Failed to create connection pool");
        return NULL;
    }

    for (size_t i = 0; i < options->connections; i++)
    {
        capy_arena *arena = capy_arena_init(KiB(4) + options->line_buffer_size, options->mem_connection_max);

        if (arena == NULL)
        {
            capy_log_errno(errno, "Failed to create connection arena");
            return NULL;
        }

        http_conn *conn = make(arena, http_conn, 1);

        conn->arena = arena;
        conn->router = router;
        conn->state = STATE_CLOSED;
        conn->options = options;
        conn->line_buffer = capy_buffer_init(arena, options->line_buffer_size);
        conn->line_buffer->arena = NULL;
        conn->marker_init = capy_arena_end(arena);

        if ((err = pthread_mutex_init(&conn->mut, NULL)))
        {
            capy_log_errno(err, "Failed to create worker mutex");
            return NULL;
        }

        conns[i] = conn;
    }

    return conns;
}

static http_worker *http_create_workers(capy_arena *arena, int epoll_fd, sigset_t *mask, size_t workers_count)
{
    int err;

    http_worker *workers = make(arena, http_worker, workers_count);

    if (workers == NULL)
    {
        capy_log_errno(errno, "Failed to create workers pool");
        return NULL;
    }

    for (size_t i = 0; i < workers_count; i++)
    {
        workers[i].id = i;
        workers[i].epoll_fd = epoll_fd;
        workers[i].mask = mask;

        if ((err = pthread_create(&workers[i].thread_id, NULL, conn_worker, &workers[i])))
        {
            capy_log_errno(err, "Failed to create worker thread");
            return NULL;
        }
    }

    return workers;
}

static int http_clean_connections(http_conn **conns, size_t *count)
{
    int err;

    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    size_t total_count = *count;

    for (size_t i = 0; i < total_count; i++)
    {
        http_conn *conn = conns[i];

        if (pthread_mutex_trylock(&conn->mut) == EBUSY)
        {
            continue;
        }

        if (conn->state == STATE_BLOCKED)
        {
            if (difftime(ts.tv_sec, conn->timestamp.tv_sec) > 15)
            {
                capy_loginf("server: connection (%d) closed due to inactivity", conn->socket);

                if ((err = conn_close(conn)))
                {
                    return err;
                }
            }
        }

        pthread_mutex_unlock(&conn->mut);
    }

    size_t active_count = 0;

    for (size_t i = 0; i < total_count; i++)
    {
        http_conn *conn = conns[i];

        if (conn->state == STATE_CLOSED)
        {
            continue;
        }

        conns[i] = conns[active_count];
        conns[active_count] = conn;

        active_count += 1;
    }

    *count = active_count;

    return 0;
}

int capy_http_serve(capy_http_server_options options)
{
    int err;

    options = http_get_default_options(options);

    int server_fd = http_bind_server(options.host, options.port);

    if (server_fd == -1)
    {
        return capy_log_errno(EADDRNOTAVAIL, "Failed to bind server");
    }

    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    int signal_fd = signalfd(-1, &mask, 0);

    if (signal_fd == -1)
    {
        return capy_log_errno(errno, "Failed to get signal file descriptors");
    }

    struct epoll_event event;

    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1)
    {
        return capy_log_errno(errno, "Failed to create epoll");
    }

    int workers_epoll_fd = epoll_create1(0);

    if (workers_epoll_fd == -1)
    {
        return capy_log_errno(errno, "Failed to create workers epoll");
    }

    event = (struct epoll_event){
        .events = EPOLLIN,
        .data.u64 = SIGNAL_EPOLL_EVENT,
    };

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &event) == -1)
    {
        return capy_log_errno(errno, "Failed to add signal to server epoll");
    }

    if (epoll_ctl(workers_epoll_fd, EPOLL_CTL_ADD, signal_fd, &event) == -1)
    {
        return capy_log_errno(errno, "Failed to add signal to server epoll");
    }

    event = (struct epoll_event){
        .events = EPOLLIN,
        .data.u64 = 0,
    };

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        return capy_log_errno(errno, "Failed to add server socket to server epoll");
    }

    if (listen(server_fd, 10) == -1)
    {
        return capy_log_errno(errno, "Failed to listen");
    }

    size_t reserve = (sizeof(http_worker) * options.workers) +
                     (sizeof(http_conn *) * options.connections);

    capy_arena *arena = capy_arena_init(reserve, MiB(8));

    if (arena == NULL)
    {
        capy_log_errno(ENOMEM, "Failed to create arena for worker threads");
        return ENOMEM;
    }

    capy_http_router *router = capy_http_router_init(arena, options.routes_size, options.routes);

    http_conn **conns = http_create_connections(arena, router, &options);
    size_t conns_count = 0;

    http_worker *workers = http_create_workers(arena, workers_epoll_fd, &mask, options.workers);

    capy_loginf("server: listening at %s:%s (workers = %lu, connections = %lu)",
                options.host, options.port, options.workers, options.connections);

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    int keepcnt = 4;
    int keepidle = 60;
    int keepintvl = 15;

    unsigned int timeout = cast(unsigned int, (keepidle + (keepcnt * keepintvl)) * 1000);

    struct epoll_event events[10];

    for (int stop = false; !stop;)
    {
        int events_count = epoll_pwait(epoll_fd, events, 10, 5000, &mask);

        if (events_count == -1)
        {
            err = errno;

            if (err == EINTR)
            {
                continue;
            }

            return capy_log_errno(errno, "Failed to receive events from epoll");
        }

        if ((err = http_clean_connections(conns, &conns_count)))
        {
            return err;
        }

        for (int i = 0; i < events_count; i++)
        {
            if (events[i].data.u64 == SIGNAL_EPOLL_EVENT)
            {
                stop = true;
                break;
            }

            int conn_fd = accept4(server_fd, address_buffer, &address_buffer_size, SOCK_NONBLOCK);

            if (conn_fd == -1)
            {
                return capy_log_errno(errno, "Failed to accept connection");
            }

            if (conns_count >= options.connections)
            {
                if (close(conn_fd) == -1)
                {
                    return capy_log_errno(errno, "Failed to close connection");
                }

                continue;
            }

            if (setsockopt(conn_fd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int)) == -1)
            {
                return capy_log_errno(errno, "Failed to set SO_KEEPALIVE value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt)) == -1)
            {
                return capy_log_errno(errno, "Failed to set TCP_KEEPCNT value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)) == -1)
            {
                return capy_log_errno(errno, "Failed to set TCP_KEEPIDLE value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl)) == -1)
            {
                return capy_log_errno(errno, "Failed to set TCP_KEEPINTVL value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &timeout, sizeof(timeout)) == -1)
            {
                return capy_log_errno(errno, "Failed to set TCP_USER_TIMEOUT value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int)) == -1)
            {
                return capy_log_errno(errno, "Failed to set TCP_NODELAY value");
            }

            http_conn *conn = conns[conns_count++];

            conn->socket = conn_fd;
            conn->state = STATE_INIT;
            timespec_get(&conn->timestamp, TIME_UTC);

            if (conn_epoll(conn, workers_epoll_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET | EPOLLONESHOT) == -1)
            {
                return capy_log_errno(errno, "Failed to add client to epoll");
            }
        }
    }

    for (size_t i = 0; i < options.workers; i++)
    {
        pthread_join(workers[i].thread_id, NULL);
    }

    for (size_t i = 0; i < options.connections; i++)
    {
        pthread_mutex_destroy(&conns[i]->mut);
        capy_arena_destroy(conns[i]->arena);
    }

    capy_arena_destroy(arena);
    close(signal_fd);
    close(server_fd);
    close(epoll_fd);
    close(workers_epoll_fd);

    capy_loginf("server: shutting down");

    return 0;
}
