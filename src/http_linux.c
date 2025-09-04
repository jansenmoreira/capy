#include <arpa/inet.h>
#include <capy/capy.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct http_worker
{
    size_t id;
    pthread_t thread_id;
    int epoll_fd;
    int trace;
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
    STATE_WRITE_HEADERS,
    STATE_WRITE_CONTENT,
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
        case STATE_WRITE_HEADERS:
            return "STATE_WRITE_HEADERS";
        case STATE_WRITE_CONTENT:
            return "STATE_WRITE_CONTENT";
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

    capy_arena *arena;
    void *arena_marker;

    http_conn_state state;
    http_conn_state after_read;
    http_conn_state after_unblock;

    capy_strbuf *msg_buffer;
    capy_strbuf *req_body;
    capy_strbuf *resp_head;
    capy_strbuf *resp_body;

    size_t line_cursor;
    size_t chunk_size;

    capy_http_request *request;
    capy_http_handler *handler;

    size_t mem_begin;
    size_t mem_headers;
    size_t mem_content;
    size_t mem_trailers;
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
    while (conn->line_cursor <= conn->msg_buffer->size)
    {
        if (conn->line_cursor >= 2 &&
            conn->msg_buffer->data[conn->line_cursor - 2] == '\r' &&
            conn->msg_buffer->data[conn->line_cursor - 1] == '\n')
        {
            break;
        }

        conn->line_cursor += 1;
    }

    if (conn->line_cursor > conn->msg_buffer->size)
    {
        return -1;
    }

    *line = capy_string_bytes(conn->line_cursor - 2, conn->msg_buffer->data);

    return 0;
}

