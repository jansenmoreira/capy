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

#define SIGNAL_EPOLL_EVENT -1ULL

typedef struct http_worker
{
    size_t id;
    pthread_t thread_id;
    int epoll_fd;
    pthread_mutex_t *mut;
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
    STATE_HANDLER_FAILURE,
    STATE_SERVER_FAILURE,
    STATE_CLOSE_CONNECTION,
    STATE_BLOCKED,
} http_conn_state;

static const char *http_conn_state_cstr(http_conn_state state)
{
    switch (state)
    {
        case STATE_INIT:
            return "STATE_INIT";
        case STATE_READ_SOCKET:
            return "STATE_READ_SOCKET";
        case STATE_PARSE_REQUEST_LINE:
            return "STATE_PARSE_REQUEST_LINE";
        case STATE_PARSE_HEADERS:
            return "STATE_PARSE_HEADERS";
        case STATE_PARSE_CONTENT:
            return "STATE_PARSE_CONTENT";
        case STATE_PARSE_CHUNK_SIZE:
            return "STATE_PARSE_CHUNK_SIZE";
        case STATE_PARSE_CHUNK_DATA:
            return "STATE_PARSE_CHUNK_DATA";
        case STATE_PARSE_TRAILERS:
            return "STATE_PARSE_TRAILERS";
        case STATE_PROCESS_REQUEST:
            return "STATE_PROCESS_REQUEST";
        case STATE_WRITE_RESPONSE:
            return "STATE_WRITE_RESPONSE";
        case STATE_BAD_REQUEST:
            return "STATE_BAD_REQUEST";
        case STATE_HANDLER_FAILURE:
            return "STATE_HANDLER_FAILURE";
        case STATE_SERVER_FAILURE:
            return "STATE_SERVER_FAILURE";
        case STATE_CLOSE_CONNECTION:
            return "STATE_CLOSE_CONNECTION";
        case STATE_BLOCKED:
            return "STATE_BLOCKED";
        case STATE_UNKNOWN:
        default:
            return "STATE_UNKNOWN";
    }
}

