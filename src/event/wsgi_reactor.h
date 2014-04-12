#ifndef WSGI_REACTOR_H
#define WSGI_REACTOR_H


struct wsgi_event_loop_s {
    void        *self;
    int         (*add)(void *self, wsgi_event_handler_t *h);
    int         (*del)(void *self, wsgi_event_handler_t *h);
    int         (*wait)(void *self, int timeout);
    int         (*close)(void *self);
};

struct wsgi_event_handler_s {
    void        *self;
    int         (*get_handle)(void *self);
    int         (*handle_event)(void *self);
};


wsgi_reactor_t *wsgi_reactor_create(wsgi_gc_t *gc, wsgi_event_loop_t *l);
int wsgi_reactor_register(wsgi_reactor_t *r, wsgi_event_handler_t *h);
int wsgi_reactor_unregister(wsgi_reactor_t *r, wsgi_event_handler_t *h);
int wsgi_reactor_wait_for_events(wsgi_reactor_t *r);
int wsgi_reactor_destroy(wsgi_reactor_t *r);

#endif /* WSGI_REACTOR_H */
