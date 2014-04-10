
#include <wsgi_event.h>


static int wsgi_connection_get_handle(void *self);


void
wsgi_connection_init(wsgi_connection_t *c, wsgi_gc_t *gc,
                     int (handle_read)(void *self))
{
    c->gc = gc;
    c->event_handler.self = c;
    c->event_handler.get_handle = wsgi_connection_get_handle;
    c->event_handler.handle_event = handle_read;
    wsgi_socket_init(&c->socket, gc->log);
}


void
wsgi_connection_close(wsgi_connection_t *c)
{
    if (c->socket.fd == -1) {
        return;
    }

    wsgi_log_debug(c->gc->log, WSGI_LOG_SOURCE_EVENT,
                   "closing connection: %p, fd: %d",
                   c, c->socket.fd);

    wsgi_reactor_unregister(c->acceptor->reactor, &c->event_handler);
    wsgi_socket_close(&c->socket);
    wsgi_pool_get_back(c->acceptor->pool, c);
    wsgi_gc_reset(c->gc);
}


static int
wsgi_connection_get_handle(void *self)
{
    return ((wsgi_connection_t *) self)->socket.fd;
}
