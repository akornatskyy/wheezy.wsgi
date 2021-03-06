#ifndef WSGI_GC_H
#define WSGI_GC_H


typedef struct wsgi_gc_block_s wsgi_gc_block_t;
typedef struct wsgi_gc_ref_s wsgi_gc_ref_t;

struct wsgi_gc_block_s {
    u_char              *current;
    size_t              left;
    u_int               fails;
    wsgi_gc_block_t     *next;
};

struct wsgi_gc_ref_s {
    u_char              *ref1;
    u_char              *ref2;
    u_char              *ref3;
    wsgi_gc_ref_t       *next;
};

struct wsgi_gc_s {
    wsgi_gc_block_t     b;
    size_t              block_size;
    wsgi_gc_block_t     *current;
    const wsgi_log_t    *log;
    wsgi_gc_ref_t       *ref;
    size_t              ref_size;
};


wsgi_gc_t *wsgi_gc_create(size_t size, const wsgi_log_t *log);

void wsgi_gc_destroy(wsgi_gc_t *gc);

void wsgi_gc_reset(wsgi_gc_t *gc);

void *wsgi_gc_malloc(wsgi_gc_t *h, size_t size);

void *wsgi_gc_calloc(wsgi_gc_t *gc, size_t size);

void *wsgi_gc_malloc_ref(wsgi_gc_t *gc, size_t size);

#endif /* WSGI_GC_H */
