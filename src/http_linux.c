#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <capy/capy.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define CONNECTION_HEADERS_SIZE 32 * 1024
#define CONNECTION_ARENA_LIMIT 1024 * 1024

typedef enum
{
    STATE_READ_HEADERS,
    STATE_PARSE_HEADERS,
    STATE_READ_CONTENT,
    STATE_READ_CHUNKS,
    STATE_READ_TRAILERS,
    STATE_PROCESS_REQUEST,
    STATE_WRITE_RESPONSE,
    STATE_CLOSE_CONNECTION,
} http_connection_state;

typedef struct http_connection
{
    capy_arena *arena;
    int socket;
    http_connection_state state;
    char *header_buffer;
    size_t header_buffer_size;
    size_t header_buffer_limit;
    size_t header_end;
    capy_http_request *request;
    char *response_buffer;
    size_t response_buffer_size;
    size_t response_buffer_writen;
    capy_http_handler *handler;
    int idle;
} http_connection;

static inline int log_errno(int err, const char *format)
{
    fprintf(stderr, format, strerror(err));
    return err;
}

static inline http_connection_state connection_write_response(http_connection *connection)
{
    size_t bytes_wanted = connection->response_buffer_size - connection->response_buffer_writen;

    int bytes_written = send(connection->socket, connection->response_buffer, bytes_wanted, 0);

    if (bytes_written == 0)
    {
        return STATE_CLOSE_CONNECTION;
    }
    else if (bytes_written < 0)
    {
        log_errno(errno, "send(connection_write_response): %s\n");
        return STATE_CLOSE_CONNECTION;
    }

    connection->response_buffer_writen += bytes_written;

    if (connection->response_buffer_writen == connection->response_buffer_size)
    {
        return STATE_CLOSE_CONNECTION;
    }

    connection->idle = true;

    return STATE_CLOSE_CONNECTION;
}

static inline http_connection_state connection_parse_headers(http_connection *connection)
{
    capy_string request_headers = capy_string_bytes(connection->header_buffer, connection->header_end);
    capy_http_request *request = capy_http_parse_header(connection->arena, request_headers, 1024);

    if (request == NULL)
    {
        return STATE_CLOSE_CONNECTION;
    }

    connection->request = request;

    return STATE_PROCESS_REQUEST;
}

static inline http_connection_state connection_read_headers(http_connection *connection)
{
    size_t bytes_wanted = connection->header_buffer_limit - connection->header_buffer_size;

    if (bytes_wanted == 0)
    {
        return STATE_WRITE_RESPONSE;
    }

    int bytes_read = recv(connection->socket, connection->header_buffer, bytes_wanted, 0);

    if (bytes_read == 0)
    {
        return STATE_CLOSE_CONNECTION;
    }
    else if (bytes_read < 0)
    {
        log_errno(errno, "recv(connection_read_headers): %s\n");
        return STATE_CLOSE_CONNECTION;
    }

    connection->header_buffer_size += bytes_read;

    if (connection->header_end < 4)
    {
        connection->header_end = 4;
    }

    while (connection->header_end <= connection->header_buffer_size)
    {
        const char *buffer = connection->header_buffer + connection->header_end - 4;

        if (strncmp(buffer, "\r\n\r\n", 4) == 0)
        {
            return STATE_PARSE_HEADERS;
        }

        connection->header_end += 1;
    }

    connection->idle = true;

    return STATE_READ_HEADERS;
}

static inline http_connection_state connection_handle_request(http_connection *connection)
{
    capy_http_response response = {NULL};

    if (connection->handler(connection->arena, connection->request, &response))
    {
        return STATE_CLOSE_CONNECTION;
    }

    connection->response_buffer_size = 8 * 1024;
    connection->response_buffer = capy_arena_make(char, connection->arena, 8 * 1024);

    const char *format =
        "HTTP/1.1 %d\r\n"
        "Content-Length: %llu\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%.*s";

    connection->response_buffer_size = snprintf(connection->response_buffer,
                                                connection->response_buffer_size,
                                                format,
                                                response.status,
                                                response.content.size,
                                                (int)response.content.size,
                                                response.content.data);

    return STATE_WRITE_RESPONSE;
}

static inline int connection_handle(http_connection *connection)
{
    capy_assert(connection);

    connection->idle = false;

    while (!connection->idle)
    {
        switch (connection->state)
        {
            case STATE_READ_HEADERS:
                connection->state = connection_read_headers(connection);
                break;

            case STATE_PARSE_HEADERS:
                connection->state = connection_parse_headers(connection);
                break;

            case STATE_PROCESS_REQUEST:
                connection->state = connection_handle_request(connection);
                break;

            case STATE_WRITE_RESPONSE:
                connection->state = connection_write_response(connection);
                break;

            case STATE_CLOSE_CONNECTION:
            default:
            {
                if (close(connection->socket) == -1)
                {
                    log_errno(errno, "close(close_connection): %s\n");
                }

                capy_arena_free(connection->arena);
                return 0;
            }
        }
    }

    return 0;
}

int capy_http_serve(const char *host, const char *port, capy_http_handler handler)
{
    capy_assert(host || port);
    capy_assert(handler);

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    struct addrinfo *server_addresses;

    int err = getaddrinfo(host, port, &hints, &server_addresses);

    if (err)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        return err;
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
        return log_errno(err, "bind: %s");
    }

    if (listen(server_fd, 50) == -1)
    {
        return log_errno(errno, "listen: %s");
    }

    printf("server: listening at '%s' and '%s'\n", host ? host : "localhost", port);

    int epollfd = epoll_create1(0);

    if (epollfd == -1)
    {
        return log_errno(errno, "epoll_create: %s");
    }

    struct epoll_event event = {
        .events = EPOLLIN,
        .data.ptr = NULL,
    };

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        return log_errno(errno, "epoll_ctl(server): %s");
    }

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    struct epoll_event events[10];

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

            return log_errno(err, "epoll_wait: %s");
        }

        for (int i = 0; i < fdcount; i++)
        {
            if (events[i].data.ptr)
            {
                connection_handle(events[i].data.ptr);
            }
            else if (events[i].data.ptr == NULL)
            {
                int connection_fd = accept(server_fd, address_buffer, &address_buffer_size);

                if (connection_fd == -1)
                {
                    return log_errno(errno, "accept: %s");
                }

                capy_arena *arena = capy_arena_init(CONNECTION_ARENA_LIMIT);

                if (arena == NULL)
                {
                    return log_errno(ENOMEM, "capy_arena_init: %s");
                }

                char *header_buffer = capy_arena_make(char, arena, CONNECTION_HEADERS_SIZE);

                http_connection *connection = capy_arena_make(http_connection, arena, 1);
                connection->arena = arena;
                connection->header_buffer = header_buffer;
                connection->header_buffer_limit = CONNECTION_HEADERS_SIZE;
                connection->socket = connection_fd;
                connection->handler = handler;

                struct epoll_event client_event = {
                    .events = EPOLLIN,
                    .data.ptr = connection,
                };

                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connection_fd, &client_event) == -1)
                {
                    return log_errno(errno, "epoll_ctl(client): %s");
                }
            }
        }
    }

    return err;
}
