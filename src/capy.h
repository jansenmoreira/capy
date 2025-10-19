#ifndef CAPY_IMPL_H
#define CAPY_IMPL_H

#include <capy/macros.h>

#define TASKQUEUE_REMOVED Cast(size_t, -1)

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
} capy_httpconnstate;

typedef struct capy_tcpconn capy_tcpconn;

typedef struct capy_httpconn
{
    struct capy_tcpconn *tcp;

    size_t conn_id;

    capy_arena *arena;
    void *marker_init;

    ssize_t mem_headers;
    ssize_t mem_content;
    ssize_t mem_trailers;
    ssize_t mem_response;

    capy_httpconnstate state;
    capy_httpconnstate after_read;

    capy_buffer *line_buffer;

    capy_buffer *content_buffer;
    capy_buffer *response_buffer;

    size_t line_cursor;
    size_t chunk_size;

    capy_httpreq request;
    capy_httpresp response;

    capy_httprouter *router;

    capy_httpserveropt *options;

    struct timespec created;
    struct timespec timestamp;
} capy_httpconn;

typedef struct capy_task_ctx capy_task_ctx;

typedef struct capy_task
{
    capy_task_ctx *ctx;
    void *data;
    void (*entrypoint)(void *ctx);
    void (*cleanup)(void *ctx);
    struct timespec deadline;
    capy_fd fd;
    bool write;
    size_t queuepos;
} capy_task;

typedef union capy_taskqueue
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
} capy_taskqueue;

typedef struct capy_poll capy_poll;

typedef struct capy_scheduler
{
    bool cancel;
    capy_arena *arena;
    capy_poll *poll;
    capy_task *main;
    capy_task *poller;
    capy_task *cleaner;
    capy_task *active;
    capy_task *previous;
    capy_taskqueue *queue;
} capy_scheduler;

typedef struct logger_t
{
    FILE *file;
    unsigned int mask;
    const char *timefmt;
} logger_t;

