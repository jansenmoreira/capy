#ifndef CAPY_LINUX_IMPL_H
#define CAPY_LINUX_IMPL_H

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
    int socket;
    struct ssl_st *ssl;
    bool ssl_fatal;
    capy_task *task;
};

typedef struct capy_httpserver
{
    capy_httprouter *router;
    capy_httpserveropt options;
    struct ssl_ctx_st *ssl_ctx;
} capy_httpserver;

typedef struct capy_poll
{
    int fd;
    int signal_fd;
    bool cancel;
} capy_poll;

capy_err capy_tcpconn_tlsrecv_(capy_tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
capy_err capy_tcpconn_tlssend_(capy_tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
void capy_tcpconn_cleanup_(void *data);
void capy_tcpconn_run_(void *);

capy_err capy_gaierr_(int err);
int capy_log_sslerr_(const char *buffer, size_t len, Unused void *userdata);
capy_err capy_httpserver_init_openssl_(capy_httpserver *server);
capy_httpserveropt capy_httpserver_default_options_(capy_httpserveropt options);
capy_err capy_httpserver_listen_(int *serverfd, capy_httpserveropt *options);
capy_httpconn *capy_httpserver_create_connection_(capy_httpserver *server, int conn_fd);
capy_err capy_httpserver_accept_(capy_httpserver *server, int server_fd);
void *capy_httpserver_serve_(void *data);

#endif
