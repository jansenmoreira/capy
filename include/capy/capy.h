#ifndef CAPY_H
#define CAPY_H

#include <inttypes.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

//
// Utility Functions
//

size_t align_to(size_t v, size_t n);
size_t next_pow2(size_t v);
int64_t timespec_diff(struct timespec a, struct timespec b);
void nanoseconds_normalize(int64_t *ns, const char **unit);

#ifdef __GNUC__

#define Format(i) __attribute__((format(printf, (i), (i) + 1)))
#define MustCheck __attribute__((warn_unused_result))
#define Unused __attribute__((unused))
#define Ignore (void)!

#else

#define Format(i)
#define MustCheck
#define Unused
#define Ignore

#endif

#define InOut
#define Out

//
// ERROR HANDLING
//

typedef struct capy_err
{
    int code;
    const char *msg;
} capy_err;

Format(2) capy_err capy_err_fmt(int code, const char *fmt, ...);
capy_err capy_err_errno(int err);
capy_err capy_err_wrap(capy_err err, const char *msg);

//
// MEMORY ARENAS
//

// Arenas are used throughout the library to allocate data on the heap. Arenas are simple memory
// allocators that work similarly to a stack. Allocations grow the end of the arena's memory region, and
// freeing shrinks it. Usually, functions that need to allocate memory will ask for more from the arena
// and will not free it. The module that created the arena will free all the allocated memory together at
// the end of its lifecycle. This helps avoid use-after-free errors, and it's easier to understand who should
// be destroying allocated memory.
// https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator
typedef struct capy_arena capy_arena;

// Initializes an Arena.
// At least `min` bytes will always be allocated
// Trying to allocate more than `max` bytes will return ENOMEM.
// If initialization fails, it returns `NULL`.
MustCheck capy_arena *capy_arena_init(size_t min, size_t max);

// Destroys an Arena, freeing all memory.
capy_err capy_arena_destroy(capy_arena *arena);

// Allocates a memory block of `size` bytes in an Arena.
// The memory block has `align` alignment.
// If `zeroinit` is `true`, it zero-initializes the memory block.
// If allocation fails, it returns `NULL`.
MustCheck void *capy_arena_alloc(capy_arena *arena, size_t size, size_t align, int zeroinit);

// Reallocates a memory block from `cur_size` bytes to `new_size` bytes in an Arena.
// If `addr + cur_size` is the end of the Arena, the arena grows and the memory block's address doesn't change.
// Otherwise, a new memory block is allocated, and the data is copied to the new location.
// Returns the address of the memory block.
// If allocation fails, it returns `NULL`.
MustCheck void *capy_arena_realloc(capy_arena *arena, void *addr, size_t cur_size, size_t new_size, int zeroinit);

// Shrinks the arena's used memory to `addr`.
MustCheck capy_err capy_arena_free(capy_arena *arena, void *addr);

// Returns a pointer to the end of an Arena.
void *capy_arena_end(capy_arena *arena);

// Returns the number of bytes allocated by an Arena.
size_t capy_arena_size(capy_arena *arena);

//
// ASSERTIONS
//

void capy_assert_(int condition, const char *file, int line, const char *expression);

#ifdef NDEBUG
#define capy_assert(exp)
#else
#define capy_assert(exp) capy_assert_((exp) ? 1 : 0, __FILE__, __LINE__, #exp)
#endif

//
// LOGGING
//

typedef enum capy_loglevel
{
    CAPY_LOG_MEM = 1 << 0,
    CAPY_LOG_DEBUG = 1 << 1,
    CAPY_LOG_INFO = 1 << 4,
    CAPY_LOG_WARNING = 1 << 5,
    CAPY_LOG_ERROR = 1 << 6,
} capy_loglevel;

void capy_logger_init(FILE *file);
void capy_logger_min_level(capy_loglevel level);
void capy_logger_add_level(capy_loglevel level);
void capy_logger_time_format(const char *fmt);
Format(2) void capy_log(capy_loglevel level, const char *fmt, ...);

//
// CHARACTERS
//

bool capy_char_isdigit(char c);
bool capy_char_ishexdigit(char c);
bool capy_char_isalpha(char c);
bool capy_char_isalphanumeric(char c);
char capy_char_uppercase(char c);
char capy_char_lowercase(char c);
int capy_char_is(char c, const char *chars);

uint32_t capy_unicode_utf16(uint16_t high, uint16_t low);
size_t capy_unicode_utf8encode(char *buffer, uint32_t code);

//
// STRINGS
//