capy_err capy_tcpconn_recv_(capy_tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
capy_err capy_tcpconn_send_(capy_tcpconn *conn, capy_buffer *buffer, uint64_t timeout);
capy_err capy_tcpconn_shutdown_(capy_tcpconn *conn);
capy_err capy_tcpconn_close_(capy_tcpconn *conn);

char *capy_err_buf_(void);

capy_httproutermap *capy_httproutermap_init_(capy_arena *arena, size_t capacity);
capy_httprouter *capy_httproutermap_get_(capy_httproutermap *map, capy_string key);
MustCheck capy_err capy_httproutermap_set_(capy_arena *arena, capy_httproutermap *map, capy_httprouter router);
capy_httprouter *capy_httprouter_add_route_(capy_arena *arena, capy_httprouter *router, capy_httpmethod method, capy_string suffix, capy_string path, capy_http_handler handler);
capy_httprouter *capy_httprouter_init_(capy_arena *arena, int n, capy_httproute *routes);
capy_httproute *capy_httprouter_get_route_(capy_httprouter *router, capy_httpmethod method, capy_string path);
capy_err capy_httprouter_handle_request_(capy_arena *arena, capy_httprouter *router, capy_httpreq *request, capy_httpresp *response);

bool capy_http_validate_string_(capy_string s, int categories, const char *chars);
capy_string capy_http_next_token_(capy_string *buffer, const char *delimiters);
size_t capy_http_consume_chars_(capy_string *buffer, const char *chars, size_t limit);
capy_err capy_http_canonicalize_field_(capy_arena *arena, capy_string *output, capy_string input);
capy_err capy_httpresp_write_status_(capy_httpresp *response);
capy_err capy_httpresp_write_cstr_(capy_httpresp *response, const char *msg);
capy_err capy_http_pctdecode_query_(capy_arena *arena, capy_string *output, capy_string input);

void capy_httpconn_close_(capy_httpconn *conn);
int capy_httpconn_parse_msgline_(capy_httpconn *conn, capy_string *line);
void capy_httpconn_shl_msg_(capy_httpconn *conn, size_t size);
void capy_httpconn_shl_msgline_(capy_httpconn *conn);
capy_err capy_httpconn_parse_reqline_(capy_httpconn *conn);
capy_err capy_httpconn_parse_headers_(capy_httpconn *conn);
capy_err capy_httpconn_parse_trailers_(capy_httpconn *conn);
capy_err capy_httpconn_parse_chunksize_(capy_httpconn *conn);
capy_err capy_httpconn_parse_chunkdata_(capy_httpconn *conn);
capy_err capy_httpconn_parse_reqbody_(capy_httpconn *conn);
capy_err capy_httpconn_prepare_badrequest_(capy_httpconn *conn);
capy_err capy_httpconn_route_request_(capy_httpconn *conn);
capy_err capy_httpconn_reset_(capy_httpconn *conn);
void capy_httpconn_trace_(capy_httpconn *conn);
capy_err capy_httpconn_write_response_(capy_httpconn *conn);
capy_err capy_httpconn_read_request_(capy_httpconn *conn);
capy_err capy_httpconn_run_(capy_httpconn *conn);

void capy_poll_(void *data);
capy_err capy_poll_init_(capy_scheduler *scheduler);
capy_err capy_poll_add_(capy_scheduler *scheduler, capy_task *task);
capy_err capy_poll_remove_(capy_scheduler *scheduler, capy_task *task);
capy_err capy_poll_destroy_(capy_scheduler *scheduler);

capy_task *capy_task_init_(capy_arena *arena, size_t size, void (*entrypoint)(void *data), void (*cleanup)(void *data), void *data);
void capy_scheduler_switch_(capy_scheduler *scheduler, capy_task *task);
capy_err capy_scheduler_init_(void);
capy_err capy_scheduler_shutdown_(capy_scheduler *scheduler, int timeout);
capy_err capy_scheduler_waitfd_(capy_scheduler *scheduler, capy_task *task, capy_fd fd, bool write, uint64_t timeout);
capy_err capy_scheduler_sleep_(capy_scheduler *scheduler, capy_task *task, uint64_t timeout);
void capy_scheduler_clean_(void *data);

capy_task_ctx *capy_task_ctx_init(capy_arena *arena, void *stack, void (*entrypoint)(void));
void capy_task_switch_(capy_task_ctx *next, capy_task_ctx *current);
void capy_task_entrypoint_(void);

capy_taskqueue *capy_taskqueue_init_(capy_arena *arena);
int64_t capy_taskqueue_cmp_(capy_taskqueue *queue, size_t a, size_t b);
void capy_taskqueue_swap_(capy_taskqueue *queue, size_t a, size_t b);
void capy_taskqueue_siftup_(capy_taskqueue *queue, size_t node);
void capy_taskqueue_siftdown_(capy_taskqueue *queue, size_t node);
capy_err capy_taskqueue_add_(capy_taskqueue *queue, capy_task *value);
capy_task *capy_taskqueue_peek_(capy_taskqueue *queue);
capy_task *capy_taskqueue_remove_(capy_taskqueue *queue, size_t node);
capy_task *capy_taskqueue_pop_(capy_taskqueue *queue);
int64_t capy_taskqueue_timeout_(capy_taskqueue *queue);

uint8_t capy_uri_char_categories_(char c);
capy_string capy_uri_pctenc_shl_(capy_string input);
int capy_uri_pctenc_validate_(capy_string input, int categories, const char *chars);
size_t capy_uri_string_validate_(capy_string s, int categories, const char *chars);
capy_string capy_uri_parse_hexword_(capy_string input);
capy_string capy_uri_parse_decoctet_(capy_string input);
size_t capy_uri_valid_userinfo_(capy_string userinfo);
size_t capy_uri_valid_port_(capy_string port);
size_t capy_uri_valid_fragment_(capy_string fragment);
size_t capy_uri_valid_query_(capy_string query);
size_t capy_uri_valid_reg_name_(capy_string host);
size_t capy_uri_valid_segment_(capy_string segment);
size_t capy_uri_valid_ipv4_(capy_string host);
size_t capy_uri_valid_ipv6_(capy_string host);
size_t capy_uri_valid_host_(capy_string host);
size_t capy_uri_valid_scheme_(capy_string scheme);
size_t capy_uri_valid_path_(capy_string path);
capy_string capy_uri_path_merge_(capy_arena *arena, capy_string base, capy_string reference);

#endif