static inline void conn_msg_shl(http_conn *conn, size_t size)
{
    capy_strbuf_shl(conn->msg_buffer, size);

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

static inline http_conn_state conn_write_headers(http_conn *conn)
{
    ssize_t bytes_written = send(conn->socket,
                                 conn->resp_head->data,
                                 conn->resp_head->size,
                                 MSG_MORE | MSG_DONTWAIT);

    if (bytes_written == 0)
    {
        return STATE_CLOSE_CONNECTION;
    }
    else if (bytes_written == -1)
    {
        int err = errno;

        if (err == EWOULDBLOCK || err == EAGAIN)
        {
            conn->after_unblock = STATE_WRITE_HEADERS;
            return STATE_BLOCKED;
        }

        if (err != ECONNRESET)
        {
            capy_log_errno(errno, "Failed to write message");
        }

        return STATE_CLOSE_CONNECTION;
    }

    capy_strbuf_shl(conn->resp_head, (size_t)(bytes_written));

    if (conn->resp_head->size == 0)
    {
        return STATE_WRITE_CONTENT;
    }

    return STATE_WRITE_HEADERS;
}

static inline http_conn_state conn_write_content(http_conn *conn)
{
    ssize_t bytes_written = send(conn->socket,
                                 conn->resp_body->data,
                                 conn->resp_body->size,
                                 MSG_DONTWAIT);

    if (bytes_written == 0)
    {
        return STATE_CLOSE_CONNECTION;
    }
    else if (bytes_written == -1)
    {
        int err = errno;

        if (err == EWOULDBLOCK || err == EAGAIN)
        {
            conn->after_unblock = STATE_WRITE_CONTENT;
            return STATE_BLOCKED;
        }

        if (err != ECONNRESET)
        {
            capy_log_errno(errno, "Failed to write message");
        }

        return STATE_CLOSE_CONNECTION;
    }

    capy_strbuf_shl(conn->resp_body, (size_t)(bytes_written));

    if (conn->resp_body->size == 0)
    {
        if (conn_epoll(conn, EPOLL_CTL_MOD, EPOLLIN) == -1)
        {
            capy_log_errno(errno, "Failed to update client epoll");
            abort();
        }

        return STATE_INIT;
    }

    return STATE_WRITE_CONTENT;
}

static inline http_conn_state conn_parse_request_line(http_conn *conn)
{
    if (conn->request == NULL)
    {
        conn->request = capy_arena_make(capy_http_request, conn->arena, 1);
    }

    capy_string line;

    if (conn_msg_get_line(conn, &line))
    {
        conn->after_read = STATE_PARSE_REQUEST_LINE;
        return STATE_READ_SOCKET;
    }

    if (capy_http_parse_request_line(conn->arena, conn->request, line))
    {
        return STATE_CLOSE_CONNECTION;
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

        if (conn->request->headers == NULL)
        {
            conn->request->headers = capy_http_fields_init(conn->arena, 32);
        }

        if (capy_http_parse_field(conn->arena, conn->request->headers, line))
        {
            return STATE_CLOSE_CONNECTION;
        }

        conn_msg_consume_line(conn);
    }

    if (capy_http_request_validate(conn->arena, conn->request))
    {
        return STATE_CLOSE_CONNECTION;
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
            conn->request->trailers = capy_http_fields_init(conn->arena, 32);
        }

        if (capy_http_parse_field(conn->arena, conn->request->trailers, line))
        {
            return STATE_CLOSE_CONNECTION;
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
        return STATE_CLOSE_CONNECTION;
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
    if (conn->req_body == NULL)
    {
        conn->req_body = capy_strbuf_init(conn->arena, 1024);
    }

    size_t msg_size = conn->msg_buffer->size;

    if (conn->chunk_size + 2 > msg_size)
    {
        capy_strbuf_write_bytes(conn->req_body, msg_size, conn->msg_buffer->data);
        conn_msg_shl(conn, msg_size);

        conn->chunk_size -= msg_size;

        conn->after_read = STATE_PARSE_CHUNK_DATA;
        return STATE_READ_SOCKET;
    }

    const char *end = conn->msg_buffer->data + conn->chunk_size;

    if (end[0] != '\r' || end[1] != '\n')
    {
        return STATE_CLOSE_CONNECTION;
    }

    capy_strbuf_write_bytes(conn->req_body, conn->chunk_size, conn->msg_buffer->data);
    conn_msg_shl(conn, conn->chunk_size + 2);

    return STATE_PARSE_CHUNK_SIZE;
}

static inline http_conn_state conn_read_socket(http_conn *conn)
{
    size_t message_limit = conn->msg_buffer->capacity;
    size_t message_size = conn->msg_buffer->size;
    size_t bytes_wanted = message_limit - message_size;

    if (bytes_wanted == 0)
    {
        return STATE_CLOSE_CONNECTION;
    }

    ssize_t bytes_read = recv(conn->socket,
                              conn->msg_buffer->data + message_size,
                              bytes_wanted, MSG_DONTWAIT);

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

    capy_strbuf_resize(conn->msg_buffer, message_size + (size_t)(bytes_read));

    return conn->after_read;
}

static inline http_conn_state conn_parse_content(http_conn *conn)
{
    if (conn->req_body == NULL)
    {
        conn->req_body = capy_strbuf_init(conn->arena, 1024);
    }

    size_t message_size = conn->msg_buffer->size;

    if (conn->chunk_size > message_size)
    {
        capy_strbuf_write_bytes(conn->req_body, message_size, conn->msg_buffer->data);
        conn_msg_shl(conn, message_size);

        conn->chunk_size -= message_size;

        conn->after_read = STATE_PARSE_CONTENT;
        return STATE_READ_SOCKET;
    }

    capy_strbuf_write_bytes(conn->req_body, conn->chunk_size, conn->msg_buffer->data);
    conn_msg_shl(conn, conn->chunk_size);

    return STATE_PROCESS_REQUEST;
}

static inline http_conn_state conn_process_request(http_conn *conn)
{
    if (conn->req_body != NULL)
    {
        conn->request->content = conn->req_body->data;
        conn->request->content_length = conn->req_body->size;
    }

    capy_http_response response = {
        .content = capy_strbuf_init(conn->arena, 1024),
        .headers = capy_http_fields_init(conn->arena, 8),
    };

    if (conn->handler(conn->arena, conn->request, &response))
    {
        return STATE_CLOSE_CONNECTION;
    }

    conn->resp_head = capy_strbuf_init(conn->arena, 1024);

    capy_http_write_headers(conn->resp_head, &response);

    conn->resp_body = response.content;

    if (conn_epoll(conn, EPOLL_CTL_MOD, EPOLLOUT) == -1)
    {
        capy_log_errno(errno, "Failed to update client epoll");
        abort();
    }

    return STATE_WRITE_HEADERS;
}

static http_conn_state conn_message_init(http_conn *conn)
{
    capy_arena_shrink(conn->arena, conn->arena_marker);

    conn->line_cursor = 0;
    conn->after_read = STATE_UNKNOWN;
    conn->after_unblock = STATE_UNKNOWN;
    conn->request = NULL;

    conn->req_body = NULL;
    conn->resp_head = NULL;
    conn->resp_body = NULL;

    return STATE_PARSE_REQUEST_LINE;
}

static int conn_close(http_conn *conn)
{
    int socket = conn->socket;

    capy_arena_free(conn->arena);

    if (close(socket) == -1)
    {
        return capy_log_errno(errno, "Failed to close connection");
    }

    return 0;
}

static inline void conn_trace(http_conn *conn)
{
    char buffer[128] = {0};

    time_t t = time(NULL);
    strftime(buffer, 127, "%X", gmtime(&t));

    size_t arena_size = capy_arena_size(conn->arena);

    float mem_total = (float)(arena_size) / 1024.0f;
    float mem_headers = (float)(conn->mem_headers - conn->mem_begin) / 1024.0f;
    float mem_content = (float)(conn->mem_content - conn->mem_headers) / 1024.0f;
    float mem_trailers = (float)(conn->mem_trailers - conn->mem_content) / 1024.0f;
    float mem_response = (float)(arena_size - conn->mem_trailers) / 1024.0f;
    size_t to_read = (conn->msg_buffer) ? conn->msg_buffer->size : 0;
    size_t to_write_headers = (conn->resp_head) ? conn->resp_head->size : 0;
    size_t to_write_content = (conn->resp_body) ? conn->resp_body->size : 0;

    printf(
        "%-24s | mem_total = %.2f KiB, mem_headers = %.2f KiB, mem_content = %.2f KiB, mem_trailers = %.2f KiB\n"
        "%-24s | mem_response = %.2f KiB, worker = %lu, socket = %d, to_read = %lu, to_write = %lu\n",
        buffer, mem_total, mem_headers, mem_content, mem_trailers,
        http_conn_state_cstr(conn->state), mem_response,
        conn->worker->id, conn->socket,
        to_read, to_write_content + to_write_headers);
}

static inline int conn_state_machine(http_conn *conn)
{
    capy_assert(conn != NULL);

    for (;;)
    {
        if (conn->worker->trace)
        {
            conn_trace(conn);
        }

        switch (conn->state)
        {
            case STATE_INIT:
                conn->state = conn_message_init(conn);
                conn->mem_begin = capy_arena_size(conn->arena);
                conn->mem_headers = conn->mem_begin;
                conn->mem_content = conn->mem_begin;
                conn->mem_trailers = conn->mem_begin;
                break;

            case STATE_READ_SOCKET:
                conn->state = conn_read_socket(conn);
                break;

            case STATE_PARSE_REQUEST_LINE:
                conn->state = conn_parse_request_line(conn);
                conn->mem_headers = capy_arena_size(conn->arena);
                conn->mem_content = conn->mem_headers;
                conn->mem_trailers = conn->mem_content;
                break;

            case STATE_PARSE_HEADERS:
                conn->state = conn_parse_headers(conn);
                conn->mem_headers = capy_arena_size(conn->arena);
                conn->mem_content = conn->mem_headers;
                conn->mem_trailers = conn->mem_content;
                break;

            case STATE_PARSE_CONTENT:
                conn->state = conn_parse_content(conn);
                conn->mem_content = capy_arena_size(conn->arena);
                conn->mem_trailers = conn->mem_content;
                break;

            case STATE_PARSE_CHUNK_SIZE:
                conn->state = conn_parse_chunk_size(conn);
                break;

            case STATE_PARSE_CHUNK_DATA:
                conn->state = conn_parse_chunk_data(conn);
                break;

            case STATE_PARSE_TRAILERS:
                conn->state = conn_parse_trailers(conn);
                break;

            case STATE_PROCESS_REQUEST:
                conn->state = conn_process_request(conn);
                break;

            case STATE_WRITE_HEADERS:
                conn->state = conn_write_headers(conn);
                break;

            case STATE_WRITE_CONTENT:
                conn->state = conn_write_content(conn);
                break;

            case STATE_CLOSE_CONNECTION:
                return conn_close(conn);

            case STATE_BLOCKED:
                conn->state = conn->after_unblock;
                return 0;

            case STATE_UNKNOWN:
            default:
                capy_assert(false);
        }
    }
}

static void *conn_worker(void *data)
{
    http_worker *worker = data;

    struct epoll_event events[10];

    for (;;)
    {
        int fdcount = epoll_wait(worker->epoll_fd, events, 10, -1);

        if (fdcount == -1)
        {
            int err = errno;

            if (err == EINTR)
            {
                continue;
            }

            capy_log_errno(errno, "Failed to receive events from epoll");

            return NULL;
        }

        for (int i = 0; i < fdcount; i++)
        {
            if (conn_state_machine(events[i].data.ptr))
            {
                abort();
            }
        }
    }
}

static http_server_options options_default = {
    .trace = false,
    .workers = 0,
};

int capy_http_serve(const char *host, const char *port, capy_http_handler handler, http_server_options *options)
{
    capy_assert(host || port);
    capy_assert(handler != NULL);

    if (options == NULL)
    {
        options = &options_default;
    }

    if (options->workers == 0)
    {
        options->workers = (size_t)(sysconf(_SC_NPROCESSORS_CONF)) * 2;
    }

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    struct addrinfo *server_addresses;

    int err = getaddrinfo(host, port, &hints, &server_addresses);

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

    int epollfd = epoll_create1(0);

    if (epollfd == -1)
    {
        return capy_log_errno(errno, "Failed to create epoll");
    }

    struct epoll_event event = {
        .events = EPOLLIN,
        .data.ptr = NULL,
    };

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        return capy_log_errno(errno, "Failed to add server socket to epoll");
    }

    if (listen(server_fd, 50) == -1)
    {
        return capy_log_errno(errno, "Failed to listen");
    }

    printf("server: listening at '%s' and '%s' using %lu workers\n", host ? host : "localhost", port, options->workers);

    http_worker *workers = calloc(options->workers, sizeof(http_worker));

    for (size_t i = 0; i < options->workers; i++)
    {
        workers[i].id = i;
        workers[i].epoll_fd = epoll_create1(0);
        workers[i].trace = options->trace;

        if (workers[i].epoll_fd == -1)
        {
            return capy_log_errno(errno, "Failed to create epoll");
        }

        if (pthread_create(&workers[i].thread_id, NULL, conn_worker, &workers[i]) == -1)
        {
            return capy_log_errno(errno, "Failed to create worker thread");
        }
    }

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    struct epoll_event events[10];

    size_t target = 0;

    for (;;)
    {
        int fdcount = epoll_wait(epollfd, events, 10, -1);

        if (fdcount == -1)
        {
            err = errno;

            if (err == EINTR)
            {
                continue;
            }

            return capy_log_errno(errno, "Failed to receive events from epoll");
        }

        for (int i = 0; i < fdcount; i++)
        {
            int conn_fd = accept(server_fd, address_buffer, &address_buffer_size);

            if (conn_fd == -1)
            {
                return capy_log_errno(errno, "Failed to accept connection");
            }

            capy_arena *arena = capy_arena_init(2 * 1024 * 1024);

            if (arena == NULL)
            {
                return capy_log_errno(errno, "Failed to create connection arena");
            }

            http_worker *worker = &workers[target];

            http_conn *conn = capy_arena_make(http_conn, arena, 1);
            conn->arena = arena;
            conn->socket = conn_fd;
            conn->handler = handler;
            conn->state = STATE_INIT;
            conn->worker = worker;

            conn->msg_buffer = capy_strbuf_init(arena, 8 * 1024);
            conn->msg_buffer->arena = NULL;

            conn->arena_marker = capy_arena_top(arena);

            if (conn_epoll(conn, EPOLL_CTL_ADD, EPOLLIN) == -1)
            {
                return capy_log_errno(errno, "Failed to add client to epoll");
            }

            target = (target + 1) % options->workers;
        }
    }

    return err;
}
