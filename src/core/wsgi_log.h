#ifndef _WSGI_LOG_H_INCLUDED_
#define _WSGI_LOG_H_INCLUDED_


#include <wsgi_config.h>


#define WSGI_LOG_EMERG          0
#define WSGI_LOG_ERR            1
#define WSGI_LOG_INFO           2
#define WSGI_LOG_DEBUG          3

#define WSGI_LOG_SOURCE_ALL     0x0F
#define WSGI_LOG_MAX_SOURCE     15
#define WSGI_MAX_MSG            128


typedef void (*wsgi_log_handler_pt) (wsgi_log_t *log, char *msg, size_t size);
struct wsgi_log_s {
    uint_t                  log_level;
    uint_t                  log_source;
    wsgi_log_handler_pt     handler;
};


wsgi_log_t* wsgi_log_init();
void wsgi_log_msg(wsgi_log_t* log, uint_t level, uint_t source,
                  const char *fmt, ...);
void wsgi_log_set_source(uint_t source, const char *name);


#define wsgi_log_emerg(log, ...) \
    wsgi_log_msg(log, WSGI_LOG_EMERG, __VA_ARGS__)

#define wsgi_log_error(log, source, ...) \
    if (log->log_level >= WSGI_LOG_ERR && log->log_source & source) \
        wsgi_log_msg(log, WSGI_LOG_ERR, source, __VA_ARGS__)

#define wsgi_log_info(log, source, ...) \
    if (log->log_level >= WSGI_LOG_INFO && log->log_source & source) \
        wsgi_log_msg(log, WSGI_LOG_INFO, source, __VA_ARGS__)

#if (WSGI_DEBUG)

#define wsgi_log_debug(log, source, ...) \
    if (log->log_level == WSGI_LOG_DEBUG && log->log_source & source) \
        wsgi_log_msg(log, WSGI_LOG_DEBUG, source, __VA_ARGS__)

#else

#define wsgi_log_debug(log, ...)

#endif /* WSGI_DEBUG */

#endif /* _WSGI_LOG_H_INCLUDED_ */
