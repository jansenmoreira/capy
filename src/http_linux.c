#include <arpa/inet.h>
#include <capy/capy.h>
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

#define SIGNAL_EPOLL_EVENT ((void *)(-1))

typedef struct http_worker
{
    size_t id;
    pthread_t thread_id;
    int epoll_fd;
    pthread_mutex_t mut;
    struct http_conn **conns;
    size_t conns_count;
    size_t conns_max;
} http_worker;

typedef enum
{
    STATE_UNKNOWN,
    STATE_INIT,
    STATE_AGAIN,
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
        case STATE_AGAIN:
            return "STATE_AGAIN";
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
    http_worker *worker;
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

    capy_buffer *message_line;
    capy_buffer *content_buf;
    capy_buffer *response_buf;

    size_t line_cursor;
    size_t chunk_size;

    capy_http_request *request;
    capy_http_router *router;

    capy_http_server_options *options;

    struct timespec time;
} http_conn;

static int conn_epoll(http_conn *conn, int op, uint32_t events)
{
    struct epoll_event client_event = {
        .events = events,
        .data.ptr = conn,
    };

    return epoll_ctl(conn->worker->epoll_fd, op, conn->socket, &client_event);
}

static inline int log_gaierr(int err, const char *file, int line, const char *msg)
{
    fprintf(stderr, "[%s:%d] => %s: %s\n", file, line, msg, gai_strerror(err));
    return err;
}

static inline int conn_msg_get_line(http_conn *conn, capy_string *line)
{
    while (conn->line_cursor <= conn->message_line->size)
    {
        if (conn->line_cursor >= 2 &&
            conn->message_line->data[conn->line_cursor - 2] == '\r' &&
            conn->message_line->data[conn->line_cursor - 1] == '\n')
        {
            break;
        }

        conn->line_cursor += 1;
    }

    if (conn->line_cursor > conn->message_line->size)
    {
        return -1;
    }

    *line = capy_string_bytes(conn->line_cursor - 2, conn->message_line->data);

    return 0;
}

