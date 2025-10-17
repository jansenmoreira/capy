#ifndef CAPY_IMPL_H
#define CAPY_IMPL_H

#include <capy/macros.h>

#define TASKQUEUE_REMOVED Cast(size_t, -1)

typedef struct tcpconn tcpconn;

typedef union capy_httproutermap
{
    capy_strmap strmap;
    struct
    {
        size_t size;
        size_t capacity;
        size_t element_size;
        struct capy_httprouter *items;
    };
} capy_httproutermap;

typedef struct capy_httprouter
{
    capy_string segment;
    capy_httproutermap *segments;
    capy_httproute routes[10];
} capy_httprouter;

typedef enum
{
    STATE_UNKNOWN,
    STATE_RESET,
    STATE_READ_REQUEST,
    STATE_PARSE_REQLINE,
    STATE_PARSE_HEADERS,
    STATE_PARSE_CONTENT,
    STATE_PARSE_CHUNKSIZE,
    STATE_PARSE_CHUNKDATA,
    STATE_PARSE_TRAILERS,
    STATE_ROUTE_REQUEST,
    STATE_WRITE_RESPONSE,
    STATE_BAD_REQUEST,
    STATE_SERVER_FAILURE,
    STATE_SSL_SHUTDOWN,
    STATE_CLOSE,
} httpconnstate;

typedef struct httpconn
{
    tcpconn *tcp;

    size_t conn_id;

    capy_arena *arena;
    void *marker_init;

    ssize_t mem_headers;
    ssize_t mem_content;
    ssize_t mem_trailers;
    ssize_t mem_response;

    httpconnstate state;
    httpconnstate after_read;

    capy_buffer *line_buffer;

    capy_buffer *content_buffer;
    capy_buffer *response_buffer;

    size_t line_cursor;
    size_t chunk_size;

    capy_httpreq request;
    capy_httpresp response;

    capy_httprouter *router;

    capy_httpserveropt *options;

    struct timespec timestamp;
} httpconn;

#if defined(CAPY_OS_LINUX) && defined(__x86_64__)
typedef struct taskabi
{
    uintptr_t rsp;
    uintptr_t rbp;
    uintptr_t rbx;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;
} taskabi;
#endif

typedef struct capy_task
{
    taskabi abi;
    void *ctx;
    void (*cleanup)(void *);
    struct timespec deadline;
    capy_fd fd;
    bool write;
    size_t queuepos;
} capy_task;

typedef union taskqueue
{
    capy_vec vec;
    struct
    {
        size_t size;
        size_t capacity;
        size_t element_size;
        capy_task **data;
        capy_arena *arena;
    };
} taskqueue;

typedef struct logger_t
{
    FILE *file;
    unsigned int mask;
    const char *timefmt;
} logger_t;

