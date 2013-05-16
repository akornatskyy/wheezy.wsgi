
#ifndef _WSGI_POOL_H_INCLUDED_
#define _WSGI_POOL_H_INCLUDED_


struct wsgi_pool_s {
    const wsgi_log_t    *log;
    void                *items;
    u_int               capacity;
    void                **free;
    u_int               free_count;
};


int wsgi_pool_init(wsgi_pool_t *p, wsgi_gc_t *gc, u_int capacity,
                   size_t size);
void *wsgi_pool_acquire(wsgi_pool_t *p);
int wsgi_pool_get_back(wsgi_pool_t *p, void *item);


#endif /* _WSGI_POOL_H_INCLUDED_ */
