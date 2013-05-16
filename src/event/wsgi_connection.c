
#include <wsgi_event.h>


void wsgi_connection_init(wsgi_connection_t *c, wsgi_gc_t *gc)
{
    c->log = gc->log;
    c->gc = gc;

    wsgi_socket_init(&c->socket, c->log);
}


void wsgi_connection_reset(wsgi_connection_t *c)
{
    wsgi_gc_reset(c->gc);
    wsgi_socket_init(&c->socket, c->log);
}


int wsgi_connection_get_handle(void *self)
{
    return ((wsgi_connection_t *) self)->socket.fd;
}


int wsgi_connection_close(wsgi_connection_t *c)
{
    if (c->socket.fd == -1) {
        return WSGI_ERROR;
    }

    if (wsgi_reactor_unregister(c->acceptor->reactor,
                                &c->event_handler) != WSGI_OK) {
        return WSGI_ERROR;
    }

    if (wsgi_socket_close(&c->socket) != WSGI_OK) {
        return WSGI_ERROR;
    }

    if (wsgi_pool_get_back(c->acceptor->connection_pool, c) != WSGI_OK) {
        return WSGI_ERROR;
    }

    return WSGI_OK;
}
