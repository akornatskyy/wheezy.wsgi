
#include <wsgi_core.h>


static const char *log_level[] = {
    "[EMERG]",
    "[ERROR]",
    "[INFO]",
    "[DEBUG]"
};

static const char *log_source[WSGI_LOG_MAX_SOURCE + 1];

static wsgi_log_t log;


static void wsgi_printf_log_handler(const wsgi_log_t *log,
                                    const char *msg, size_t size);


wsgi_log_t *wsgi_log_init()
{
    u_int i;
    for(i = 1; i < WSGI_LOG_MAX_SOURCE; i++) {
        log_source[i] = "undefined";
    }

    log.log_level = WSGI_LOG_DEBUG;
    log.log_source = WSGI_LOG_SOURCE_ALL;
    log.handler = wsgi_printf_log_handler;
    return &log;
}


void wsgi_log_msg(const wsgi_log_t *log, u_int level, u_int source,
                  const char *fmt, ...)
{
    va_list args;
    char msg[WSGI_MAX_MSG];
    char *p;
    size_t n, s;
    struct tm *ti;
    struct timeval tp;

    gettimeofday(&tp, 0);
    ti = localtime(&tp.tv_sec);
    strftime(msg, 20, "%F %T", ti);
    p = msg + 19;
    s = WSGI_MAX_MSG - 19;

    n = snprintf(p, s, ",%d %s %s: ",
                 (u_int)(tp.tv_usec / 1000),
                 log_level[level],
                 log_source[source]);

    s -= n + 1; p += n;
    va_start(args, fmt);
    n = vsnprintf(p, s, fmt, args);
    va_end(args);

    n = n >= s ? s - 1 : n;
    p += n; *p++ = '\n'; *p = '\0';

    log->handler(log, msg, p - msg);
}


void wsgi_printf_log_handler(const wsgi_log_t *log, const char *msg,
                             size_t size)
{
    printf(msg);
}


void wsgi_log_set_source(u_int source, const char *name)
{
    assert(source > 0 && source < WSGI_LOG_MAX_SOURCE);
    log_source[source] = name;
}
