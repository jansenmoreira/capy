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

typedef struct tcpconn
{
    int socket;
    struct ssl_st *ssl;
    bool ssl_fatal;
    int want_write;
    struct httpworker *worker;
    size_t connId;
} tcpconn;

static capy_err httpconn_update_epoll(httpconn *conn, int epoll_fd, int op, uint32_t events)
{
    events |= (conn->tcp->want_write) ? EPOLLOUT : EPOLLIN;

    struct epoll_event client_event = {
        .events = events,
        .data.ptr = conn,
    };

    if (epoll_ctl(epoll_fd, op, conn->tcp->socket, &client_event) == -1)
    {
        return ErrWrap(ErrStd(errno), "Failed to update workers epoll");
    }

    return Ok;
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

static capy_err tcpconn_recv_tls(tcpconn *conn, capy_buffer *buffer)
{
    ERR_clear_error();

    size_t bytes_wanted = buffer->capacity - buffer->size;
    size_t bytes_read = 0;

    if (!SSL_read_ex(conn->ssl, buffer->data + buffer->size, bytes_wanted, &bytes_read))
    {
        int ssl_err = SSL_get_error(conn->ssl, 0);

        switch (ssl_err)
        {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
            {
                conn->want_write = (ssl_err == SSL_ERROR_WANT_WRITE);
                return ErrStd(EWOULDBLOCK);
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

static capy_err tcpconn_recv(tcpconn *conn, capy_buffer *buffer)
{
    if (conn->ssl)
    {
        return tcpconn_recv_tls(conn, buffer);
    }

    capy_err err;

    size_t bytes_wanted = buffer->capacity - buffer->size;
    ssize_t bytes_read = recv(conn->socket, buffer->data + buffer->size, bytes_wanted, 0);

    if (bytes_read == -1)
    {
        err = ErrStd(errno);

        if (err.code == EWOULDBLOCK || err.code == EAGAIN)
        {
            conn->want_write = false;
            return ErrStd(EWOULDBLOCK);
        }

        return err;
    }

    buffer->size += Cast(size_t, bytes_read);
    return Ok;
}

static capy_err tcpconn_send_tls(tcpconn *conn, capy_buffer *buffer)
{
    ERR_clear_error();

    size_t bytes_written = 0;

    if (!SSL_write_ex(conn->ssl, buffer->data, buffer->size, &bytes_written))
    {
        int ssl_err = SSL_get_error(conn->ssl, 0);

        switch (ssl_err)
        {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
            {
                conn->want_write = (ssl_err == SSL_ERROR_WANT_WRITE);
                return ErrStd(EWOULDBLOCK);
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

static capy_err tcpconn_send(tcpconn *conn, capy_buffer *buffer)
{
    if (conn->ssl)
    {
        return tcpconn_send_tls(conn, buffer);
    }

    capy_err err;

    ssize_t bytes_written = send(conn->socket, buffer->data, buffer->size, 0);

    if (bytes_written == -1)
    {
        err = ErrStd(errno);

        if (err.code == EWOULDBLOCK || err.code == EAGAIN)
        {
            conn->want_write = true;
            return ErrStd(EWOULDBLOCK);
        }

        return err;
    }

    capy_buffer_shl(buffer, Cast(size_t, bytes_written));
    return Ok;
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

typedef struct httpworker
{
    size_t id;
    pthread_t thread_id;
    int epoll_fd;
    sigset_t *mask;
    struct httpconn **connections;
} httpworker;

static void *httpworker_loop(void *data)
{
    capy_err err;

    httpworker *worker = data;

    struct epoll_event events[16];

    for (;;)
    {
        int events_count = epoll_pwait(worker->epoll_fd, events, 16, -1, worker->mask);

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

            httpconn *conn = events[i].data.ptr;

            if (conn->tcp->worker == NULL)
            {
                conn->tcp->worker = worker;
                conn->co_parent = capy_co_active();

                bool full = true;

                for (size_t i = 0; i < conn->options->connections; i++)
                {
                    if (worker->connections[i] == NULL)
                    {
                        worker->connections[i] = conn;
                        conn->conn_id = i;
                        full = false;
                        break;
                    }
                }

                if (full)
                {
                    httpconn_close(conn);
                    capy_arena_destroy(conn->arena);
                    continue;
                }
            }

            active_conn = conn;
            capy_co_switch(conn->co_self);

            if (conn->tcp->socket != 0)
            {
                httpconn_update_epoll(conn, worker->epoll_fd, EPOLL_CTL_MOD, EPOLLET | EPOLLONESHOT);
            }
            else
            {
                worker->connections[conn->conn_id] = NULL;
                capy_arena_destroy(conn->arena);
            }
        }
    }
}

typedef struct httpserver
{
    capy_arena *arena;
    capy_httprouter *router;
    int fd;
    int epoll_fd;
    int workers_epoll_fd;
    int signal_fd;
    sigset_t signals;
    httpworker *workers;
    capy_httpserveropt options;
    struct ssl_ctx_st *ssl_ctx;
} httpserver;

static capy_err httpserver_free_openssl(httpserver *server)
{
    SSL_CTX_free(server->ssl_ctx);
    return Ok;
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
        SSL_CTX_free(server->ssl_ctx);
        return ErrFmt(EPROTO, "Failed to set the minimum TLS protocol version");
    }

    SSL_CTX_set_options(server->ssl_ctx, (SSL_OP_IGNORE_UNEXPECTED_EOF |
                                          SSL_OP_NO_RENEGOTIATION |
                                          SSL_OP_CIPHER_SERVER_PREFERENCE));

    if (!SSL_CTX_use_certificate_chain_file(server->ssl_ctx, server->options.certificate_chain))
    {
        ERR_print_errors_cb(http_ssl_log_error, NULL);
        SSL_CTX_free(server->ssl_ctx);
        return ErrFmt(EPROTO, "Failed to load the server certificate chain file");
    }

    if (!SSL_CTX_use_PrivateKey_file(server->ssl_ctx, server->options.certificate_key, SSL_FILETYPE_PEM))
    {
        ERR_print_errors_cb(http_ssl_log_error, NULL);
        SSL_CTX_free(server->ssl_ctx);
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

    if (!options.connections)
    {
        options.connections = 2048;
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

static capy_err httpserver_bind(httpserver *server)
{
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    struct addrinfo *addresses;

    capy_err err = http_gai_error(getaddrinfo(server->options.host, server->options.port, &hints, &addresses));

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

static capy_err httpserver_create_signal_fd(httpserver *server)
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

static capy_err httpserver_create_epoll(httpserver *server)
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

static capy_err httpserver_create_workers_epoll(httpworker *worker, int signal_fd)
{
    struct epoll_event event;

    worker->epoll_fd = epoll_create1(0);

    if (worker->epoll_fd == -1)
    {
        return ErrStd(errno);
    }

    event = (struct epoll_event){
        .events = EPOLLIN,
        .data.u64 = SIGNAL_EPOLL_EVENT,
    };

    if (epoll_ctl(worker->epoll_fd, EPOLL_CTL_ADD, signal_fd, &event) == -1)
    {
        return ErrStd(errno);
    }

    return Ok;
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

    conn->co_self = capy_co_init(arena, KiB(16), httpconn_run);

    if (conn->co_self == NULL)
    {
        capy_arena_destroy(arena);
        return NULL;
    }

    conn->marker_init = capy_arena_end(arena);

    timespec_get(&conn->timestamp, TIME_UTC);
    return conn;
}

static capy_err httpserver_create_workers(httpserver *server)
{
    capy_err err;

    server->workers = Make(server->arena, httpworker, server->options.workers);

    if (server->workers == NULL)
    {
        return ErrStd(ENOMEM);
    }

    for (size_t i = 0; i < server->options.workers; i++)
    {
        httpworker *worker = server->workers + i;

        worker->id = i;
        worker->epoll_fd = server->workers_epoll_fd;
        worker->mask = &server->signals;

        worker->connections = Make(server->arena, httpconn *, server->options.connections);

        if (worker->connections == NULL)
        {
            return ErrStd(ENOMEM);
        }

        err = httpserver_create_workers_epoll(server->workers + i, server->signal_fd);

        if (err.code)
        {
            return err;
        }

        int code = pthread_create(&worker->thread_id, NULL, httpworker_loop, worker);

        if (code)
        {
            return ErrStd(code);
        }
    }

    return Ok;
}

static void httpserver_destroy(httpserver *server)
{
    for (size_t i = 0; i < server->options.workers; i++)
    {
        httpworker *worker = server->workers + i;

        pthread_join(worker->thread_id, NULL);

        for (size_t i = 0; i < server->options.workers; i++)
        {
            httpconn *conn = worker->connections[i];

            if (conn == NULL)
            {
                continue;
            }

            if (conn->tcp->socket != 0)
            {
                httpconn_close(conn);
            }

            capy_arena_destroy(conn->arena);
        }
    }

    if (server->options.protocol == CAPY_HTTPS)
    {
        httpserver_free_openssl(server);
    }

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

    options = httpserver_options_with_defaults(options);

    httpserver server = {
        .router = router,
        .arena = arena,
        .options = options,
    };

    err = httpserver_bind(&server);

    if (err.code)
    {
        return ErrWrap(err, "Failed to bind server");
    }

    err = httpserver_create_signal_fd(&server);

    if (err.code)
    {
        return ErrWrap(err, "Failed to create signal file descriptors");
    }

    err = httpserver_create_epoll(&server);

    if (err.code)
    {
        return ErrWrap(err, "Failed to create listen epoll");
    }

    if (options.protocol == CAPY_HTTPS)
    {
        err = httpserver_init_openssl(&server);

        if (err.code)
        {
            return ErrWrap(err, "Failed to initialize SSL context");
        }
    }

    err = httpserver_create_workers(&server);

    if (err.code)
    {
        return ErrWrap(err, "Failed to create workers pool");
    }

    if (listen(server.fd, 10) == -1)
    {
        return ErrWrap(ErrStd(errno), "Failed to listen");
    }

    LogInf("server: listening at %s:%s (protocol = %s, workers = %lu, connections = %lu)",
           server.options.host, server.options.port,
           (options.protocol == CAPY_HTTPS) ? "https" : "http",
           server.options.workers, server.options.connections);

    void *address_buffer = &(struct sockaddr_storage){0};
    socklen_t address_buffer_size = sizeof(address_buffer);

    int keepcnt = 4;
    int keepidle = 60;
    int keepintvl = 15;

    unsigned int timeout = Cast(unsigned int, (keepidle + (keepcnt * keepintvl)) * 1000);

    struct epoll_event events[10];

    size_t workerId = 0;

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

            httpconn *conn = httpserver_create_connection(&server, conn_fd);

            if (conn == NULL)
            {
                return ErrWrap(ErrStd(ENOMEM), "Failed to create connection");
            }

            httpworker *worker = server.workers + workerId;

            err = httpconn_update_epoll(conn, worker->epoll_fd, EPOLL_CTL_ADD, EPOLLET | EPOLLONESHOT);

            if (err.code)
            {
                return err;
            }

            workerId = (workerId + 1) % server.options.workers;
        }
    }

    httpserver_destroy(&server);

    LogInf("server: shutting down");

    return Ok;
}
