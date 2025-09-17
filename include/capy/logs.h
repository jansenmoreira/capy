#ifndef CAPY_LOGS_H
#define CAPY_LOGS_H

#include <capy/arena.h>

#define CAPY_LOG_MEM 1
#define CAPY_LOG_DEBUG 30
#define CAPY_LOG_INFO 28
#define CAPY_LOG_WARNING 24
#define CAPY_LOG_ERROR 16

typedef struct capy_logger
{
    FILE *file;
    char *buffer;
    unsigned int mask;
} capy_logger;

extern capy_logger *capy_stdout;

void capy_log_init(unsigned int mask);
void capy_logf(capy_logger *logger, unsigned int level, const char *format, ...);

#define capy_logmem(...) capy_logf(capy_stdout, 1, __VA_ARGS__)
#define capy_logdbg(...) capy_logf(capy_stdout, 2, __VA_ARGS__)
#define capy_loginf(...) capy_logf(capy_stdout, 4, __VA_ARGS__)
#define capy_logwrn(...) capy_logf(capy_stdout, 8, __VA_ARGS__)
#define capy_logerr(...) capy_logf(capy_stdout, 16, __VA_ARGS__)

#endif