static inline void conn_msg_shl(http_conn *conn, size_t size)
{
    capy_buffer_shl(conn->message_line, size);

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
                                 conn->response_buf->data,
                                 conn->response_buf->size,
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

    capy_buffer_shl(conn->response_buf, (size_t)(bytes_written));

    if (conn->response_buf->size == 0)
    {
        return STATE_AGAIN;
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

    if (conn->request == NULL)
    {
        conn->request = capy_arena_make(conn->arena, capy_http_request, 1);
    }

    int err = capy_http_parse_reqline(conn->arena, conn->request, line);

    if (err)
    {
        switch (err)
        {
            case ENOMEM:
                return STATE_SERVER_FAILURE;
            case EINVAL:
                return STATE_BAD_REQUEST;
        }
    }

    conn_msg_consume_line(conn);

    return STATE_PARSE_HEADERS;
}

static inline http_conn_state conn_parse_headers(http_conn *conn)
{
    int err;

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

        if (conn->request->headers == NULL)
        {
            conn->request->headers = capy_strkvmmap_init(conn->arena, 16);
        }

        err = capy_http_parse_field(conn->request->headers, line);

        if (err)
        {
            switch (err)
            {
                case EINVAL:
                    return STATE_BAD_REQUEST;
                default:
                    return STATE_SERVER_FAILURE;
            }
        }

        conn_msg_consume_line(conn);
    }

    err = capy_http_request_validate(conn->arena, conn->request);

    if (err)
    {
        switch (err)
        {
            case EINVAL:
                return STATE_BAD_REQUEST;
            default:
                return STATE_SERVER_FAILURE;
        }
    }

    if (conn->request->content_length)
    {
        conn->chunk_size = conn->request->content_length;
        return STATE_PARSE_CONTENT;
    }
    else if (conn->request->chunked)
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

        if (conn->request->trailers == NULL)
        {
            conn->request->trailers = capy_strkvmmap_init(conn->arena, 8);
        }

        int err = capy_http_parse_field(conn->request->trailers, line);

        if (err)
        {
            switch (err)
            {
                case EINVAL:
                    return STATE_BAD_REQUEST;
                default:
                    return STATE_SERVER_FAILURE;
            }
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
    if (conn->content_buf == NULL)
    {
        conn->content_buf = capy_buffer_init(conn->arena, 1024);
    }

    size_t msg_size = conn->message_line->size;

    if (conn->chunk_size + 2 > msg_size)
    {
        if (capy_buffer_wbytes(conn->content_buf, msg_size, conn->message_line->data))
        {
            return STATE_SERVER_FAILURE;
        }

        conn_msg_shl(conn, msg_size);

        conn->chunk_size -= msg_size;

        conn->after_read = STATE_PARSE_CHUNK_DATA;
        return STATE_READ_SOCKET;
    }

    const char *end = conn->message_line->data + conn->chunk_size;

    if (end[0] != '\r' || end[1] != '\n')
    {
        return STATE_BAD_REQUEST;
    }

    if (capy_buffer_wbytes(conn->content_buf, conn->chunk_size, conn->message_line->data))
    {
        return STATE_SERVER_FAILURE;
    }

    conn_msg_shl(conn, conn->chunk_size + 2);

    return STATE_PARSE_CHUNK_SIZE;
}

static inline http_conn_state conn_read_socket(http_conn *conn)
{
    size_t message_limit = conn->message_line->capacity;
    size_t message_size = conn->message_line->size;
    size_t bytes_wanted = message_limit - message_size;

    if (bytes_wanted == 0)
    {
        return STATE_BAD_REQUEST;
    }

    ssize_t bytes_read = recv(conn->socket,
                              conn->message_line->data + message_size,
                              bytes_wanted,
                              0);

    if (bytes_read == 0)
    {
        return STATE_CLOSE_CONNECTION;
    }
    else if (bytes_read == -1)
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

    if (capy_buffer_resize(conn->message_line, message_size + (size_t)(bytes_read)))
    {
        return STATE_SERVER_FAILURE;
    }

    return conn->after_read;
}

static inline http_conn_state conn_parse_content(http_conn *conn)
{
    if (conn->content_buf == NULL)
    {
        conn->content_buf = capy_buffer_init(conn->arena, conn->chunk_size);
    }

    size_t message_size = conn->message_line->size;

    if (conn->chunk_size > message_size)
    {
        if (capy_buffer_wbytes(conn->content_buf, message_size, conn->message_line->data))
        {
            return STATE_SERVER_FAILURE;
        }

        conn_msg_shl(conn, message_size);

        conn->chunk_size -= message_size;

        conn->after_read = STATE_PARSE_CONTENT;
        return STATE_READ_SOCKET;
    }

    if (capy_buffer_wbytes(conn->content_buf, conn->chunk_size, conn->message_line->data))
    {
        return STATE_SERVER_FAILURE;
    }

    conn_msg_shl(conn, conn->chunk_size);

    return STATE_PROCESS_REQUEST;
}

static inline http_conn_state conn_process_request(http_conn *conn)
{
    if (conn->content_buf != NULL)
    {
        conn->request->content = conn->content_buf->data;
        conn->request->content_length = conn->content_buf->size;
    }

    capy_http_response response = {
        .headers = capy_strkvmmap_init(conn->arena, 8),
        .content = capy_buffer_init(conn->arena, 1024),
    };

    if (capy_http_router_handle(conn->arena, conn->router, conn->request, &response))
    {
        return STATE_HANDLER_FAILURE;
    }

    response.status = CAPY_HTTP_OK;

    conn->response_buf = capy_buffer_init(conn->arena, 2 * 1024);

    if (capy_http_write_headers(conn->response_buf, &response))
    {
        return STATE_SERVER_FAILURE;
    }

    if (capy_buffer_wbytes(conn->response_buf, response.content->size, response.content->data))
    {
        return STATE_SERVER_FAILURE;
    }

    if (conn_epoll(conn, EPOLL_CTL_MOD, EPOLLOUT) == -1)
    {
        capy_log_errno(errno, "Failed to update client epoll");
        abort();
    }

    return STATE_WRITE_RESPONSE;
}

static http_conn_state conn_message_again(http_conn *conn)
{
    if (conn_epoll(conn, EPOLL_CTL_MOD, EPOLLIN) == -1)
    {
        capy_log_errno(errno, "Failed to update client epoll");
        abort();
    }

    return STATE_INIT;
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

    conn->request = NULL;

    conn->content_buf = NULL;
    conn->response_buf = NULL;

    conn->mem_headers = 0;
    conn->mem_content = 0;
    conn->mem_trailers = 0;
    conn->mem_response = 0;

    return STATE_PARSE_REQUEST_LINE;
}

static int conn_close(http_conn *conn)
{
    if (close(conn->socket) == -1)
    {
        return capy_log_errno(errno, "Failed to close connection");
    }

    conn->worker->conns[conn->conn_id] = NULL;

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

static inline void conn_trace(http_conn *conn)
{
    struct timespec end, begin = conn->time;
    timespec_get(&end, TIME_UTC);
    conn->time = end;

    int64_t elapsed = difftimespec_us(end, begin);

    char time[10];
    char time_info[64];

    strftime(time, 10, "%X", gmtime(&end.tv_sec));
    snprintf(time_info, 64, "%s.%03ld %9ldµs", time, end.tv_nsec / 1000000, elapsed);

    size_t mem_total = capy_arena_size(conn->arena);
    size_t to_read = (conn->message_line) ? conn->message_line->size : 0;
    size_t to_write = (conn->response_buf) ? conn->response_buf->size : 0;

    printf(
        "%-24s | worker | socket | mem_req_headers  | mem_req_content  | mem_req_trailers | mem_response     | mem_total        | to_read  | to_write\n"
        "%-24s | %-6lu | %-6d | %-16ld | %-16ld | %-16ld | %-16ld | %-16lu | %-8lu | %-8lu\n",
        time_info, http_conn_state_cstr(conn->state), conn->worker->id, conn->socket,
        conn->mem_headers, conn->mem_content, conn->mem_trailers, conn->mem_response, mem_total, to_read, to_write);
}

static inline int conn_state_machine(http_conn *conn)
{
    capy_assert(conn != NULL);

    timespec_get(&conn->time, TIME_UTC);

    if (conn->state == STATE_BLOCKED)
    {
        conn->state = conn->after_unblock;
    }

    for (;;)
    {
        if (conn->options->trace)
        {
            conn_trace(conn);
        }

        if (conn_limits_exceeded(conn))
        {
            conn->state = STATE_SERVER_FAILURE;
        }

        ssize_t begin = (ssize_t)capy_arena_size(conn->arena);

        switch (conn->state)
        {
            case STATE_INIT:
                conn->state = conn_message_init(conn);
                break;

            case STATE_AGAIN:
                conn->state = conn_message_again(conn);
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

            case STATE_WRITE_RESPONSE:
                conn->state = conn_write_response(conn);
                break;

            case STATE_BLOCKED:
                return 0;

            case STATE_BAD_REQUEST:
            case STATE_CLOSE_CONNECTION:
                return conn_close(conn);

            case STATE_HANDLER_FAILURE:
            case STATE_SERVER_FAILURE:
            case STATE_UNKNOWN:
            default:
                capy_assert(false);
        }
    }
}

static void conn_worker_cleanup(http_worker *worker)
{
    pthread_mutex_lock(&worker->mut);

    printf("TRACE -> thread %lu cleanup (active connections: %lu)\n", worker->id, worker->conns_count);

    struct timespec ts;

    timespec_get(&ts, TIME_UTC);

    size_t conn_count = 0;

    for (size_t i = 0; i < worker->conns_count; i++)
    {
        http_conn *conn = worker->conns[i];

        if (conn == NULL)
        {
            printf("TRACE -> cleaning NULL conn %lu\n", i);
            continue;
        }

        if (conn->state == STATE_BLOCKED)
        {
            double diff = difftime(ts.tv_sec, conn->time.tv_sec);

            if (diff > 15)
            {
                printf("TRACE -> cleaning TIMEOUT conn %lu\n", i);
                conn_close(conn);
                continue;
            }
        }

        if (conn_count != i)
        {
            printf("TRACE -> moving conn from %lu to %lu\n", i, conn_count);
        }

        worker->conns[conn_count] = conn;
        conn->conn_id = conn_count;

        conn_count += 1;
    }

    worker->conns_count = conn_count;

    pthread_mutex_unlock(&worker->mut);
}

static void *conn_worker(void *data)
{
    http_worker *worker = data;

    struct epoll_event events[10];

    for (;;)
    {
        int nevents = epoll_wait(worker->epoll_fd, events, 10, 5 * 1000);

        if (nevents == -1)
        {
            int err = errno;

            if (err == EINTR)
            {
                continue;
            }

            capy_log_errno(err, "Failed to receive events from epoll");
            return NULL;
        }

        for (int i = 0; i < nevents; i++)
        {
            if (events[i].data.ptr == SIGNAL_EPOLL_EVENT)
            {
                return 0;
            }

            if (conn_state_machine(events[i].data.ptr))
            {
                abort();
            }
        }

        conn_worker_cleanup(worker);
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

int capy_http_serve(const char *host, const char *port, capy_http_router *router, capy_http_server_options options)
{
    int err;

    capy_assert(host || port);
    capy_assert(router != NULL);

    options = http_get_default_options(options);

    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
    {
        return capy_log_errno(errno, "Failed to block signals");
    }

    int signal_fd = signalfd(-1, &mask, 0);

    if (signal_fd == -1)
    {
        return capy_log_errno(errno, "Failed to get signal file descriptors");
    }

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    struct addrinfo *server_addresses;

    err = getaddrinfo(host, port, &hints, &server_addresses);

    if (err)
    {
        return log_gaierr(err, __FILE__, __LINE__, "Failed to get server address");
    }

    struct addrinfo *address;
    int server_fd = -1;

    for (address = server_addresses; address != NULL; address = address->ai_next)
    {
        server_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

        if (server_fd == -1)
        {
            err = errno;
            continue;
        }

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
        {
            return errno;
        }

        if (bind(server_fd, address->ai_addr, address->ai_addrlen) == -1)
        {
            err = errno;
            continue;
        }

        break;
    }

    freeaddrinfo(server_addresses);

    if (address == NULL)
    {
        return capy_log_errno(errno, "Failed to bind socket");
    }

    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1)
    {
        return capy_log_errno(errno, "Failed to create epoll");
    }

    struct epoll_event event = {
        .events = EPOLLIN,
        .data.ptr = SIGNAL_EPOLL_EVENT,
    };

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &event) == -1)
    {
        return capy_log_errno(errno, "Failed to add signal to server epoll");
    }

    event.events = EPOLLIN;
    event.data.ptr = NULL;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        return capy_log_errno(errno, "Failed to add server socket to server epoll");
    }

    if (listen(server_fd, 10) == -1)
    {
        return capy_log_errno(errno, "Failed to listen");
    }

    printf("server: listening at '%s' and '%s' using %lu workers\n", host ? host : "localhost", port, options.workers);

    capy_arena *server_arena = capy_arena_init(8 * 1024 * 1024);

    http_worker *workers = capy_arena_make(server_arena, http_worker, options.workers);

    if (workers == NULL)
    {
        return capy_log_errno(ENOMEM, "Failed to allocate memory for worker threads")
    }

    for (size_t i = 0; i < options.workers; i++)
    {
        http_conn **conns = capy_arena_make(server_arena, http_conn *, options.workers_conn_max);

        if (workers == NULL)
        {
            return capy_log_errno(ENOMEM, "Failed to allocate memory for worker threads")
        }

        workers[i].id = i;
        workers[i].epoll_fd = epoll_create1(0);
        workers[i].conns = conns;
        workers[i].conns_max = options.workers_conn_max;
        workers[i].conns_count = 0;

        pthread_mutex_init(&workers[i].mut, NULL);

        if (workers[i].epoll_fd == -1)
        {
            return capy_log_errno(errno, "Failed to create epoll");
        }

        event.events = EPOLLIN;
        event.data.ptr = SIGNAL_EPOLL_EVENT;

        if (epoll_ctl(workers[i].epoll_fd, EPOLL_CTL_ADD, signal_fd, &event) == -1)
        {
            return capy_log_errno(errno, "Failed to add signal to thread epoll");
        }

        if (pthread_create(&workers[i].thread_id, NULL, conn_worker, &workers[i]) == -1)
        {
            return capy_log_errno(errno, "Failed to create worker thread");
        }

        memset(workers[i].conns, 0, options.workers_conn_max * sizeof(http_conn *));
    }

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    struct epoll_event events[5];

    size_t target = 0;

    int stopped = false;

    while (!stopped)
    {
        int count = epoll_wait(epoll_fd, events, 5, -1);

        if (count == -1)
        {
            err = errno;

            if (err == EINTR)
            {
                continue;
            }

            return capy_log_errno(errno, "Failed to receive events from epoll");
        }

        for (int i = 0; i < count; i++)
        {
            if (events[i].data.ptr == SIGNAL_EPOLL_EVENT)
            {
                stopped = true;
                break;
            }

            int conn_fd = accept4(server_fd, address_buffer, &address_buffer_size, SOCK_NONBLOCK);

            if (conn_fd == -1)
            {
                return capy_log_errno(errno, "Failed to accept connection");
            }

            int keepcnt = 4;
            int keepidle = 60;
            int keepintvl = 15;
            unsigned int timeout = cast(unsigned int, (keepidle + (keepcnt * keepintvl)) * 1000);

            if (setsockopt(conn_fd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int)) == -1)
            {
                return capy_log_errno(errno, "Failed to set SO_KEEPALIVE value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt)) == -1)
            {
                return capy_log_errno(errno, "Failed to get TCP_KEEPCNT value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)) == -1)
            {
                return capy_log_errno(errno, "Failed to get TCP_KEEPIDLE value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl)) == -1)
            {
                return capy_log_errno(errno, "Failed to get TCP_KEEPINTVL value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &timeout, sizeof(timeout)) == -1)
            {
                return capy_log_errno(errno, "Failed to set TCP_USER_TIMEOUT value");
            }

            if (setsockopt(conn_fd, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int)) == -1)
            {
                return capy_log_errno(errno, "Failed to set TCP_NODELAY value");
            }

            capy_arena *arena = capy_arena_init(options.max_mem_total);

            if (arena == NULL)
            {
                return capy_log_errno(errno, "Failed to create connection arena");
            }

            http_conn *conn = capy_arena_make(arena, http_conn, 1);

            conn->arena = arena;
            conn->socket = conn_fd;
            conn->router = router;
            conn->state = STATE_INIT;
            conn->options = &options;
            conn->worker = NULL;
            conn->message_line = capy_buffer_init(arena, options.msg_buffer_size);
            conn->message_line->arena = NULL;
            conn->marker_init = capy_arena_end(arena);

            for (size_t i = 0; conn->worker == NULL && i < options.workers; i++)
            {
                http_worker *worker = &workers[target];
                pthread_mutex_lock(&worker->mut);

                if (worker->conns_count < worker->conns_max)
                {
                    conn->worker = worker;
                    conn->conn_id = worker->conns_count;
                    worker->conns[conn->conn_id] = conn;
                    worker->conns_count += 1;

                    if (conn_epoll(conn, EPOLL_CTL_ADD, EPOLLIN) == -1)
                    {
                        return capy_log_errno(errno, "Failed to add client to epoll");
                    }
                }

                target = (target + 1) % options.workers;
                pthread_mutex_unlock(&worker->mut);
            }

            if (conn->worker == NULL)
            {
                close(conn_fd);
            }
        }
    }

    for (size_t i = 0; i < options.workers; i++)
    {
        pthread_join(workers[i].thread_id, NULL);
    }

    return err;
}
