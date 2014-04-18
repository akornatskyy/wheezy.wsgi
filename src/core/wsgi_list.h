
#ifndef WSGI_LIST_H
#define WSGI_LIST_H


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

#define wsgi_list_last_item(l)                                                \
    (void *) ((u_char *) (l)->items + (l)->size * ((l)->length - 1))

#endif /* WSGI_LIST_H */
