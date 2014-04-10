#ifndef _WSGI_CONNECTION_H_INCLUDED_
#define _WSGI_CONNECTION_H_INCLUDED_


struct wsgi_connection_s {
    wsgi_gc_t               *gc;
    wsgi_acceptor_t         *acceptor;
    wsgi_event_handler_t    event_handler;
    wsgi_socket_t           socket;
    void                    *request;
};


void wsgi_connection_init(wsgi_connection_t *c, wsgi_gc_t *gc,
                          int (handle_read)(void *self));
void wsgi_connection_close(wsgi_connection_t *c);

#endif /* _WSGI_CONNECTION_H_INCLUDED_ */
