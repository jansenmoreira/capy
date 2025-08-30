#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <capy/capy.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.c"

typedef struct http_worker
{
    int id;
    pthread_t thread_id;
    int epoll_fd;
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
    STATE_CLOSE_CONNECTION,
} http_connection_state;

static const char *http_connection_state_cstr(http_connection_state state)
{
    switch (state)
    {
        case STATE_UNKNOWN:
            return "STATE_UNKNOWN";
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
        case STATE_CLOSE_CONNECTION:
            return "STATE_CLOSE_CONNECTION";
    }
}

typedef struct http_connection
{
    capy_arena *arena;
    void *arena_marker;
    char *message_buffer;
    http_connection_state state;
    http_connection_state after_read;
    http_connection_state after_write;
    int socket;
    int blocked;
    int worker_id;
    size_t line_cursor;
    char *content_buffer;
    size_t chunk_size;
    capy_http_request *request;
    capy_string response;
    capy_http_handler *handler;
} http_connection;

static inline int log_gaierr(int err, const char *file, int line, const char *msg)
{
    fprintf(stderr, "[%s:%d] => %s: %s\n", file, line, msg, gai_strerror(err));
    return err;
}

static inline int log_errno(int err, const char *file, int line, const char *msg)
{
    fprintf(stderr, "[%s:%d] => %s: %s\n", file, line, msg, strerror(err));
    return err;
}

static inline int connection_message_get_line(http_connection *connection, capy_string *line)
{
    size_t message_size = capy_vec_size(connection->message_buffer);

    while (connection->line_cursor <= message_size)
    {
        if (connection->line_cursor >= 2 &&
            connection->message_buffer[connection->line_cursor - 2] == '\r' &&
            connection->message_buffer[connection->line_cursor - 1] == '\n')
        {
            break;
        }

        connection->line_cursor += 1;
    }

    if (connection->line_cursor > message_size)
    {
        return -1;
    }

    *line = capy_string_bytes(connection->message_buffer, connection->line_cursor - 2);

    return 0;
}

static inline void connection_message_buffer_shl(http_connection *connection, size_t size)
{
    capy_vec_delete(connection->message_buffer, 0, size);

    if (size >= connection->line_cursor)
    {
        connection->line_cursor = 2;
    }
    else
    {
        connection->line_cursor -= size;
    }
}

static inline void connection_message_consume_line(http_connection *connection)
{
    connection_message_buffer_shl(connection, connection->line_cursor);
}

static inline http_connection_state connection_write_response(http_connection *connection)
{
    ssize_t bytes_written = send(connection->socket, connection->response.data, connection->response.size, 0);

    if (bytes_written == 0)
    {
        return STATE_CLOSE_CONNECTION;
    }
    else if (bytes_written < 0)
    {
        log_errno(errno, __FILE__, __LINE__, "Failed to write response");
        return STATE_CLOSE_CONNECTION;
    }

    connection->response = capy_string_shl(connection->response, bytes_written);

    if (connection->response.size == 0)
    {
        return STATE_INIT;
    }

    connection->blocked = true;

    return STATE_WRITE_RESPONSE;
}

static inline http_connection_state connection_parse_request_line(http_connection *connection)
{
    capy_string line;

    if (connection_message_get_line(connection, &line))
    {
        connection->after_read = STATE_PARSE_REQUEST_LINE;
        return STATE_READ_SOCKET;
    }

    if (capy_http_parse_request_line(connection->arena, line, connection->request))
    {
        return STATE_CLOSE_CONNECTION;
    }

    connection_message_consume_line(connection);

    return STATE_PARSE_HEADERS;
}

static inline http_connection_state connection_parse_headers(http_connection *connection)
{
    for (;;)
    {
        capy_string line;

        if (connection_message_get_line(connection, &line))
        {
            connection->after_read = STATE_PARSE_HEADERS;
            return STATE_READ_SOCKET;
        }

        if (line.size == 0)
        {
            connection_message_consume_line(connection);
            break;
        }

        if (capy_http_parse_field(connection->arena, line, &connection->request->headers))
        {
            return STATE_CLOSE_CONNECTION;
        }

        connection_message_consume_line(connection);
    }

    if (capy_http_content_length(connection->request))
    {
        return STATE_CLOSE_CONNECTION;
    }

    if (connection->request->content_length)
    {
        return STATE_PARSE_CONTENT;
    }
    else if (connection->request->chunked)
    {
        return STATE_PARSE_CHUNK_SIZE;
    }

    return STATE_PROCESS_REQUEST;
}

static inline http_connection_state connection_parse_trailers(http_connection *connection)
{
    for (;;)
    {
        capy_string line;

        if (connection_message_get_line(connection, &line))
        {
            connection->after_read = STATE_PARSE_TRAILERS;
            return STATE_READ_SOCKET;
        }

        if (line.size == 0)
        {
            connection_message_consume_line(connection);
            break;
        }

        if (capy_http_parse_field(connection->arena, line, &connection->request->trailers))
        {
            return STATE_CLOSE_CONNECTION;
        }

        connection_message_consume_line(connection);
    }

    return STATE_PROCESS_REQUEST;
}

