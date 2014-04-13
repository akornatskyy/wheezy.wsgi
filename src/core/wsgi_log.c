
#include <wsgi_core.h>

#if WSGI_DEBUG

#define TIMESTAMP_FMT "\x1B[30;1m%F %T"
#define TIMESTAMP_LEN 27
#define MSG_FMT ",%03d\x1B[0m %s\x1B[0m \x1B[35;1m%s\x1B[0m: "

static const char *log_level[] = {
    "\x1B[31m[EMERG]",
    "\x1B[31;1m[ERROR]",
    "\x1B[33;1m[WARN]",
    "\x1B[32;1m[INFO]",
    "\x1B[36m[DEBUG]"
};

#else

#define TIMESTAMP_FMT "%F %T"
#define TIMESTAMP_LEN 20
#define MSG_FMT ",%03d %s %s: "

static const char *log_level[] = {
    "[EMERG]",
    "[ERROR]",
    "[WARN]",
    "[INFO]",
    "[DEBUG]"
};

#endif


static const char *log_source[WSGI_LOG_MAX_SOURCE + 1];

static wsgi_log_t g_log;


static void wsgi_printf_log_handler(const wsgi_log_t *log,
                                    const char *msg, size_t size);


wsgi_log_t *
wsgi_log_init(void)
{
    u_int i;
    for(i = 1; i < WSGI_LOG_MAX_SOURCE; i++) {
        log_source[i] = "undefined";
    }

#if WSGI_DEBUG
    g_log.log_level = WSGI_LOG_DEBUG;
#else
    g_log.log_level = WSGI_LOG_INFO;
#endif
    g_log.log_source = WSGI_LOG_SOURCE_ALL;
    g_log.handler = wsgi_printf_log_handler;
    return &g_log;
}


void
wsgi_log_msg(const wsgi_log_t *log, u_int level, u_int source,
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
    strftime(msg, TIMESTAMP_LEN, TIMESTAMP_FMT, ti);
    p = msg + TIMESTAMP_LEN - 1;
    s = WSGI_MAX_MSG - TIMESTAMP_LEN + 1;

    n = snprintf(p, s, MSG_FMT,
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


void
wsgi_printf_log_handler(const wsgi_log_t *log, const char *msg, size_t size)
{
    fputs(msg, stdout);
}


void
wsgi_log_set_source(u_int source, const char *name)
{
    assert(source > 0 && source < WSGI_LOG_MAX_SOURCE);
    log_source[source] = name;
}