static capy_err tcpconn_recv(tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
static capy_err tcpconn_send(tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
static capy_err tcpconn_shutdown(tcpconn *conn);
static capy_err tcpconn_close(tcpconn *conn);

static char *capy_err_buf(void);

static bool http_validate_string(capy_string s, int categories, const char *chars);
static capy_string http_next_token(capy_string *buffer, const char *delimiters);
static size_t http_consume_chars(capy_string *buffer, const char *chars, size_t limit);
static capy_err http_canonical_field_name(capy_arena *arena, capy_string *output, capy_string input);
static capy_httproutermap *capy_http_router_map_init(capy_arena *arena, size_t capacity);
static capy_httprouter *capy_http_router_map_get(capy_httproutermap *map, capy_string key);
static MustCheck capy_err capy_http_router_map_set(capy_arena *arena, capy_httproutermap *map, capy_httprouter router);
static capy_httprouter *http_route_add_(capy_arena *arena, capy_httprouter *router, capy_httpmethod method, capy_string suffix, capy_string path, capy_http_handler handler);
static capy_err http_response_status(capy_httpresp *response);
static capy_err http_response_wcstr(capy_httpresp *response, const char *msg);
static capy_err http_query_pctdecode(capy_arena *arena, capy_string *output, capy_string input);
static capy_httprouter *capy_http_router_init(capy_arena *arena, int n, capy_httproute *routes);
static capy_httproute *capy_http_route_get(capy_httprouter *router, capy_httpmethod method, capy_string path);
static capy_err capy_http_router_handle(capy_arena *arena, capy_httprouter *router, capy_httpreq *request, capy_httpresp *response);

static void httpconn_close(httpconn *conn);
static int httpconn_parse_msgline(httpconn *conn, capy_string *line);
static void httpconn_shl_msg(httpconn *conn, size_t size);
static void httpconn_shl_msgline(httpconn *conn);
static capy_err httpconn_parse_reqline(httpconn *conn);
static capy_err httpconn_parse_headers(httpconn *conn);
static capy_err httpconn_parse_trailers(httpconn *conn);
static capy_err httpconn_parse_chunksize(httpconn *conn);
static capy_err httpconn_parse_chunkdata(httpconn *conn);
static capy_err httpconn_parse_reqbody(httpconn *conn);
static capy_err httpconn_prepare_badrequest(httpconn *conn);
static capy_err httpconn_route_request(httpconn *conn);
static capy_err httpconn_reset(httpconn *conn);
static void httpconn_trace(httpconn *conn);
static capy_err httpconn_write_response(httpconn *conn);
static capy_err httpconn_read_request(httpconn *conn);
static capy_err httpconn_run(httpconn *conn);

static void taskscheduler_done(void);
static capy_err taskscheduler_unsubscribe(capy_task *task);
static capy_err taskscheduler_subscribe(capy_task *task);
static capy_err taskscheduler_enqueue(capy_task *task);
static capy_err taskscheduler_wait(void);
static bool taskscheduler_canceled(void);
static capy_err taskscheduler_active(capy_task **task);
static capy_err taskscheduler_join(Unused int timeout);

static void task_set_stack(taskabi *abi, void *stack);
static void task_switch_(capy_task *next, capy_task *current);
static void task_destroy(void);

static taskqueue *taskqueue_init(capy_arena *arena);
static int64_t taskqueue_cmp(taskqueue *queue, size_t a, size_t b);
static void taskqueue_swap(taskqueue *queue, size_t a, size_t b);
static void taskqueue_siftup(taskqueue *queue, size_t node);
static void taskqueue_siftdown(taskqueue *queue, size_t node);
static capy_err taskqueue_add(taskqueue *queue, capy_task *value);
static capy_task *taskqueue_peek(taskqueue *queue);
static capy_task *taskqueue_remove(taskqueue *queue, size_t node);
static capy_task *taskqueue_pop(taskqueue *queue);
static int64_t taskqueue_timeout(taskqueue *queue);

static uint8_t uri_char_categories(char c);
static capy_string uri_pctenc_shl(capy_string input);
static int uri_pctenc_validate(capy_string input, int categories, const char *chars);
static size_t uri_string_validate(capy_string s, int categories, const char *chars);
static capy_string uri_parse_hex_word(capy_string input);
static capy_string uri_parse_dec_octet(capy_string input);
static size_t uri_valid_userinfo(capy_string userinfo);
static size_t uri_valid_port(capy_string port);
static size_t uri_valid_fragment(capy_string fragment);
static size_t uri_valid_query(capy_string query);
static size_t uri_valid_reg_name(capy_string host);
static size_t uri_valid_segment(capy_string segment);
static size_t uri_valid_ipv4(capy_string host);
static size_t uri_valid_ipv6(capy_string host);
static size_t uri_valid_host(capy_string host);
static size_t uri_valid_scheme(capy_string scheme);
static size_t uri_valid_path(capy_string path);
static capy_string uri_path_merge(capy_arena *arena, capy_string base, capy_string reference);

#ifdef CAPY_OS_LINUX

struct capy_arena
{
    size_t used;
    size_t capacity;
    size_t min;
    size_t max;
    size_t size;
    size_t page_size;
};

typedef struct tcpconn
{
    int socket;
    struct ssl_st *ssl;
    bool ssl_fatal;
    int want_write;
    struct httpworker *worker;
    size_t connId;
    capy_task *task;
} tcpconn;

typedef struct httpserver
{
    capy_httprouter *router;
    capy_httpserveropt options;
    struct ssl_ctx_st *ssl_ctx;
} httpserver;

typedef struct taskscheduler
{
    int fd;
    int signal_fd;
    bool cancel;
    taskqueue *queue;
    capy_arena *arena;
    capy_task main;
    capy_task *poller;
    capy_task *cleaner;
    capy_task *active;
    capy_task *previous;
} taskscheduler;

static capy_err http_gai_error(int err);
static int http_ssl_log_error(const char *buffer, size_t len, Unused void *userdata);
static capy_err tcpconn_recv_tls(tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
static capy_err tcpconn_send_tls(tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
static void tcpconn_cleanup(void *data);
static void tcpconn_run(void);
static capy_err httpserver_init_openssl(httpserver *server);
static capy_httpserveropt httpserver_options_with_defaults(capy_httpserveropt options);
static capy_err httpserver_listen(int *serverfd, capy_httpserveropt *options);
static httpconn *httpserver_create_connection(httpserver *server, int conn_fd);
static capy_err httpserver_accept(httpserver *server, int server_fd);
static void *httpserver_serve(void *data);

static void taskscheduler_switch(capy_task *handle);
static void taskscheduler_poll(void);
static void taskcheduler_clean(void);
static capy_err taskcheduler_init(void);

#endif  // ifdef CAPY_OS_LINUX

#endif  // ifdef CAPY_IMPL_H
