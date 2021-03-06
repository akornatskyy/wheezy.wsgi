#ifndef WSGI_EVENT_MODULE_H
#define WSGI_EVENT_MODULE_H


typedef wsgi_event_loop_t *(wsgi_event_loop_create_pt)(
        wsgi_gc_t *gc, u_int length);


wsgi_reactor_t* wsgi_event_ctx_get_reactor(wsgi_event_ctx_t* ctx);
int wsgi_event_ctx_add_event_loop(wsgi_event_ctx_t* ctx, const char *name,
                                  wsgi_event_loop_create_pt create);

#endif /* WSGI_EVENT_MODULE_H */