typedef struct http_conn
{
    int socket;

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

static int conn_epoll(http_conn *conn, int epoll_fd, int op, uint32_t events)
{
    struct epoll_event client_event = {
        .events = events,
        .data.ptr = conn,
    };

    return epoll_ctl(epoll_fd, op, conn->socket, &client_event);
}

static inline int log_gaierr(int err, const char *file, int line, const char *msg)
{
    fprintf(stderr, "[%s:%d] => %s: %s\n", file, line, msg, gai_strerror(err));
    return err;
}

static inline int conn_msg_get_line(http_conn *conn, capy_string *line)
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

static inline void conn_msg_shl(http_conn *conn, size_t size)
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

static inline void conn_msg_consume_line(http_conn *conn)
{
    conn_msg_shl(conn, conn->line_cursor);
}

static inline http_conn_state conn_write_response(http_conn *conn)
{
    ssize_t bytes_written = send(conn->socket,
                                 conn->response_buffer->data,
                                 conn->response_buffer->size,
                                 0);

    if (bytes_written == 0)
    {
        return STATE_CLOSE_CONNECTION;
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

        return STATE_CLOSE_CONNECTION;
    }

    capy_buffer_shl(conn->response_buffer, cast(size_t, bytes_written));

    if (conn->response_buffer->size == 0)
    {
        if (conn->request.close)
        {
            return STATE_CLOSE_CONNECTION;
        }

        return STATE_INIT;
    }

    return STATE_WRITE_RESPONSE;
}

static inline http_conn_state conn_parse_request_line(http_conn *conn)
{
    capy_string line;

    if (conn_msg_get_line(conn, &line))
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

    conn_msg_consume_line(conn);

    return STATE_PARSE_HEADERS;
}

static inline http_conn_state conn_parse_headers(http_conn *conn)
{
    for (;;)
    {
        capy_string line;

        if (conn_msg_get_line(conn, &line))
        {
            conn->after_read = STATE_PARSE_HEADERS;
            return STATE_READ_SOCKET;
        }

        if (line.size == 0)
        {
            conn_msg_consume_line(conn);
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

        conn_msg_consume_line(conn);
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

        if (conn_msg_get_line(conn, &line))
        {
            conn->after_read = STATE_PARSE_TRAILERS;
            return STATE_READ_SOCKET;
        }

        if (line.size == 0)
        {
            conn_msg_consume_line(conn);
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

        conn_msg_consume_line(conn);
    }

    return STATE_PROCESS_REQUEST;
}

static inline http_conn_state conn_parse_chunk_size(http_conn *conn)
{
    capy_string line;

    if (conn_msg_get_line(conn, &line))
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

    conn_msg_consume_line(conn);

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

        conn_msg_shl(conn, msg_size);

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

    conn_msg_shl(conn, conn->chunk_size + 2);

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
        return STATE_CLOSE_CONNECTION;
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

        return STATE_CLOSE_CONNECTION;
    }

    if (capy_buffer_resize(conn->line_buffer, line_size + (size_t)(bytes_read)))
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

        conn_msg_shl(conn, message_size);

        conn->chunk_size -= message_size;

        conn->after_read = STATE_PARSE_CONTENT;
        return STATE_READ_SOCKET;
    }

    if (capy_buffer_wbytes(conn->content_buffer, conn->chunk_size, conn->line_buffer->data))
    {
        return STATE_SERVER_FAILURE;
    }

    conn_msg_shl(conn, conn->chunk_size);

    return STATE_PROCESS_REQUEST;
}

static inline http_conn_state conn_handler_failure(http_conn *conn)
{
    conn->response.status = CAPY_HTTP_INTERNAL_SERVER_ERROR;

    if (capy_buffer_wcstr(conn->response.content, "Internal Server Error\n"))
    {
        return STATE_SERVER_FAILURE;
    }

    if (capy_http_write_response(conn->response_buffer, &conn->response))
    {
        return STATE_SERVER_FAILURE;
    }

    return STATE_WRITE_RESPONSE;
}

static inline http_conn_state conn_bad_request(http_conn *conn)
{
    conn->response.status = CAPY_HTTP_BAD_REQUEST;
    conn->request.close = true;

    if (capy_strkvmmap_add(conn->response.headers, strl("Connection"), strl("close")))
    {
        return STATE_SERVER_FAILURE;
    }

    if (capy_strkvmmap_add(conn->response.headers, strl("Content-Type"), strl("text/plain; charset=UTF-8")))
    {
        return STATE_SERVER_FAILURE;
    }

    if (capy_buffer_wcstr(conn->response.content, "Malformed HTTP Request\n"))
    {
        return STATE_SERVER_FAILURE;
    }

    if (capy_http_write_response(conn->response_buffer, &conn->response))
    {
        return STATE_SERVER_FAILURE;
    }

    return STATE_WRITE_RESPONSE;
}

static inline http_conn_state conn_process_request(http_conn *conn)
{
    conn->request.content = conn->content_buffer->data;
    conn->request.content_length = conn->content_buffer->size;

    if (capy_http_router_handle(conn->arena, conn->router, &conn->request, &conn->response))
    {
        return STATE_HANDLER_FAILURE;
    }

    if (conn->request.close)
    {
        if (capy_strkvmmap_add(conn->response.headers, strl("Connection"), strl("close")))
        {
            return STATE_SERVER_FAILURE;
        }
    }

    if (capy_http_write_response(conn->response_buffer, &conn->response))
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

    conn->line_cursor = 2;
    conn->after_read = STATE_UNKNOWN;
    conn->after_unblock = STATE_UNKNOWN;

    conn->request = (capy_http_request){
        .headers = capy_strkvmmap_init(conn->arena, 16),
        .trailers = capy_strkvmmap_init(conn->arena, 4),
    };

    conn->response = (capy_http_response){
        .headers = capy_strkvmmap_init(conn->arena, 16),
        .content = capy_buffer_init(conn->arena, 256),
    };

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

    capy_arena_destroy(conn->arena);

    return 0;
}

static int64_t difftimespec_us(struct timespec a, struct timespec b)
{
    return ((a.tv_sec - b.tv_sec) * (int64_t)(1000000000) + a.tv_nsec - b.tv_nsec) / 1000;
}

static inline int conn_limits_exceeded(http_conn *conn)
{
    if ((size_t)conn->mem_headers > conn->options->max_mem_req_headers)
    {
        return true;
    }

    if ((size_t)conn->mem_content > conn->options->max_mem_req_content)
    {
        return true;
    }

    if ((size_t)conn->mem_trailers > conn->options->max_mem_req_trailers)
    {
        return true;
    }

    return false;
}

static inline void conn_trace(http_conn *conn, http_worker *worker)
{
    struct timespec timestamp;
    timespec_get(&timestamp, TIME_UTC);

    int64_t elapsed = difftimespec_us(timestamp, conn->timestamp);

    conn->timestamp = timestamp;

    char time[10];
    char time_info[64];

    strftime(time, 10, "%X", gmtime(&timestamp.tv_sec));
    snprintf(time_info, 64, "%s.%03ld %9ldµs", time, timestamp.tv_nsec / 1000000, elapsed);

    size_t mem_total = capy_arena_size(conn->arena);
    size_t to_read = (conn->line_buffer) ? conn->line_buffer->size : 0;
    size_t to_write = (conn->response_buffer) ? conn->response_buffer->size : 0;

    printf(
        "%-24s | worker | socket | mem_req_headers  | mem_req_content  | mem_req_trailers | mem_response     | mem_total        | to_read  | to_write\n"
        "%-24s | %-6lu | %-6d | %-16ld | %-16ld | %-16ld | %-16ld | %-16lu | %-8lu | %-8lu\n",
        time_info, http_conn_state_cstr(conn->state), worker->id, conn->socket,
        conn->mem_headers, conn->mem_content, conn->mem_trailers, conn->mem_response, mem_total, to_read, to_write);
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
        if (conn->options->trace)
        {
            conn_trace(conn, worker);
        }

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

            case STATE_HANDLER_FAILURE:
                conn->state = conn_handler_failure(conn);
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

            case STATE_CLOSE_CONNECTION:
                return conn_close(conn);

            case STATE_SERVER_FAILURE:
            case STATE_UNKNOWN:
            default:
                capy_assert(false);
        }

        if (conn_limits_exceeded(conn))
        {
            conn->state = STATE_BAD_REQUEST;
        }
    }
}