// Strings are an array of `size` bytes stored in `data`.
// Because a String can be a slice of another String, it may not be null-terminated.
// Should always be printed using the "%.*s" format specifier for printf-style functions.
typedef struct capy_string
{
    const char *data;
    size_t size;
} capy_string;

// Creates a null-terminated copy of `input` and returns it in `output`.
// Memory is allocated from `arena`.
// If allocation fails, returns a non-zero error code.
MustCheck capy_err capy_string_copy(capy_arena *arena, Out capy_string *output, capy_string input);

// Creates a null-terminated lowercase copy of `input` and returns it in `output`.
// Memory is allocated from `arena`.
// If allocation fails, returns a non-zero error code.
MustCheck capy_err capy_string_lower(capy_arena *arena, Out capy_string *output, capy_string input);

// Creates a null-terminated uppercase copy of `input` and returns it in `output`.
// Memory is allocated from `arena`.
// If allocation fails, returns a non-zero error code.
MustCheck capy_err capy_string_upper(capy_arena *arena, Out capy_string *output, capy_string input);

// Creates a null-terminated String from the concatenation of `n` Strings in `list`,
// separated by a `separator`, and returns it in `output`.
// Memory is allocated from `arena`.
// The `separator` must be null-terminated.
// If allocation fails, returns a non-zero error code.
MustCheck capy_err capy_string_join(capy_arena *arena, Out capy_string *output, const char *separator, int n, capy_string *list);

// Parses a hexadecimal number from `input` and stores the number in `value`.
// Returns the number of bytes read.
size_t capy_string_parse_hexdigits(Out uint64_t *value, capy_string input);

// Returns a String that is the longest common prefix of `a` and `b`.
capy_string capy_string_prefix(capy_string a, capy_string b);

// Returns a String that is `s` with leading characters found in `set` removed.
capy_string capy_string_ltrim(capy_string s, const char *set);

// Returns a String that is `s` with trailing characters found in `set` removed.
capy_string capy_string_rtrim(capy_string s, const char *set);

// Returns a String that is `s` with leading and trailing characters found in `set` removed.
capy_string capy_string_trim(capy_string s, const char *set);

// Returns true if `a` compares equal to `b`. Otherwise, returns false.
bool capy_string_eq(capy_string a, capy_string b);

// Creates a String of `size` bytes stored in `data`.
capy_string capy_string_bytes(size_t size, const char *data);

// Creates a String from a null-terminated byte sequence stored in `data`.
capy_string capy_string_cstr(const char *data);

// Creates a String from a substring of `s` that starts at `begin` and ends at `end`.
capy_string capy_string_slice(capy_string s, size_t begin, size_t end);

// Creates a String from `s` by removing `size` leftmost bytes.
capy_string capy_string_shl(capy_string s, size_t size);

// Creates a String from `s` by removing `size` rightmost bytes.
capy_string capy_string_shr(capy_string s, size_t size);

//
// Hash Function
//

// Returns a 64-bit hash value for a `key` of `length` bytes.
uint64_t capy_hash(const void *key, uint64_t length);

//
// Vector
// Helper functions to manage generic Vectors
//

// Inserts `count` values from `values` at index `position` in `items`.
// The size of each element is `element_size`.
// If needed, reallocates the `items` memory block using `arena`.
// If `values` is `NULL`, the function will only reserve `count` elements at `position`.
// Returns a pointer to the updated `items`.
// Returns the number of elements in `size`.
// Returns the vector capacity in `capacity`.
// If allocation fails, returns `NULL`.
MustCheck void *capy_vec_insert(capy_arena *arena,
                                void *items,
                                size_t element_size,
                                InOut size_t *capacity,
                                InOut size_t *size,
                                size_t position,
                                size_t count,
                                const void *values);

// Deletes `count` elements at index `position` in `items`.
// The size of each element is `element_size`.
// Returns the size of the vector.
size_t capy_vec_delete(void *items, size_t element_size, size_t size, size_t position, size_t count);

//
// Byte Buffer
//

// Byte Buffers are used throughout the library to compose Strings and to read data from Files/Sockets.
// A buffer will grow to fit new writes using an `arena`. The number of bytes written is stored in `size`.
// As long as `size` is less than `capacity`, the buffer will not need to allocate more memory.
// The resulting data is stored in `data`, but the address of the data can change after a new allocation is made.
typedef struct capy_buffer
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    char *data;
} capy_buffer;

// Initializes a Buffer with `arena` as the memory allocator and allocates `capacity` bytes of space.
// If initialization fails, returns NULL.
MustCheck capy_buffer *capy_buffer_init(capy_arena *arena, size_t capacity);

