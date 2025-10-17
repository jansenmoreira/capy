#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "capy.h"

// STATIC DEFINITIONS

static capy_err http_gai_error(int err)
{
    if (err == 0) return Ok;
    return (capy_err){.code = err, .msg = gai_strerror(err)};
}

static int http_ssl_log_error(const char *buffer, size_t len, Unused void *userdata)
{
    len = (len > 0) ? len - 1 : 0;
    LogErr("%.*s", (int)len, buffer);
    return 0;
}

static capy_err tcpconn_close(tcpconn *conn)
{
    if (conn->ssl)
    {
        SSL_free(conn->ssl);
    }

    close(conn->socket);
    conn->socket = 0;
    return Ok;
}

static capy_err tcpconn_recv(tcpconn *conn, capy_buffer *buffer, uint64_t timeout)
{
    if (conn->ssl)
    {
        return tcpconn_recv_tls(conn, buffer, timeout);
    }

    size_t bytes_wanted = buffer->capacity - buffer->size;

    for (;;)
    {
        ssize_t bytes_read = recv(conn->socket, buffer->data + buffer->size, bytes_wanted, 0);

        if (bytes_read == -1)
        {
            capy_err err = ErrStd(errno);

            if (err.code == EWOULDBLOCK || err.code == EAGAIN)
            {
                capy_err err = capy_task_waitfd(conn->socket, false, timeout);

                if (err.code)
                {
                    return err;
                }

                continue;
            }

            return err;
        }

        buffer->size += Cast(size_t, bytes_read);
        return Ok;
    }
}

static capy_err tcpconn_recv_tls(tcpconn *conn, capy_buffer *buffer, uint64_t timeout)
{
    size_t bytes_wanted = buffer->capacity - buffer->size;
    size_t bytes_read = 0;

    for (;;)
    {
        ERR_clear_error();

        if (!SSL_read_ex(conn->ssl, buffer->data + buffer->size, bytes_wanted, &bytes_read))
        {
            int ssl_err = SSL_get_error(conn->ssl, 0);

            switch (ssl_err)
            {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                {
                    capy_err err = capy_task_waitfd(conn->socket, ssl_err == SSL_ERROR_WANT_WRITE, timeout);

                    if (err.code)
                    {
                        return err;
                    }

                    continue;
                }

                case SSL_ERROR_ZERO_RETURN:
                {
                    return Ok;
                }

                case SSL_ERROR_SYSCALL:
                {
                    conn->ssl_fatal = true;
                    return ErrStd(errno);
                }

                default:
                {
                    conn->ssl_fatal = true;
                    return ErrStd(EPROTO);
                }
            }
        }

        buffer->size += Cast(size_t, bytes_read);
        return Ok;
    }
}

static capy_err tcpconn_send(tcpconn *conn, capy_buffer *buffer, uint64_t timeout)
{
    if (conn->ssl)
    {
        return tcpconn_send_tls(conn, buffer, timeout);
    }

    capy_err err;

    for (;;)
    {
        ssize_t bytes_written = send(conn->socket, buffer->data, buffer->size, 0);

        if (bytes_written == -1)
        {
            err = ErrStd(errno);

            if (err.code == EWOULDBLOCK || err.code == EAGAIN)
            {
                capy_err err = capy_task_waitfd(conn->socket, true, timeout);

                if (err.code)
                {
                    return err;
                }

                continue;
            }

            return err;
        }

        capy_buffer_shl(buffer, Cast(size_t, bytes_written));
        return Ok;
    }
}

static capy_err tcpconn_send_tls(tcpconn *conn, capy_buffer *buffer, uint64_t timeout)
{
    size_t bytes_written = 0;

    for (;;)
    {
        ERR_clear_error();

        if (!SSL_write_ex(conn->ssl, buffer->data, buffer->size, &bytes_written))
        {
            int ssl_err = SSL_get_error(conn->ssl, 0);

            switch (ssl_err)
            {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                {
                    capy_err err = capy_task_waitfd(conn->socket, ssl_err == SSL_ERROR_WANT_WRITE, timeout);

                    if (err.code)
                    {
                        return err;
                    }

                    continue;
                }

                case SSL_ERROR_ZERO_RETURN:
                {
                    return Ok;
                }

                case SSL_ERROR_SYSCALL:
                {
                    conn->ssl_fatal = true;
                    return ErrStd(errno);
                }

                default:
                {
                    conn->ssl_fatal = true;
                    return ErrStd(EPROTO);
                }
            }
        }

        capy_buffer_shl(buffer, Cast(size_t, bytes_written));
        return Ok;
    }
}