static inline http_connection_state connection_parse_chunk_size(http_connection *connection)
{
    if (connection->content_buffer == NULL)
    {
        connection->content_buffer = capy_vec_of(char, connection->arena, 1024);
    }

    capy_string line;

    if (connection_message_get_line(connection, &line))
    {
        connection->after_read = STATE_PARSE_CHUNK_SIZE;
        return STATE_READ_SOCKET;
    }

    int64_t value;

    if (capy_string_hex(line, &value) == 0)
    {
        return STATE_CLOSE_CONNECTION;
    }

    connection->chunk_size = value;

    connection_message_consume_line(connection);

    if (connection->chunk_size == 0)
    {
        return STATE_PARSE_TRAILERS;
    }

    return STATE_PARSE_CHUNK_DATA;
}

static inline http_connection_state connection_parse_chunk_data(http_connection *connection)
{
    capy_string buffer = capy_string_bytes(connection->message_buffer, capy_vec_size(connection->message_buffer));

    if (connection->chunk_size + 2 > buffer.size)
    {
        capy_vec_insert(connection->content_buffer, capy_vec_size(connection->content_buffer), buffer.size, (void *)buffer.data);
        connection_message_buffer_shl(connection, buffer.size);

        connection->chunk_size -= buffer.size;

        connection->after_read = STATE_PARSE_CHUNK_DATA;
        return STATE_READ_SOCKET;
    }

    if (strncmp(buffer.data + connection->chunk_size, "\r\n", 2) != 0)
    {
        return STATE_CLOSE_CONNECTION;
    }

    capy_vec_insert(connection->content_buffer, capy_vec_size(connection->content_buffer), connection->chunk_size, (void *)buffer.data);
    connection_message_buffer_shl(connection, connection->chunk_size + 2);

    return STATE_PARSE_CHUNK_SIZE;
}

static inline http_connection_state connection_read_socket(http_connection *connection)
{
    size_t message_limit = capy_vec_capacity(connection->message_buffer);
    size_t message_size = capy_vec_size(connection->message_buffer);

    ssize_t bytes_wanted = message_limit - message_size;

    if (bytes_wanted == 0)
    {
        return STATE_WRITE_RESPONSE;
    }

    ssize_t bytes_read = recv(connection->socket,
                              connection->message_buffer + message_size,
                              bytes_wanted, 0);

    if (bytes_read == 0)
    {
        return STATE_CLOSE_CONNECTION;
    }
    else if (bytes_read < 0)
    {
        log_errno(errno, __FILE__, __LINE__, "Failed to read message headers");
        return STATE_CLOSE_CONNECTION;
    }

    capy_vec_resize(connection->message_buffer, message_size + bytes_read);

    connection->blocked = bytes_read < bytes_wanted;

    return connection->after_read;
}

static inline http_connection_state connection_parse_content(http_connection *connection)
{
    if (connection->content_buffer == NULL)
    {
        connection->content_buffer = capy_vec_of(char, connection->arena, connection->request->content_length);
        capy_vec_fixed(connection->content_buffer);
    }

    size_t message_size = capy_vec_size(connection->message_buffer);

    if (message_size > 0)
    {
        capy_vec_insert(connection->content_buffer, 0, message_size, connection->message_buffer);
        capy_vec_delete(connection->message_buffer, 0, message_size);
    }

    if (capy_vec_size(connection->content_buffer) == connection->request->content_length)
    {
        return STATE_PROCESS_REQUEST;
    }

    connection->after_read = STATE_PARSE_CONTENT;
    return STATE_READ_SOCKET;
}

static inline http_connection_state connection_process_request(http_connection *connection)
{
    if (connection->content_buffer != NULL)
    {
        connection->request->content = connection->content_buffer;
        connection->request->content_length = capy_vec_size(connection->content_buffer);
    }

    capy_http_response response = {NULL};

    if (connection->handler(connection->arena, connection->request, &response))
    {
        return STATE_CLOSE_CONNECTION;
    }

    connection->response = capy_http_write_response(connection->arena, &response);

    return STATE_WRITE_RESPONSE;
}

static http_connection_state connection_message_init(http_connection *connection)
{
    capy_arena_shrink(connection->arena, connection->arena_marker);

    connection->line_cursor = 0;
    connection->after_read = STATE_UNKNOWN;
    connection->after_write = STATE_UNKNOWN;
    connection->request = NULL;
    connection->content_buffer = NULL;
    connection->response = capy_string_literal("");

    connection->request = capy_arena_make(capy_http_request, connection->arena, 1);
    connection->request->headers = capy_smap_of(capy_http_field, connection->arena, 16);
    connection->request->trailers = capy_smap_of(capy_http_field, connection->arena, 16);

    return STATE_PARSE_REQUEST_LINE;
}

static int connection_close(http_connection *connection)
{
    int socket = connection->socket;

    capy_arena_free(connection->arena);

    if (close(socket) == -1)
    {
        return log_errno(errno, __FILE__, __LINE__, "Failed to close connection");
    }

    return 0;
}

