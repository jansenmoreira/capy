#include <capy/capy.h>
#include <capy/macros.h>

typedef struct logger_t
{
    FILE *file;
    unsigned int mask;
    const char *timefmt;
} logger_t;

static logger_t logger = {
    .file = NULL,
    .mask = 0,
};

void capy_logger_init(FILE *file)
{
    logger.file = file;
    logger.timefmt = "%F %T";
    setvbuf(stdout, NULL, _IOLBF, 0);
    capy_logger_min_level(CAPY_LOG_INFO);
}

void capy_logger_min_level(capy_loglevel level)
{
    logger.mask = 0;

    switch (level)
    {
        case CAPY_LOG_MEM:
            logger.mask |= CAPY_LOG_MEM;
        case CAPY_LOG_DEBUG:
            logger.mask |= CAPY_LOG_DEBUG;
        case CAPY_LOG_INFO:
            logger.mask |= CAPY_LOG_INFO;
        case CAPY_LOG_WARNING:
            logger.mask |= CAPY_LOG_WARNING;
        case CAPY_LOG_ERROR:
            logger.mask |= CAPY_LOG_ERROR;
    }
}

void capy_logger_add_level(capy_loglevel level)
{
    logger.mask |= level;
}

void capy_logger_time_format(const char *fmt)
{
    logger.timefmt = fmt;
}

static const char *levelmsg[] = {
    [CAPY_LOG_MEM] = "\x1b[32m[MEM]\x1b[0m",
    [CAPY_LOG_DEBUG] = "\x1b[35m[DBG]\x1b[0m",
    [CAPY_LOG_INFO] = "[INF]",
    [CAPY_LOG_WARNING] = "\x1b[33m[WRN]\x1b[0m",
    [CAPY_LOG_ERROR] = "\x1b[31m[ERR]\x1b[0m",
};

void capy_log(capy_loglevel level, const char *format, ...)
{
    thread_local static char log_buffer[KiB(2)];

    if (level & logger.mask)
    {
        ssize_t max = KiB(2);
        ssize_t n = 0;

        struct timespec timestamp;
        timespec_get(&timestamp, TIME_UTC);

        n += (ssize_t)strftime(log_buffer + n, (size_t)max, logger.timefmt, gmtime(&timestamp.tv_sec));
        max = max - n;

        n += snprintf(log_buffer + n, (size_t)max, ".%03ld", timestamp.tv_nsec / 1000000);
        max = max - n;

        n += snprintf(log_buffer + n, (size_t)max, " %s ", levelmsg[level]);
        max = max - n;

        va_list args;
        va_start(args, format);
        n += vsnprintf(log_buffer + n, (size_t)max, format, args);
        va_end(args);

        fprintf(logger.file, "%.*s\n", (int)n, log_buffer);
    }
}