static capy_err tcpconn_shutdown(tcpconn *conn)
{
    if (conn->ssl)
    {
        SSL_shutdown(conn->ssl);
    }

    shutdown(conn->socket, SHUT_RDWR);
    return Ok;
}

static void tcpconn_cleanup(void *data)
{
    httpconn *conn = data;
    capy_arena_destroy(conn->arena);
}

static void tcpconn_run(void)
{
    capy_task *task = capy_task_active();
    httpconn *conn = capy_task_ctx(task);
    httpconn_run(conn);
}

static capy_err httpserver_init_openssl(httpserver *server)
{
    server->ssl_ctx = SSL_CTX_new(TLS_server_method());

    if (server->ssl_ctx == NULL)
    {
        ERR_print_errors_cb(http_ssl_log_error, NULL);
        return ErrFmt(EPROTO, "Failed to create server SSL_CTX");
    }

    if (!SSL_CTX_set_min_proto_version(server->ssl_ctx, TLS1_2_VERSION))
    {
        ERR_print_errors_cb(http_ssl_log_error, NULL);
        return ErrFmt(EPROTO, "Failed to set the minimum TLS protocol version");
    }

    SSL_CTX_set_options(server->ssl_ctx, (SSL_OP_IGNORE_UNEXPECTED_EOF |
                                          SSL_OP_NO_RENEGOTIATION |
                                          SSL_OP_CIPHER_SERVER_PREFERENCE));

    if (!SSL_CTX_use_certificate_chain_file(server->ssl_ctx, server->options.certificate_chain))
    {
        ERR_print_errors_cb(http_ssl_log_error, NULL);
        return ErrFmt(EPROTO, "Failed to load the server certificate chain file");
    }

    if (!SSL_CTX_use_PrivateKey_file(server->ssl_ctx, server->options.certificate_key, SSL_FILETYPE_PEM))
    {
        ERR_print_errors_cb(http_ssl_log_error, NULL);
        return ErrFmt(EPROTO, "Failed to load the server private key file");
    }

    static const unsigned char id[] = "capy-https-server";

    SSL_CTX_set_session_id_context(server->ssl_ctx, id, sizeof(id));
    SSL_CTX_set_session_cache_mode(server->ssl_ctx, SSL_SESS_CACHE_SERVER);
    SSL_CTX_sess_set_cache_size(server->ssl_ctx, 1024);
    SSL_CTX_set_timeout(server->ssl_ctx, 3600);
    SSL_CTX_set_verify(server->ssl_ctx, SSL_VERIFY_NONE, NULL);

    return Ok;
}

static capy_httpserveropt httpserver_options_with_defaults(capy_httpserveropt options)
{
    if (!options.workers)
    {
        options.workers = (size_t)(sysconf(_SC_NPROCESSORS_CONF));
    }

    if (options.host == NULL)
    {
        options.host = "127.0.0.1";
    }

    if (options.port == NULL)
    {
        options.port = "8080";
    }

    if (!options.inactivity_timeout)
    {
        options.inactivity_timeout = Seconds(30);
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

static capy_err httpserver_listen(int *serverfd, capy_httpserveropt *options)
{
    *serverfd = -1;

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    struct addrinfo *addresses;

    capy_err err = http_gai_error(getaddrinfo(options->host, options->port, &hints, &addresses));

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

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (int[]){1}, sizeof(int)) == -1)
        {
            err = ErrStd(errno);
            break;
        }

        if (bind(fd, address->ai_addr, address->ai_addrlen) == -1)
        {
            err = ErrStd(errno);
            continue;
        }

        *serverfd = fd;
        break;
    }

    freeaddrinfo(addresses);

    if (err.code)
    {
        return err;
    }

    int flags = fcntl(*serverfd, F_GETFL);

    if (flags == -1)
    {
        return ErrStd(errno);
    }

    if (fcntl(*serverfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        return ErrStd(errno);
    }

    if (listen(*serverfd, 2048) == -1)
    {
        return ErrStd(errno);
    }

    return err;
}

