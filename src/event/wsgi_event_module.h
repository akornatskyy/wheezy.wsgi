#ifndef _WSGI_EVENT_MODULE_H_INCLUDED_
#define _WSGI_EVENT_MODULE_H_INCLUDED_


struct wsgi_event_ctx_s {
    wsgi_gc_t           *gc;
    uint                events;
    wsgi_reactor_t      *reactor;
};

#endif /* _WSGI_EVENT_MODULE_H_INCLUDED_ */
