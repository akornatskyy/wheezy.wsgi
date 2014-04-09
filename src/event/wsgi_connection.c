
#include <wsgi_event.h>


static int wsgi_connection_get_handle(void *self);


void
wsgi_connection_init(wsgi_connection_t *c, wsgi_gc_t *gc)
{
    c->gc = gc;
    c->event_handler.get_handle = wsgi_connection_get_handle;
    wsgi_socket_init(&c->socket, gc->log);
}


int
wsgi_connection_close(wsgi_connection_t *c)
{
    if (c->socket.fd == -1) {
        return WSGI_OK;
    }

    wsgi_log_debug(c->gc->log, WSGI_LOG_SOURCE_EVENT,
                   "closing connection: %p, fd: %d",
                   c, c->socket.fd);

    wsgi_gc_reset(c->gc);

    if (wsgi_reactor_unregister(c->acceptor->reactor,
                                &c->event_handler) != WSGI_OK) {
        return WSGI_ERROR;
    }

    if (wsgi_socket_close(&c->socket) != WSGI_OK) {
        return WSGI_ERROR;
    }

    if (wsgi_pool_get_back(c->acceptor->pool, c) != WSGI_OK) {
        return WSGI_ERROR;
    }

    return WSGI_OK;
}


static int
wsgi_connection_get_handle(void *self)
{
    return ((wsgi_connection_t *) self)->socket.fd;
}
