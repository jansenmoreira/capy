#ifndef CAPY_LINUX_IMPL_H
#define CAPY_LINUX_IMPL_H

#include <arpa/inet.h>
#include <sys/socket.h>

#include "capy.h"

struct capy_arena
{
    size_t used;
    size_t capacity;
    size_t min;
    size_t max;
    size_t size;
    size_t page_size;
};

struct capy_tcpconn
{
    int fd;
    struct ssl_st *ssl;
    bool ssl_fatal;
    char addr[INET6_ADDRSTRLEN];
    uint16_t port;
};

typedef struct capy_httpserver
{
    capy_httprouter *router;
    capy_httpserveropt options;
    struct ssl_ctx_st *ssl_ctx;
    int fd;
    char addr[INET6_ADDRSTRLEN];
    uint16_t port;
} capy_httpserver;

typedef struct capy_poll
{
    int fd;
    int signal_fd;
} capy_poll;

capy_err capy_tcpconn_tlsrecv_(capy_tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
capy_err capy_tcpconn_tlssend_(capy_tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
void capy_tcpconn_cleanup_(void *data);
void capy_tcpconn_run_(void *);

void capy_socket_address_(char *output, uint16_t *port, struct sockaddr *sa);
capy_err capy_gaierr_(int err);
int capy_log_sslerr_(const char *buffer, size_t len, Unused void *userdata);
capy_err capy_httpserver_init_openssl_(capy_httpserver *server);
capy_httpserveropt capy_httpserver_default_options_(capy_httpserveropt options);
capy_err capy_httpserver_listen_(capy_httpserver *server);
capy_err capy_httpserver_accept_(capy_httpserver *server);
capy_httpconn *capy_httpserver_create_connection_(capy_httpserver *server, int conn_fd, struct sockaddr *);
void *capy_httpserver_serve_(void *data);

#endif
