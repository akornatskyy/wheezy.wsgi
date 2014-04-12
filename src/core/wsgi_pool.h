
#ifndef WSGI_POOL_H
#define WSGI_POOL_H


struct wsgi_pool_s {
    const wsgi_log_t    *log;
    void                *items;
    u_int               capacity;
    void                **free;
    u_int               free_count;
};


wsgi_pool_t *wsgi_pool_create(wsgi_gc_t *gc, u_int capacity,
                              size_t size);
int wsgi_pool_init(wsgi_pool_t *p, wsgi_gc_t *gc, u_int capacity,
                   size_t size);
void *wsgi_pool_acquire(wsgi_pool_t *p);
int wsgi_pool_get_back(wsgi_pool_t *p, void *item);


#endif /* WSGI_POOL_H */
