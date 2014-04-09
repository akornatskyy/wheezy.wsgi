#ifndef _WSGI_ACCEPTOR_H_INCLUDED_
#define _WSGI_ACCEPTOR_H_INCLUDED_

#include <wsgi_addr.h>
#include <wsgi_reactor.h>
#include <wsgi_socket.h>


struct wsgi_acceptor_s {
    const wsgi_log_t                *log;
    wsgi_event_handler_t            event_handler;
    wsgi_reactor_t                  *reactor;
    wsgi_socket_t                   socket;
    wsgi_pool_t                     *pool;
    int                             (*handle_open)(wsgi_connection_t *c);
    const void                      *config;
};


wsgi_acceptor_t *
wsgi_acceptor_create(wsgi_gc_t *gc, wsgi_reactor_t *r, wsgi_pool_t *p,
                     int (*handle_open)(wsgi_connection_t *c),
                     void *config);
int wsgi_acceptor_open(wsgi_acceptor_t *a, wsgi_addr_t *addr);
int wsgi_acceptor_close(wsgi_acceptor_t *a);
int wsgi_acceptor_get_handle(void *instance);
int wsgi_acceptor_handle_event(void *instance);

#endif /* _WSGI_ACCEPTOR_H_INCLUDED_ */