static inline int connection_state_machine(http_connection *connection)
{
    capy_assert(connection != NULL);

    connection->blocked = false;

    while (!connection->blocked)
    {
        printf("state = %s, worker = %d, socket = %d, message_buffer = %lu, arena = %.2f KiB\n",
               http_connection_state_cstr(connection->state),
               connection->worker_id,
               connection->socket,
               capy_vec_size(connection->message_buffer),
               ((size_t)(capy_arena_top(connection->arena)) - (size_t)(connection->arena)) / 1024.0f);

        switch (connection->state)
        {
            case STATE_INIT:
                connection->state = connection_message_init(connection);
                break;

            case STATE_READ_SOCKET:
                connection->state = connection_read_socket(connection);
                break;

            case STATE_PARSE_REQUEST_LINE:
                connection->state = connection_parse_request_line(connection);
                break;

            case STATE_PARSE_HEADERS:
                connection->state = connection_parse_headers(connection);
                break;

            case STATE_PARSE_CONTENT:
                connection->state = connection_parse_content(connection);
                break;

            case STATE_PARSE_CHUNK_SIZE:
                connection->state = connection_parse_chunk_size(connection);
                break;

            case STATE_PARSE_CHUNK_DATA:
                connection->state = connection_parse_chunk_data(connection);
                break;

            case STATE_PARSE_TRAILERS:
                connection->state = connection_parse_trailers(connection);
                break;

            case STATE_PROCESS_REQUEST:
                connection->state = connection_process_request(connection);
                break;

            case STATE_WRITE_RESPONSE:
                connection->state = connection_write_response(connection);
                break;

            case STATE_CLOSE_CONNECTION:
                return connection_close(connection);

            default:
                capy_assert(false);
        }
    }

    return 0;
}

static void *connection_worker(void *data)
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

            log_errno(errno, __FILE__, __LINE__, "Failed to receive events from epoll");

            return NULL;
        }

        for (int i = 0; i < fdcount; i++)
        {
            if (connection_state_machine(events[i].data.ptr))
            {
                abort();
            }
        }
    }
}

int capy_http_serve(const char *host, const char *port, size_t workers_count, capy_http_handler handler)
{
    capy_assert(host || port);
    capy_assert(handler != NULL);

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
    int server_fd;

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
        return log_errno(errno, __FILE__, __LINE__, "Failed to bind socket");
    }

    if (listen(server_fd, 50) == -1)
    {
        return log_errno(errno, __FILE__, __LINE__, "Failed to listen");
    }

    printf("server: listening at '%s' and '%s'\n", host ? host : "localhost", port);

    int epollfd = epoll_create1(0);

    if (epollfd == -1)
    {
        return log_errno(errno, __FILE__, __LINE__, "Failed to create epoll");
    }

    struct epoll_event event = {
        .events = EPOLLIN,
        .data.ptr = NULL,
    };

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        return log_errno(errno, __FILE__, __LINE__, "Failed to add server socket to epoll");
    }

    http_worker workers[workers_count];

    for (size_t i = 0; i < workers_count; i++)
    {
        workers[i].id = i;
        workers[i].epoll_fd = epoll_create1(0);

        if (workers[i].epoll_fd == -1)
        {
            return log_errno(errno, __FILE__, __LINE__, "Failed to create epoll");
        }

        if (pthread_create(&workers[i].thread_id, NULL, connection_worker, &workers[i]) == -1)
        {
            return log_errno(errno, __FILE__, __LINE__, "Failed to create worker thread");
        }
    }

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    struct epoll_event events[10];

    int target = 0;

    for (;;)
    {
        int fdcount = epoll_wait(epollfd, events, 10, -1);

        if (fdcount == -1)
        {
            int err = errno;

            if (err == EINTR)
            {
                continue;
            }

            return log_errno(errno, __FILE__, __LINE__, "Failed to receive events from epoll");
        }

        for (int i = 0; i < fdcount; i++)
        {
            int connection_fd = accept(server_fd, address_buffer, &address_buffer_size);

            if (connection_fd == -1)
            {
                return log_errno(errno, __FILE__, __LINE__, "Failed to accept connection");
            }

            capy_arena *arena = capy_arena_init(2 * 1024 * 1024);

            if (arena == NULL)
            {
                return log_errno(errno, __FILE__, __LINE__, "Failed to create connection arena");
            }

            http_connection *connection = capy_arena_make(http_connection, arena, 1);
            connection->arena = arena;
            connection->socket = connection_fd;
            connection->handler = handler;
            connection->state = STATE_INIT;
            connection->worker_id = target;

            connection->message_buffer = capy_vec_of(char, arena, 32 * 1024);
            capy_vec_fixed(connection->message_buffer);

            connection->arena_marker = capy_arena_top(arena);

            struct epoll_event client_event = {
                .events = EPOLLIN | EPOLLOUT,
                .data.ptr = connection,
            };

            if (epoll_ctl(workers[target].epoll_fd, EPOLL_CTL_ADD, connection_fd, &client_event) == -1)
            {
                return log_errno(errno, __FILE__, __LINE__, "Failed to add client to epoll");
            }

            target = (target + 1) % workers_count;
        }
    }

    return err;
}
