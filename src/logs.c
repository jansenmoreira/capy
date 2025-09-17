#include <capy/capy.h>
#include <capy/macros.h>
#include <stdarg.h>
#include <threads.h>
#include <time.h>

thread_local static char log_buffer[KiB(2)];

static capy_logger capy_stdout_ = {
    .file = NULL,
    .mask = 0,
};

capy_logger *capy_stdout = &capy_stdout_;

void capy_log_init(unsigned int mask)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    capy_stdout->file = stdout;
    capy_stdout->mask = mask;
}

void capy_logf(capy_logger *logger, unsigned int level, const char *format, ...)
{
    if (level & logger->mask)
    {
        ssize_t max = KiB(2);
        ssize_t n = 0;

        struct timespec timestamp;
        timespec_get(&timestamp, TIME_UTC);

        n += (ssize_t)strftime(log_buffer + n, (size_t)max, "%X", gmtime(&timestamp.tv_sec));
        max = max - n;

        n += snprintf(log_buffer + n, (size_t)max, ".%03ld ", timestamp.tv_nsec / 1000000);
        max = max - n;

        va_list args;
        va_start(args, format);
        n += vsnprintf(log_buffer + n, (size_t)max, format, args);
        va_end(args);

        fprintf(logger->file, "%.*s\n", (int)n, log_buffer);
    }
}