static void *conn_worker(void *data)
{
    http_worker *worker = data;

    struct epoll_event events[10];

    for (;;)
    {
        int events_count = epoll_pwait(worker->epoll_fd, events, 10, 5000, worker->mask);

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

            if (conn_state_machine(events[i].data.ptr, worker))
            {
                abort();
            }
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

    if (!options.workers_conn_max)
    {
        options.workers_conn_max = 64;
    }

    if (!options.msg_buffer_size)
    {
        options.msg_buffer_size = 8 * 1024;
    }

    if (!options.max_mem_total)
    {
        options.max_mem_total = 2 * 1024 * 1024;
    }

    if (!options.max_mem_req_content)
    {
        options.max_mem_req_content = 1 * 1024 * 1024;
    }

    if (!options.max_mem_req_headers)
    {
        options.max_mem_req_headers = 64 * 1024;
    }

    if (!options.max_mem_req_trailers)
    {
        options.max_mem_req_trailers = 8 * 1024;
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

static http_worker *http_create_workers(int epoll_fd, pthread_mutex_t *mut, sigset_t *mask, size_t workers_count)
{
    int err;

    size_t reserve = (sizeof(http_worker) * workers_count);

    capy_arena *arena = capy_arena_init(reserve, 8 * 1024 * 1024);

    if (arena == NULL)
    {
        capy_log_errno(ENOMEM, "Failed to create arena for worker threads");
        return NULL;
    }

    http_worker *workers = make(arena, http_worker, workers_count);

    for (size_t i = 0; i < workers_count; i++)
    {
        workers[i].id = i;
        workers[i].epoll_fd = epoll_fd;
        workers[i].mut = mut;
        workers[i].mask = mask;

        if ((err = pthread_create(&workers[i].thread_id, NULL, conn_worker, &workers[i])))
        {
            capy_log_errno(err, "Failed to create worker thread");
            return NULL;
        }
    }

    return workers;
}

int capy_http_serve(const char *host, const char *port, capy_http_router *router, capy_http_server_options options)
{
    int err;

    capy_assert(host || port);
    capy_assert(router != NULL);

    options = http_get_default_options(options);

    int server_fd = http_bind_server(host, port);

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

    pthread_mutex_t mut;

    if ((err = pthread_mutex_init(&mut, NULL)))
    {
        return capy_log_errno(err, "Failed to create worker mutex");
    }

    http_worker *workers = http_create_workers(workers_epoll_fd, &mut, &mask, options.workers);

    if (workers == NULL)
    {
        return ENOMEM;
    }

    printf("server: listening at '%s' and '%s' using %lu workers\n", host ? host : "localhost", port, options.workers);

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    int stopped = false;

    int keepcnt = 4;
    int keepidle = 60;
    int keepintvl = 15;

    unsigned int timeout = cast(unsigned int, (keepidle + (keepcnt * keepintvl)) * 1000);

    struct epoll_event events[10];

    while (!stopped)
    {
        int events_count = epoll_pwait(epoll_fd, events, 10, -1, &mask);

        if (events_count == -1)
        {
            err = errno;

            if (err == EINTR)
            {
                continue;
            }

            return capy_log_errno(errno, "Failed to receive events from epoll");
        }

        for (int i = 0; i < events_count; i++)
        {
            if (events[i].data.u64 == SIGNAL_EPOLL_EVENT)
            {
                stopped = true;
                break;
            }

            int conn_fd = accept4(server_fd, address_buffer, &address_buffer_size, SOCK_NONBLOCK);

            if (conn_fd == -1)
            {
                return capy_log_errno(errno, "Failed to accept connection");
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

            capy_arena *arena = capy_arena_init(16 * 1024, options.max_mem_total);

            if (arena == NULL)
            {
                return capy_log_errno(errno, "Failed to create connection arena");
            }

            http_conn *conn = make(arena, http_conn, 1);

            conn->arena = arena;
            conn->socket = conn_fd;
            conn->router = router;
            conn->state = STATE_INIT;
            conn->options = &options;
            conn->line_buffer = capy_buffer_init(arena, options.msg_buffer_size);
            conn->line_buffer->arena = NULL;
            conn->marker_init = capy_arena_end(arena);

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

    return 0;
}
