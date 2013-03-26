
#include <wsgi_core.h>


void *wsgi_malloc(size_t size, const wsgi_log_t *log)
{
    void *p;

    p = malloc(size);
    if (p == NULL) {
        wsgi_log_emerg(log, WSGI_LOG_SOURCE_ALLOC,
                       "malloc(%d) failed", size);
    }

    wsgi_log_debug(log, WSGI_LOG_SOURCE_ALLOC, "%p = malloc(%d)", p, size);
    return p;
}
