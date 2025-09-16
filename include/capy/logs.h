#include <capy/arena.h>

#define CAPY_LOG_MEM 1
#define CAPY_LOG_DEBUG 2
#define CAPY_LOG_INFO 6
#define CAPY_LOG_WARNING 14
#define CAPY_LOG_ERROR 30

typedef struct capy_logger
{
    FILE *file;
    char *buffer;
    unsigned int mask;
} capy_logger;

extern capy_logger *capy_stdout;

void capy_log_init(unsigned int mask);
void capy_logf(capy_logger *logger, unsigned int level, const char *format, ...);

#define capy_mem(...) capy_logf(capy_stdout, CAPY_LOG_MEM, __VA_ARGS__)
#define capy_debug(...) capy_logf(capy_stdout, CAPY_LOG_DEBUG, __VA_ARGS__)
#define capy_info(...) capy_logf(capy_stdout, CAPY_LOG_INFO, __VA_ARGS__)
#define capy_warning(...) capy_logf(capy_stdout, CAPY_LOG_WARNING, __VA_ARGS__)
#define capy_error(...) capy_logf(capy_stdout, CAPY_LOG_ERROR, __VA_ARGS__)
