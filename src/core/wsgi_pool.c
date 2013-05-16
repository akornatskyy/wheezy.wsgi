
#include <wsgi_core.h>


int wsgi_pool_init(wsgi_pool_t *p, wsgi_gc_t *gc, u_int capacity,
                   size_t size)
{
    u_int i;
    void *t;

    t = wsgi_gc_malloc(gc, capacity * size);
    if (t == NULL) {
        return WSGI_ERROR;
    }

    p->free = wsgi_gc_malloc(gc, capacity * sizeof(void *));
    if (p->free == NULL) {
        return WSGI_ERROR;
    }

    p->log = gc->log;
    p->capacity = p->free_count = capacity;
    p->items = t;

    for (i = 0; i < capacity; i++) {
        p->free[i] = t;
        t += size;
    }

    wsgi_log_debug(gc->log, WSGI_LOG_SOURCE_POOL,
                   "init: %p, capacity: %d, size: %d",
                   p, capacity, size);

    return WSGI_OK;
}


void *wsgi_pool_acquire(wsgi_pool_t *p)
{
    void *t;

    if (p->free_count == 0) {
        wsgi_log_error(p->log, WSGI_LOG_SOURCE_POOL,
                       "exhausted pool: %p",
                       p);
        return NULL;
    }

    t = p->free[--p->free_count];

    wsgi_log_debug(p->log, WSGI_LOG_SOURCE_POOL,
                   "%p = acquire: %p, free: %d",
                   t, p, p->free_count);

    return t;
}


int wsgi_pool_get_back(wsgi_pool_t *p, void *item)
{
    if (p->free_count == p->capacity) {
        wsgi_log_error(p->log, WSGI_LOG_SOURCE_POOL,
                       "get_back: %p, excessive item: %p",
                       p, item);
        return WSGI_ERROR;
    }

    p->free[p->free_count++] = item;

    wsgi_log_debug(p->log, WSGI_LOG_SOURCE_POOL,
                   "get_back: %p, item: %p, free: %d",
                   p, item, p->free_count);

    return WSGI_OK;
}
