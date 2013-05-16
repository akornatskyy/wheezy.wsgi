
#include <wsgi_event.h>


struct wsgi_reactor_s {
    const wsgi_log_t            *log;
    wsgi_event_loop_t           *event_loop;
    int                         active;
};


wsgi_reactor_t *wsgi_reactor_create(wsgi_gc_t *gc, wsgi_event_loop_t *l)
{
    wsgi_reactor_t *r;

    r = wsgi_gc_malloc(gc, sizeof(wsgi_reactor_t));

    r->log = gc->log;
    r->event_loop = l;
    r->active = 0;

    return r;
}


int wsgi_reactor_register(wsgi_reactor_t *r, wsgi_event_handler_t *h)
{
    return r->event_loop->add(r->event_loop->self, h);
}


int wsgi_reactor_unregister(wsgi_reactor_t *r, wsgi_event_handler_t *h)
{
    return r->event_loop->del(r->event_loop->self, h);
}


int wsgi_reactor_wait_for_events(wsgi_reactor_t *r)
{
    u_int i;

    r->active = 1;
    for (i = 0; i < 3; i++) {
        if (r->event_loop->wait(r->event_loop->self, -1) != WSGI_OK) {
            return WSGI_ERROR;
        }
    }

    return WSGI_OK;
}


int wsgi_reactor_destroy(wsgi_reactor_t *r)
{
    r->active = 0;
    r->event_loop->close(r->event_loop->self);
    r->event_loop = NULL;

    return WSGI_OK;
}
