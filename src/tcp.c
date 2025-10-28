#include <capy/macros.h>

Platform static capy_tcp *tcp_init(capy_arena *arena);
Platform static const char *tcp_addr(capy_tcp *tcp);
Platform static uint16_t tcp_port(capy_tcp *tcp);
Platform static capy_err tcp_listen(struct capy_tcp *tcp, const char *host, const char *port, size_t backlog);
Platform static capy_err tcp_connect(struct capy_tcp *tcp, const char *host, const char *port);
Platform static capy_err tcp_accept(struct capy_tcp *server, struct capy_tcp *client);
Platform static capy_err tcp_recv(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout);
Platform static capy_err tcp_send(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout);
Platform static capy_err tcp_shutdown(capy_tcp *tcp);
Platform static capy_err tcp_close(capy_tcp *tcp);
Platform static capy_err tcp_keepalive(capy_tcp *tcp, int enabled, int idle, int count, int interval);
Platform static capy_err tcp_nodelay(capy_tcp *tcp, int enabled);
Platform static capy_err tcp_tls_server(capy_tcp *server, const char *chain, const char *key);
Platform static capy_err tcp_tls_client(capy_tcp *client);
Platform static capy_fd tcp_fd(capy_tcp *tcp);

//
// PUBLIC DEFINITIONS
//

capy_tcp *capy_tcp_init(capy_arena *arena)
{
    return tcp_init(arena);
}

capy_err capy_tcp_tls_server(capy_tcp *server, const char *chain, const char *key)
{
    return tcp_tls_server(server, chain, key);
}

capy_err capy_tcp_tls_client(capy_tcp *client)
{
    return tcp_tls_client(client);
}

capy_err capy_tcp_listen(struct capy_tcp *tcp, const char *host, const char *port, size_t backlog)
{
    return tcp_listen(tcp, host, port, backlog);
}

capy_err capy_tcp_connect(struct capy_tcp *tcp, const char *host, const char *port)
{
    return tcp_connect(tcp, host, port);
}

capy_err capy_tcp_accept(struct capy_tcp *server, struct capy_tcp *client)
{
    return tcp_accept(server, client);
}

capy_err capy_tcp_recv(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout)
{
    return tcp_recv(tcp, buffer, timeout);
}

capy_err capy_tcp_send(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout)
{
    return tcp_send(tcp, buffer, timeout);
}

const char *capy_tcp_addr(capy_tcp *tcp)
{
    return tcp_addr(tcp);
}

uint16_t capy_tcp_port(capy_tcp *tcp)
{
    return tcp_port(tcp);
}

capy_err capy_tcp_shutdown(capy_tcp *tcp)
{
    return tcp_shutdown(tcp);
}

capy_err capy_tcp_close(capy_tcp *tcp)
{
    if (tcp == NULL)
    {
        return Ok;
    }

    return tcp_close(tcp);
}

capy_err capy_tcp_keepalive(capy_tcp *tcp, bool enabled, int idle, int count, int interval)
{
    return tcp_keepalive(tcp, enabled, idle, count, interval);
}

capy_err capy_tcp_nodelay(capy_tcp *tcp, bool enabled)
{
    return tcp_nodelay(tcp, enabled);
}

capy_fd capy_tcp_fd(capy_tcp *tcp)
{
    return tcp_fd(tcp);
}

//
// LINUX
//

#ifdef CAPY_OS_LINUX

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

Linux struct capy_tcp
{
    int fd;
    struct ssl_ctx_st *ssl_ctx;
    struct ssl_st *ssl;
    bool ssl_fatal;
    char addr[INET6_ADDRSTRLEN];
    uint16_t port;
};

Linux static capy_err tcp_recv_tls(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout);
Linux static capy_err tcp_send_tls(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout);
Linux static int tcp_sslerr(const char *buffer, size_t len, Unused void *userdata);
Linux static void tcp_get_address(char *output, uint16_t *port, struct sockaddr *sa);

//

Linux static int tcp_sslerr(const char *buffer, size_t len, Unused void *userdata)
{
    len = (len > 0) ? len - 1 : 0;
    LogErr("%.*s", (int)len, buffer);
    return 0;
}