// Writes a String to the end of the Buffer
// If allocation fails, returns a non-zero error code.
MustCheck capy_err capy_buffer_write_string(capy_buffer *buffer, capy_string input);

// Writes `size` bytes stored in `bytes` to the end of the Buffer
// If allocation fails, returns a non-zero error code.
MustCheck capy_err capy_buffer_write_bytes(capy_buffer *buffer, size_t size, const char *bytes);

// Writes a null-terminated string to the end of the Buffer
// If allocation fails, returns a non-zero error code.
MustCheck capy_err capy_buffer_write_cstr(capy_buffer *buffer, const char *cstr);

// Writes a null-terminator to the end of the Buffer. Buffer `size` will stay the same.
// If allocation fails, returns a non-zero error code.
MustCheck capy_err capy_buffer_write_null(capy_buffer *buffer);

// Writes at most `max` bytes to the end of the buffer using `snprintf`.
// If `max` is 0, a dry-run call to `snprintf` will be made to compute how many bytes are needed.
// Then the Buffer grows to fit the needed space and makes an effective call to `snprintf`.
// If allocation fails, returns a non-zero error code.
Format(3) MustCheck capy_err capy_buffer_write_fmt(capy_buffer *buffer, size_t max, const char *fmt, ...);

// Resizes the Buffer, allocating memory if needed.
// If allocation fails, returns a non-zero error code.
MustCheck capy_err capy_buffer_resize(capy_buffer *buffer, size_t size);

// Deletes the `size` leftmost bytes from the Buffer.
void capy_buffer_shl(capy_buffer *buffer, size_t size);

//
// String Map
//

MustCheck void *capy_strmap_set(capy_arena *arena,
                                void *data,
                                size_t element_size,
                                InOut size_t *capacity,
                                InOut size_t *size,
                                const void *entry);

void *capy_strmap_get(void *data, size_t element_size, size_t capacity, capy_string key);
size_t capy_strmap_delete(void *items, size_t element_size, size_t capacity, size_t size, capy_string key);

typedef struct capy_strset
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_string *items;
} capy_strset;

MustCheck capy_strset *capy_strset_init(capy_arena *arena, size_t capacity);
int capy_strset_has(capy_strset *s, capy_string key);
MustCheck capy_err capy_strset_add(capy_strset *s, capy_string key);
void capy_strset_delete(capy_strset *s, capy_string key);

typedef struct capy_strkv
{
    capy_string key;
    capy_string value;
} capy_strkv;

typedef struct capy_strkvmap
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_strkv *items;
} capy_strkvmap;

MustCheck capy_strkvmap *capy_strkvmap_init(capy_arena *arena, size_t capacity);
capy_strkv *capy_strkvmap_get(capy_strkvmap *m, capy_string key);
MustCheck capy_err capy_strkvmap_set(capy_strkvmap *m, capy_string key, capy_string value);
void capy_strkvmap_delete(capy_strkvmap *m, capy_string key);

typedef struct capy_strkvn
{
    capy_string key;
    capy_string value;
    struct capy_strkvn *next;
} capy_strkvn;

typedef struct capy_strkvnmap
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_strkvn *items;
} capy_strkvnmap;

MustCheck capy_strkvnmap *capy_strkvnmap_init(capy_arena *arena, size_t capacity);
capy_strkvn *capy_strkvnmap_get(capy_strkvnmap *mm, capy_string key);
MustCheck capy_err capy_strkvnmap_set(capy_strkvnmap *mm, capy_string key, capy_string value);
MustCheck capy_err capy_strkvnmap_add(capy_strkvnmap *mm, capy_string key, capy_string value);
void capy_strkvnmap_delete(capy_strkvnmap *mm, capy_string key);
void capy_strkvnmap_clear(capy_strkvnmap *mm);

//
// Base64
//

size_t capy_base64(char *output, const char *encoding, size_t n, const char *input, int padding);
size_t capy_base64url(char *output, size_t n, const char *input, int padding);
size_t capy_base64std(char *output, size_t n, const char *input, int padding);

MustCheck capy_err capy_string_base64(capy_arena *arena, capy_string *output, capy_string input, const char *encoding, int padding);
MustCheck capy_err capy_string_base64url(capy_arena *arena, capy_string *output, capy_string input, int padding);
MustCheck capy_err capy_string_base64std(capy_arena *arena, capy_string *output, capy_string input, int padding);

MustCheck capy_err capy_buffer_write_base64(capy_buffer *buffer, size_t n, const char *input, const char *encoding, int padding);
MustCheck capy_err capy_buffer_write_base64url(capy_buffer *buffer, size_t n, const char *input, int padding);
MustCheck capy_err capy_buffer_write_base64std(capy_buffer *buffer, size_t n, const char *input, int padding);

