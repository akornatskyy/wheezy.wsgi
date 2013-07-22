
#include <wsgi_http.h>


static int wsgi_http_connection_handle_read(void *self);


int
wsgi_http_connection_open(wsgi_connection_t *c)
{
    wsgi_event_handler_t *h;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_HTTP,
                   "opening connection: %p, fd: %d",
                   c, c->socket.fd);

    h = &c->event_handler;
    h->self = c;
    h->get_handle = wsgi_connection_get_handle;
    h->handle_event = wsgi_http_connection_handle_read;

    if (wsgi_reactor_register(c->acceptor->reactor, h) != WSGI_OK) {
        return WSGI_ERROR;
    }

    return WSGI_OK;
}


int
wsgi_http_connection_close(wsgi_connection_t *c)
{
    if (c->socket.fd == -1) {
        return WSGI_OK;
    }

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_HTTP,
                   "closing connection: %p, fd: %d",
                   c, c->socket.fd);

    return wsgi_connection_close(c);
}


int
wsgi_http_connection_pool_init(wsgi_pool_t *p, wsgi_log_t *log)
{
    u_int i;
    wsgi_gc_t *gc;
    wsgi_connection_t *c;

    wsgi_log_info(log, WSGI_LOG_SOURCE_HTTP,
                  "connections: %d",
                  p->capacity);

    for (i = 0, c = p->items; i < p->capacity; i++, c++) {
        gc = wsgi_gc_create(WSGI_CONNECTION_GC_SIZE, log);
        if (gc == NULL) {
            return WSGI_ERROR;
        }

        wsgi_connection_init(c, gc);
    }

    return WSGI_OK;
}


int
wsgi_http_connection_pool_close(wsgi_pool_t *p)
{
    u_int i;
    wsgi_connection_t *c;

    for (i = 0, c = p->items; i < p->capacity; i++, c++) {
        if (wsgi_http_connection_close(c) != WSGI_OK) {
            return WSGI_ERROR;
        }

        wsgi_gc_destroy(c->gc);
    }

    return WSGI_OK;
}


static int
wsgi_http_connection_handle_read(void *self)
{
    ssize_t n;
    size_t size;
    u_char buf[512];
    wsgi_connection_t *c;

    c = self;
    size = 512;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_HTTP,
                   "handling request for connection: %p",
                   c);

    n = recv(c->socket.fd, buf, size, 0);

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_HTTP,
                   "recv, fd: %d, %d of %d",
                   c->socket.fd, n, size);

    // The return value will be 0 when the peer has performed an
    // orderly shutdown.
    if (n == 0) {
        wsgi_log_debug(c->log, WSGI_LOG_SOURCE_HTTP,
                       "connection %p closed by peer, fd: %d",
                       c, c->socket.fd);
        return wsgi_http_connection_close(c);
    }

    return WSGI_OK;
}
