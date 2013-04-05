
#ifndef _WSGI_LIST_H_INCLUDED_
#define _WSGI_LIST_H_INCLUDED_


struct wsgi_list_s {
    void            *items;
    size_t          size;
    u_int           length;
    u_int           capacity;
    wsgi_gc_t       *gc;
};


wsgi_list_t *wsgi_list_create(wsgi_gc_t *gc, u_int capacity, size_t size);

int wsgi_list_init(wsgi_list_t *list, wsgi_gc_t *gc, u_int capacity,
                   size_t size);

void *wsgi_list_append(wsgi_list_t *list);

#endif /* _WSGI_LIST_H_INCLUDED_ */