//
// URI
//

typedef enum capy_uriflags
{
    CAPY_URI_SCHEME = 1 << 0,
    CAPY_URI_AUTHORITY = 1 << 1,
    CAPY_URI_QUERY = 1 << 2,
    CAPY_URI_FRAGMENT = 1 << 3,
} capy_uriflags;

typedef struct capy_uri
{
    capy_string scheme;
    capy_string authority;
    capy_string userinfo;
    capy_string host;
    capy_string port;
    capy_string path;
    capy_string query;
    capy_string fragment;
    int flags;
} capy_uri;

int capy_uri_valid(capy_uri uri);
capy_string capy_uri_string(capy_arena *arena, capy_uri uri);
capy_uri capy_uri_parse(capy_string input);
capy_uri capy_uri_parse_authority(capy_uri uri);
capy_uri capy_uri_resolve_reference(capy_arena *arena, capy_uri base, capy_uri relative);
MustCheck capy_err capy_uri_normalize(capy_arena *arena, capy_string *output, capy_string input, int lowercase);
capy_string capy_uri_path_removedots(capy_arena *arena, capy_string path);

//
// HTTP
//

typedef enum capy_httpprotocol
{
    CAPY_HTTP = 0,
    CAPY_HTTPS,
} capy_httpprotocol;

typedef enum capy_httpstatus
{
    CAPY_HTTP_STATUS_UNK = 0,
    CAPY_HTTP_CONTINUE = 100,
    CAPY_HTTP_SWITCHING_PROTOCOLS = 101,
    CAPY_HTTP_OK = 200,
    CAPY_HTTP_CREATED = 201,
    CAPY_HTTP_ACCEPTED = 202,
    CAPY_HTTP_NON_AUTHORITATIVE_INFORMATION = 203,
    CAPY_HTTP_NO_CONTENT = 204,
    CAPY_HTTP_RESET_CONTENT = 205,
    CAPY_HTTP_PARTIAL_CONTENT = 206,
    CAPY_HTTP_MULTIPLE_CHOICES = 300,
    CAPY_HTTP_MOVED_PERMANENTLY = 301,
    CAPY_HTTP_FOUND = 302,
    CAPY_HTTP_SEE_OTHER = 303,
    CAPY_HTTP_NOT_MODIFIED = 304,
    CAPY_HTTP_USE_PROXY = 305,
    CAPY_HTTP_TEMPORARY_REDIRECT = 307,
    CAPY_HTTP_PERMANENT_REDIRECT = 308,
    CAPY_HTTP_BAD_REQUEST = 400,
    CAPY_HTTP_UNAUTHORIZED = 401,
    CAPY_HTTP_PAYMENT_REQUIRED = 402,
    CAPY_HTTP_FORBIDDEN = 403,
    CAPY_HTTP_NOT_FOUND = 404,
    CAPY_HTTP_METHOD_NOT_ALLOWED = 405,
    CAPY_HTTP_NOT_ACCEPTABLE = 406,
    CAPY_HTTP_PROXY_AUTHENTICATION_REQUIRED = 407,
    CAPY_HTTP_REQUEST_TIMEOUT = 408,
    CAPY_HTTP_CONFLICT = 409,
    CAPY_HTTP_GONE = 410,
    CAPY_HTTP_LENGTH_REQUIRED = 411,
    CAPY_HTTP_PRECONDITION_FAILED = 412,
    CAPY_HTTP_CONTENT_TOO_LARGE = 413,
    CAPY_HTTP_URI_TOO_LONG = 414,
    CAPY_HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
    CAPY_HTTP_RANGE_NOT_SATISFIABLE = 416,
    CAPY_HTTP_EXPECTATION_FAILED = 417,
    CAPY_HTTP_IM_A_TEAPOT = 418,
    CAPY_HTTP_MISDIRECTED_REQUEST = 421,
    CAPY_HTTP_UNPROCESSABLE_ENTITY = 422,
    CAPY_HTTP_UPGRADE_REQUIRED = 426,
    CAPY_HTTP_INTERNAL_SERVER_ERROR = 500,
    CAPY_HTTP_NOT_IMPLEMENTED = 501,
    CAPY_HTTP_BAD_GATEWAY = 502,
    CAPY_HTTP_SERVICE_UNAVAILABLE = 503,
    CAPY_HTTP_GATEWAY_TIMEOUT = 504,
    CAPY_HTTP_HTTP_VERSION_NOT_SUPPORTED = 505,
} capy_httpstatus;

