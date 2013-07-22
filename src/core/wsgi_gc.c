
#include <wsgi_core.h>


static void *wsgi_gc_alloc_block(wsgi_gc_t *h, size_t size);


wsgi_gc_t *
wsgi_gc_create(size_t size, const wsgi_log_t *log)
{
    wsgi_gc_t *gc;

    gc = wsgi_malloc(size, log);
    if (gc == NULL) {
        return NULL;
    }

    gc->b.current = (u_char *) gc + sizeof(wsgi_gc_t);
    gc->b.left = size - sizeof(wsgi_gc_t);
    gc->b.fails = 0;
    gc->b.next = NULL;

    gc->block_size = size - sizeof(wsgi_gc_block_t);
    gc->log = log;
    gc->current = (wsgi_gc_block_t *) gc;

    wsgi_log_debug(log, WSGI_LOG_SOURCE_GC,
                   "create: %p, left: %d",
                   gc, gc->b.left);
    return gc;
}


void
wsgi_gc_destroy(wsgi_gc_t *gc)
{
    wsgi_gc_block_t *b, *n;
#if WSGI_DEBUG
    const wsgi_log_t *log;
    log = gc->log;
#endif

    for (b = &gc->b, n = b->next; ; b = n, n = n->next) {
        wsgi_log_debug(log, WSGI_LOG_SOURCE_GC,
                       "destroy: %p > %p, fails: %d, unused: %d",
                       gc, b, b->fails, b->left);

        wsgi_free(b);
        if (n == NULL) break;
    }
}


void
wsgi_gc_reset(wsgi_gc_t *gc)
{
    wsgi_gc_block_t *b;

    b = &gc->b;
    wsgi_log_debug(gc->log, WSGI_LOG_SOURCE_GC,
                   "reset: %p, fails: %d, freed: %d",
                   b, b->fails,
                   gc->block_size - b->left
                   - (sizeof(wsgi_gc_t) - sizeof(wsgi_gc_block_t)));

    b->current = (u_char *) b + sizeof(wsgi_gc_t);
    b->left = gc->block_size - sizeof(wsgi_gc_t) + sizeof(wsgi_gc_block_t);
    b->fails = 0;

    gc->current = b;
    for (b = b->next; b; b = b->next) {
        wsgi_log_debug(gc->log, WSGI_LOG_SOURCE_GC,
                       "reset: %p > %p, fails: %d, freed: %d",
                       gc, b, b->fails, gc->block_size - b->left);
        b->current = (u_char *) b + sizeof(wsgi_gc_block_t);
        b->left = gc->block_size;
        b->fails = 0;
    }
}


void *
wsgi_gc_malloc(wsgi_gc_t *gc, size_t size)
{
    u_char *p;
    wsgi_gc_block_t *b;

    if (size > gc->block_size) {
        wsgi_log_debug(gc->log, WSGI_LOG_SOURCE_GC,
                       "malloc: %p, refused: %d",
                       gc, size);
        return NULL;
    }

    for (b = gc->current; b; b = b->next) {
        if (b->left >= size) {
            p = b->current;
            wsgi_log_debug(gc->log, WSGI_LOG_SOURCE_GC,
                           "%p = malloc: %p > %p, size: %d",
                           p, gc, b, size);
            b->current = p + size;
            b->left -= size;
            return p;
        }

        wsgi_log_debug(gc->log, WSGI_LOG_SOURCE_GC,
                       "malloc: %p > %p, miss: %d, left: %d",
                       gc, b, size, b->left);
    }

    return wsgi_gc_alloc_block(gc, size);
}


void *
wsgi_gc_calloc(wsgi_gc_t *gc, size_t size)
{
    void *p;

    p = wsgi_gc_malloc(gc, size);
    if (p) {
        memset(p, 0, size);
    }

    return p;
}


static void *
wsgi_gc_alloc_block(wsgi_gc_t *gc, size_t size)
{
    u_char *p;
    wsgi_gc_block_t *b, *c, *l;

    b = wsgi_malloc(gc->block_size + sizeof(wsgi_gc_block_t), gc->log);
    if (b == NULL) {
        return NULL;
    }

    wsgi_log_debug(gc->log, WSGI_LOG_SOURCE_GC,
                   "block: %p > %p, size: %d",
                   gc, b, size);

    p = (u_char *) b + sizeof(wsgi_gc_block_t);

    b->current = p + size;
    b->left = gc->block_size - size;
    b->fails = 0;
    b->next = NULL;

    l = c = gc->current;
    while (1) {
        if (c->fails++ > 2) {
            c = l->next;
        }

        if (l->next == NULL) {
            break;
        }

        l = l->next;
    }

    l->next = b;
    gc->current = c ? c : b;

    return p;
}
