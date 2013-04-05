#ifndef _WSGI_GC_H_INCLUDED_
#define _WSGI_GC_H_INCLUDED_


typedef struct wsgi_gc_block_s wsgi_gc_block_t;

struct wsgi_gc_block_s {
    u_char              *current;
    size_t              left;
    u_int               fails;
    wsgi_gc_block_t     *next;
};

struct wsgi_gc_s {
    wsgi_gc_block_t     b;
    size_t              block_size;
    wsgi_gc_block_t     *current;
    const wsgi_log_t    *log;
};


wsgi_gc_t *wsgi_gc_create(size_t size, const wsgi_log_t *log);

void wsgi_gc_destroy(wsgi_gc_t *gc);

void wsgi_gc_reset(wsgi_gc_t *gc);

void *wsgi_gc_malloc(wsgi_gc_t *h, size_t size);

#endif /* _WSGI_GC_H_INCLUDED_ */