typedef enum capy_httpmethod
{
    CAPY_HTTP_METHOD_UNK,
    CAPY_HTTP_CONNECT,
    CAPY_HTTP_DELETE,
    CAPY_HTTP_GET,
    CAPY_HTTP_HEAD,
    CAPY_HTTP_OPTIONS,
    CAPY_HTTP_PATCH,
    CAPY_HTTP_POST,
    CAPY_HTTP_PUT,
    CAPY_HTTP_TRACE,
} capy_httpmethod;

typedef enum capy_httpversion
{
    CAPY_HTTP_VERSION_UNK,
    CAPY_HTTP_09,
    CAPY_HTTP_10,
    CAPY_HTTP_11,
    CAPY_HTTP_20,
    CAPY_HTTP_30,
} capy_httpversion;

typedef struct capy_httpreq
{
    capy_httpmethod method;
    capy_httpversion version;
    capy_uri uri;
    capy_string uri_raw;
    capy_strkvnmap *headers;
    capy_strkvnmap *trailers;
    capy_strkvnmap *params;
    capy_strkvnmap *query;
    capy_string content;

    size_t content_length;
    int chunked;
    int close;
} capy_httpreq;

typedef struct capy_httpresp
{
    capy_httpstatus status;
    capy_strkvnmap *headers;
    capy_buffer *body;
} capy_httpresp;

typedef capy_err (*capy_http_handler)(capy_arena *arena, capy_httpreq *request, capy_httpresp *response);

typedef struct capy_httproute
{
    capy_httpmethod method;
    capy_string path;
    capy_http_handler handler;
} capy_httproute;

typedef struct capy_httpserveropt
{
    const char *host;
    const char *port;

    int routes_size;

    capy_httproute *routes;

    size_t workers;
    size_t connections;

    size_t line_buffer_size;
    size_t mem_connection_max;

    capy_httpprotocol protocol;
    const char *certificate_chain;
    const char *certificate_key;
} capy_httpserveropt;

capy_httpmethod capy_http_parse_method(capy_string input);
capy_httpversion capy_http_parse_version(capy_string input);

MustCheck capy_err capy_http_parse_reqline(capy_arena *arena, capy_httpreq *request, capy_string input);
MustCheck capy_err capy_http_parse_field(capy_strkvnmap *fields, capy_string line);
MustCheck capy_err capy_http_parse_uriparams(capy_strkvnmap *params, capy_string path, capy_string handler_path);
MustCheck capy_err capy_http_parse_query(capy_strkvnmap *fields, capy_string line);
MustCheck capy_err capy_http_validate_request(capy_arena *arena, capy_httpreq *request);
MustCheck capy_err capy_http_write_response(capy_buffer *buffer, capy_httpresp *response, int close);

capy_err capy_http_serve(capy_httpserveropt options);

//
// JSON
//

typedef enum capy_jsonkind
{
    CAPY_JSON_BOOL,
    CAPY_JSON_NULL,
    CAPY_JSON_NUMBER,
    CAPY_JSON_STRING,
    CAPY_JSON_OBJECT,
    CAPY_JSON_ARRAY,
} capy_jsonkind;

typedef struct capy_json_value
{
    capy_jsonkind kind;
    union
    {
        double number;
        bool boolean;
        const char *string;
        struct capy_jsonobj *object;
        struct capy_jsonarr *array;
    };
} capy_jsonval;

typedef struct capy_jsonkv
{
    capy_string key;
    capy_jsonval value;
} capy_jsonkv;

typedef struct capy_jsonobj
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_jsonkv *items;
} capy_jsonobj;

typedef struct capy_jsonarr
{
    size_t size;
    size_t capacity;
    capy_arena *arena;
    capy_jsonval *data;
} capy_jsonarr;

capy_jsonval capy_json_null(void);
capy_jsonval capy_json_string(const char *string);
capy_jsonval capy_json_number(double number);
capy_jsonval capy_json_bool(int boolean);

MustCheck capy_jsonval capy_json_object(capy_arena *arena);
MustCheck capy_err capy_json_object_set(capy_jsonobj *object, const char *key, capy_jsonval value);
capy_jsonval *capy_json_object_get(capy_jsonobj *object, const char *key);

MustCheck capy_jsonval capy_json_array(capy_arena *arena);
MustCheck capy_err capy_json_array_push(capy_jsonarr *array, capy_jsonval value);

capy_err capy_json_deserialize(capy_arena *arena, capy_jsonval *value, const char *input);
capy_err capy_json_serialize(capy_buffer *buffer, capy_jsonval value, int tabsize);

#undef InOut
#undef Out

#endif
