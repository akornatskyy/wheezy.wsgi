#ifndef WSGI_LOG_H
#define WSGI_LOG_H


#define WSGI_LOG_EMERG          0
#define WSGI_LOG_ERROR          1
#define WSGI_LOG_WARN           2
#define WSGI_LOG_INFO           3
#define WSGI_LOG_DEBUG          4

#define WSGI_LOG_SOURCE_ALL     0xFFFFF
#define WSGI_LOG_MAX_SOURCE     WSGI_LOG_SOURCE_ALL
#define WSGI_MAX_MSG            156


typedef void (*wsgi_log_handler_pt) (const wsgi_log_t *log,
                                     const char *msg, size_t size);
struct wsgi_log_s {
    u_int                   log_level;
    u_int                   log_source;
    wsgi_log_handler_pt     handler;
};


wsgi_log_t *wsgi_log_init(void);
void wsgi_log_msg(const wsgi_log_t *log, u_int level, u_int source,
                  const char *fmt, ...);
void wsgi_log_set_source(u_int source, const char *name);


#define wsgi_log_emerg(log, ...) \
    wsgi_log_msg(log, WSGI_LOG_EMERG, __VA_ARGS__)

#define wsgi_log_error(log, source, ...) \
    if (log->log_level >= WSGI_LOG_ERROR && log->log_source & source) \
        wsgi_log_msg(log, WSGI_LOG_ERROR, source, __VA_ARGS__)

#define wsgi_log_warn(log, source, ...) \
    if (log->log_level >= WSGI_LOG_WARN && log->log_source & source) \
        wsgi_log_msg(log, WSGI_LOG_WARN, source, __VA_ARGS__)

#define wsgi_log_info(log, source, ...) \
    if (log->log_level >= WSGI_LOG_INFO && log->log_source & source) \
        wsgi_log_msg(log, WSGI_LOG_INFO, source, __VA_ARGS__)

#if WSGI_DEBUG

#define wsgi_log_debug(log, source, ...) \
    if (log->log_level == WSGI_LOG_DEBUG && log->log_source & source) \
        wsgi_log_msg(log, WSGI_LOG_DEBUG, source, __VA_ARGS__)

#else

#define wsgi_log_debug(log, ...)

#endif /* WSGI_DEBUG */

#endif /* WSGI_LOG_H */