static httpconn *httpserver_create_connection(httpserver *server, int conn_fd)
{
    SSL *ssl = NULL;

    if (server->options.protocol == CAPY_HTTPS)
    {
        ssl = SSL_new(server->ssl_ctx);

        if (ssl == NULL)
        {
            ERR_print_errors_cb(http_ssl_log_error, NULL);
            return NULL;
        }

        if (!SSL_set_fd(ssl, conn_fd))
        {
            ERR_print_errors_cb(http_ssl_log_error, NULL);
            return NULL;
        }

        SSL_set_accept_state(ssl);
    }

    capy_arena *arena = capy_arena_init(server->options.line_buffer_size + KiB(4), server->options.mem_connection_max);

    if (arena == NULL)
    {
        return NULL;
    }

    httpconn *conn = Make(arena, httpconn, 1);

    conn->arena = arena;
    conn->router = server->router;
    conn->options = &server->options;
    conn->line_buffer = capy_buffer_init(arena, server->options.line_buffer_size);
    conn->line_buffer->arena = NULL;
    conn->state = STATE_RESET;
    conn->tcp = Make(arena, tcpconn, 1);
    conn->tcp->ssl = ssl;
    conn->tcp->socket = conn_fd;
    conn->tcp->task = capy_task_init(arena, KiB(16), tcpconn_run, conn);

    if (conn->tcp->task == NULL)
    {
        capy_arena_destroy(arena);
        return NULL;
    }

    capy_task_set_cleanup(conn->tcp->task, tcpconn_cleanup);
    conn->marker_init = capy_arena_end(arena);
    conn->timestamp = capy_now();

    return conn;
}

static capy_err httpserver_accept(httpserver *server, int server_fd)
{
    capy_err err;

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    int keepcnt = 4;
    int keepidle = 60;
    int keepintvl = 15;

    unsigned int timeout = Cast(unsigned int, (keepidle + (keepcnt * keepintvl)) * 1000);

    for (;;)
    {
        err = capy_task_waitfd(server_fd, false, 0);

        if (err.code)
        {
            return err;
        }

        for (;;)
        {
            int conn_fd = accept4(server_fd, address_buffer, &address_buffer_size, SOCK_NONBLOCK);

            if (conn_fd == -1)
            {
                err = ErrStd(errno);

                if (err.code == EAGAIN || err.code == EWOULDBLOCK)
                {
                    break;
                }

                return err;
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

            httpconn *conn = httpserver_create_connection(server, conn_fd);

            if (conn == NULL)
            {
                return ErrWrap(ErrStd(ENOMEM), "Failed to create connection");
            }

            conn->conn_id = Cast(size_t, conn_fd);

            LogInf("New connection %d", conn_fd);

            err = capy_task_enqueue(conn->tcp->task, conn_fd, false, server->options.inactivity_timeout);

            if (err.code)
            {
                return ErrWrap(err, "Failed to add task");
            }
        }
    }
}

static void *httpserver_serve(void *data)
{
    httpserver *server = data;
    int server_fd;

    capy_err err = httpserver_listen(&server_fd, &server->options);

    if (err.code)
    {
        LogErr("Failed to bind and listen: %s", err.msg);
        return NULL;
    }

    LogDbg("worker: thread %zu listening for connections", capy_thread_id());

    err = httpserver_accept(server, server_fd);

    if (err.code != ECANCELED)
    {
        LogErr("Failed to accept connections: %s", err.msg);
        return NULL;
    }

    close(server_fd);
    capy_tasks_join(0);

    return NULL;
}

// PUBLIC DEFINITIONS

capy_err capy_http_serve(capy_httpserveropt options)
{
    capy_err err;

    capy_arena *arena = capy_arena_init(0, MiB(1));

    if (arena == NULL)
    {
        return ErrFmt(ENOMEM, "Failed to create server arena");
    }

    httpserver *server = Make(arena, httpserver, 1);

    if (server == NULL)
    {
        capy_arena_destroy(arena);
        return ErrFmt(ENOMEM, "Failed to create server");
    }

    server->router = capy_http_router_init(arena, options.routes_size, options.routes);

    if (server->router == NULL)
    {
        capy_arena_destroy(arena);
        return ErrFmt(ENOMEM, "Failed to create server router");
    }

    options = httpserver_options_with_defaults(options);

    server->options = options;

    if (options.protocol == CAPY_HTTPS)
    {
        err = httpserver_init_openssl(server);

        if (err.code)
        {
            capy_arena_destroy(arena);
            return ErrWrap(err, "Failed to initialize server SSL context");
        }
    }

    LogInf("server: address = %s:%s, protocol = %s, workers = %lu",
           options.host, options.port,
           (options.protocol == CAPY_HTTPS) ? "https" : "http",
           options.workers);

    sigset_t signals;

    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);
    sigprocmask(SIG_BLOCK, &signals, NULL);

    pthread_t *ids = Make(arena, pthread_t, options.workers);

    for (size_t i = 0; i < options.workers; i++)
    {
        pthread_create(ids + i, NULL, httpserver_serve, server);
    }

    for (size_t i = 0; i < options.workers; i++)
    {
        pthread_join(ids[i], NULL);
    }

    if (server->ssl_ctx != NULL)
    {
        SSL_CTX_free(server->ssl_ctx);
    }

    capy_arena_destroy(arena);

    return Ok;
}