Linux static capy_tcp *tcp_init(capy_arena *arena)
{
    struct capy_tcp *tcp = Make(arena, capy_tcp, 1);

    if (tcp == NULL)
    {
        return NULL;
    }

    tcp->fd = -1;

    return tcp;
}

Linux static const char *tcp_addr(capy_tcp *tcp)
{
    return tcp->addr;
}

Linux static uint16_t tcp_port(capy_tcp *tcp)
{
    return tcp->port;
}

Linux static void tcp_get_address(char *output, uint16_t *port, struct sockaddr *sa)
{
    switch (sa->sa_family)
    {
        case AF_INET:
        {
            struct sockaddr_in *sa4 = ReinterpretCast(struct sockaddr_in *, sa);
            *port = ntohs(sa4->sin_port);
            inet_ntop(AF_INET, &sa4->sin_addr, output, INET6_ADDRSTRLEN);
        }
        break;

        case AF_INET6:
        {
            struct sockaddr_in *sa6 = ReinterpretCast(struct sockaddr_in *, sa);
            *port = ntohs(sa6->sin_port);
            inet_ntop(AF_INET6, &sa6->sin_addr, output, INET6_ADDRSTRLEN);
        }
        break;
    }
}

Linux static capy_err tcp_tls_server(struct capy_tcp *server, const char *chain, const char *key)
{
    server->ssl_ctx = SSL_CTX_new(TLS_server_method());

    if (server->ssl_ctx == NULL)
    {
        ERR_print_errors_cb(tcp_sslerr, NULL);
        return ErrFmt(EPROTO, "Failed to create server SSL_CTX");
    }

    if (!SSL_CTX_set_min_proto_version(server->ssl_ctx, TLS1_2_VERSION))
    {
        ERR_print_errors_cb(tcp_sslerr, NULL);
        return ErrFmt(EPROTO, "Failed to set the minimum TLS protocol version");
    }

    SSL_CTX_set_options(server->ssl_ctx, (SSL_OP_IGNORE_UNEXPECTED_EOF |
                                          SSL_OP_NO_RENEGOTIATION |
                                          SSL_OP_CIPHER_SERVER_PREFERENCE));

    if (!SSL_CTX_use_certificate_chain_file(server->ssl_ctx, chain))
    {
        ERR_print_errors_cb(tcp_sslerr, NULL);
        return ErrFmt(EPROTO, "Failed to load the server certificate chain file");
    }

    if (!SSL_CTX_use_PrivateKey_file(server->ssl_ctx, key, SSL_FILETYPE_PEM))
    {
        ERR_print_errors_cb(tcp_sslerr, NULL);
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

Linux static capy_err tcp_tls_client(struct capy_tcp *client)
{
    client->ssl_ctx = SSL_CTX_new(TLS_client_method());

    if (client->ssl_ctx == NULL)
    {
        ERR_print_errors_cb(tcp_sslerr, NULL);
        return ErrFmt(EPROTO, "Failed to create client SSL context");
    }

    if (!SSL_CTX_set_default_verify_paths(client->ssl_ctx))
    {
        ERR_print_errors_cb(tcp_sslerr, NULL);
        return ErrFmt(EPROTO, "Failed to set the default trusted certificate store");
    }

    if (!SSL_CTX_set_min_proto_version(client->ssl_ctx, TLS1_2_VERSION))
    {
        ERR_print_errors_cb(tcp_sslerr, NULL);
        return ErrFmt(EPROTO, "Failed to set the minimum TLS protocol version");
    }

    SSL_CTX_set_verify(client->ssl_ctx, SSL_VERIFY_PEER, NULL);

    return Ok;
}

Linux static capy_err tcp_listen(struct capy_tcp *tcp, const char *host, const char *port, size_t backlog)
{
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    struct addrinfo *addresses;

    int gaierr = getaddrinfo(host, port, &hints, &addresses);

    if (gaierr != 0)
    {
        return (capy_err){.code = gaierr, .msg = gai_strerror(gaierr)};
    }

    capy_err err;

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

        tcp->fd = fd;
        tcp_get_address(tcp->addr, &tcp->port, address->ai_addr);
        break;
    }

    freeaddrinfo(addresses);

    if (err.code)
    {
        return err;
    }

    int socketflags = fcntl(tcp->fd, F_GETFL);

    if (socketflags == -1)
    {
        return ErrStd(errno);
    }

    if (fcntl(tcp->fd, F_SETFL, socketflags | O_NONBLOCK) == -1)
    {
        return ErrStd(errno);
    }

    if (listen(tcp->fd, Cast(int, backlog)) == -1)
    {
        return ErrStd(errno);
    }

    return Ok;
}

Linux static capy_err tcp_connect(struct capy_tcp *tcp, const char *host, const char *port)
{
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *addresses;

    int gaierr = getaddrinfo(host, port, &hints, &addresses);

    if (gaierr != 0)
    {
        return (capy_err){.code = gaierr, .msg = gai_strerror(gaierr)};
    }

    capy_err err;

    for (struct addrinfo *address = addresses; address != NULL; address = address->ai_next)
    {
        err = Ok;

        int fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

        if (fd == -1)
        {
            err = ErrStd(errno);
            continue;
        }

        if (connect(fd, address->ai_addr, address->ai_addrlen) == -1)
        {
            err = ErrStd(errno);
            continue;
        }

        tcp->fd = fd;
        tcp_get_address(tcp->addr, &tcp->port, address->ai_addr);
        break;
    }

    freeaddrinfo(addresses);

    if (err.code)
    {
        return err;
    }

    int socketflags = fcntl(tcp->fd, F_GETFL);

    if (socketflags == -1)
    {
        return ErrStd(errno);
    }

    if (fcntl(tcp->fd, F_SETFL, socketflags | O_NONBLOCK) == -1)
    {
        return ErrStd(errno);
    }

    return Ok;
}

Linux static capy_err tcp_accept(struct capy_tcp *server, struct capy_tcp *client)
{
    struct sockaddr_storage address_buffer[1];
    struct sockaddr *address = ReinterpretCast(struct sockaddr *, address_buffer);
    socklen_t address_size = sizeof(struct sockaddr_storage);

    for (;;)
    {
        client->fd = accept4(server->fd, address, &address_size, SOCK_NONBLOCK);

        if (client->fd == -1)
        {
            capy_err err = ErrStd(errno);

            if (err.code == EWOULDBLOCK || err.code == EAGAIN)
            {
                err = capy_waitfd(server->fd, false, 0);

                if (err.code)
                {
                    return err;
                }

                continue;
            }

            return ErrWrap(err, "Failed to accept connection");
        }

        tcp_get_address(client->addr, &client->port, address);

        capy_err err;

        if (server->ssl_ctx != NULL)
        {
            client->ssl = SSL_new(server->ssl_ctx);

            if (client->ssl == NULL)
            {
                ERR_print_errors_cb(tcp_sslerr, &err);
                return ErrFmt(EPROTO, "Failed to create SSL object");
            }

            if (!SSL_set_fd(client->ssl, client->fd))
            {
                ERR_print_errors_cb(tcp_sslerr, &err);
                return ErrFmt(EPROTO, "Failed to set SSL fd");
            }

            SSL_set_accept_state(client->ssl);
        }

        return Ok;
    }
}

Linux static capy_err tcp_recv(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout)
{
    if (tcp->ssl != NULL)
    {
        return tcp_recv_tls(tcp, buffer, timeout);
    }

    size_t bytes_wanted = buffer->capacity - buffer->size;

    if (bytes_wanted == 0)
    {
        return Ok;
    }

    for (;;)
    {
        ssize_t bytes_read = recv(tcp->fd, buffer->data + buffer->size, bytes_wanted, 0);

        if (bytes_read == -1)
        {
            capy_err err = ErrStd(errno);

            if (err.code == EWOULDBLOCK || err.code == EAGAIN)
            {
                capy_err err = capy_waitfd(tcp->fd, false, timeout);

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

Linux static capy_err tcp_recv_tls(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout)
{
    size_t bytes_wanted = buffer->capacity - buffer->size;
    size_t bytes_read = 0;

    if (bytes_wanted == 0)
    {
        return Ok;
    }

    for (;;)
    {
        ERR_clear_error();

        if (!SSL_read_ex(tcp->ssl, buffer->data + buffer->size, bytes_wanted, &bytes_read))
        {
            int ssl_err = SSL_get_error(tcp->ssl, 0);

            switch (ssl_err)
            {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                {
                    capy_err err = capy_waitfd(tcp->fd, ssl_err == SSL_ERROR_WANT_WRITE, timeout);

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
                    tcp->ssl_fatal = true;
                    return ErrStd(errno);
                }

                default:
                {
                    tcp->ssl_fatal = true;
                    return ErrStd(EPROTO);
                }
            }
        }

        buffer->size += Cast(size_t, bytes_read);
        return Ok;
    }
}

Linux static capy_err tcp_send(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout)
{
    if (tcp->ssl != NULL)
    {
        return tcp_send_tls(tcp, buffer, timeout);
    }

    capy_err err;

    for (;;)
    {
        ssize_t bytes_written = send(tcp->fd, buffer->data, buffer->size, 0);

        if (bytes_written == -1)
        {
            err = ErrStd(errno);

            if (err.code == EWOULDBLOCK || err.code == EAGAIN)
            {
                capy_err err = capy_waitfd(tcp->fd, true, timeout);

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

Linux static capy_err tcp_send_tls(capy_tcp *tcp, capy_buffer *buffer, uint64_t timeout)
{
    size_t bytes_written = 0;

    for (;;)
    {
        ERR_clear_error();

        if (!SSL_write_ex(tcp->ssl, buffer->data, buffer->size, &bytes_written))
        {
            int ssl_err = SSL_get_error(tcp->ssl, 0);

            switch (ssl_err)
            {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                {
                    capy_err err = capy_waitfd(tcp->fd, ssl_err == SSL_ERROR_WANT_WRITE, timeout);

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
                    tcp->ssl_fatal = true;
                    return ErrStd(errno);
                }

                default:
                {
                    tcp->ssl_fatal = true;
                    return ErrStd(EPROTO);
                }
            }
        }

        capy_buffer_shl(buffer, Cast(size_t, bytes_written));
        return Ok;
    }
}

Linux static capy_err tcp_shutdown(capy_tcp *tcp)
{
    if (tcp->ssl != NULL)
    {
        SSL_shutdown(tcp->ssl);
    }

    shutdown(tcp->fd, SHUT_RDWR);
    return Ok;
}

Linux static capy_err tcp_close(capy_tcp *tcp)
{
    if (tcp->ssl != NULL)
    {
        SSL_free(tcp->ssl);
    }

    if (tcp->ssl_ctx != NULL)
    {
        SSL_CTX_free(tcp->ssl_ctx);
    }

    close(tcp->fd);

    tcp->ssl = NULL;
    tcp->ssl_ctx = NULL;
    tcp->fd = -1;

    return Ok;
}

Linux static capy_err tcp_keepalive(capy_tcp *tcp, int enabled, int idle, int count, int interval)
{
    if (setsockopt(tcp->fd, SOL_SOCKET, SO_KEEPALIVE, &enabled, sizeof(int)) == -1)
    {
        return ErrWrap(ErrStd(errno), "Failed to set SO_KEEPALIVE value");
    }

    if (setsockopt(tcp->fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(int)) == -1)
    {
        return ErrWrap(ErrStd(errno), "Failed to set TCP_KEEPCNT value");
    }

    if (setsockopt(tcp->fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int)) == -1)
    {
        return ErrWrap(ErrStd(errno), "Failed to set TCP_KEEPIDLE value");
    }

    if (setsockopt(tcp->fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int)) == -1)
    {
        return ErrWrap(ErrStd(errno), "Failed to set TCP_KEEPINTVL value");
    }

    return Ok;
}

Linux static capy_err tcp_nodelay(capy_tcp *tcp, int enabled)
{
    if (setsockopt(tcp->fd, IPPROTO_TCP, TCP_NODELAY, &enabled, sizeof(int)) == -1)
    {
        return ErrWrap(ErrStd(errno), "Failed to set TCP_NODELAY value");
    }

    return Ok;
}

Linux static capy_fd tcp_fd(capy_tcp *tcp)
{
    return tcp->fd;
}

#endif
