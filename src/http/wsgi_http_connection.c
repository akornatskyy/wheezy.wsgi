
#include <wsgi_http.h>


int
wsgi_http_connection_open(wsgi_connection_t *c)
{
    wsgi_log_debug(c->gc->log, WSGI_LOG_SOURCE_HTTP,
                   "opening connection: %p, fd: %d",
                   c, c->socket.fd);

    c->request = wsgi_http_request_create(c);
    if (c->request == NULL) {
        wsgi_connection_close(c);
        return WSGI_ERROR;
    }

    return wsgi_http_connection_handle_read(c);
}


int
wsgi_http_connection_handle_read(void *self)
{
    ssize_t n, size;
    wsgi_connection_t *c;
    wsgi_http_request_t *r;

    c = self;
    r = c->request;

    wsgi_log_debug(c->gc->log, WSGI_LOG_SOURCE_HTTP,
                   "connection: %p, handling request: %p",
                   c, r);

    for (;;) {
        size = r->buffer_end - r->buffer_last;
        n = recv(c->socket.fd, r->buffer_last, size, 0 /* flags */);

        wsgi_log_debug(c->gc->log, WSGI_LOG_SOURCE_HTTP,
                       "recv, fd: %d, %d of %d",
                       c->socket.fd, n, size);

        // The return value will be 0 when the peer has performed an
        // orderly shutdown.
        if (n == 0) {
            wsgi_log_debug(c->gc->log, WSGI_LOG_SOURCE_HTTP,
                           "connection %p closed by peer, fd: %d",
                           c, c->socket.fd);
            wsgi_connection_close(c);
            return WSGI_ERROR;
        }

        if (n == -1) {
            wsgi_log_error(c->gc->log, WSGI_LOG_SOURCE_HTTP,
                           "recv, fd: %d, errno %d: %s",
                           c, c->socket.fd, errno, strerror(errno));
            wsgi_connection_close(c);
            return WSGI_ERROR;
        }

        r->buffer_last += n;

        if (r->handle_read(r) != WSGI_OK) {
            wsgi_connection_close(c);
            return WSGI_ERROR;
        }

        if (n < size) break;
    }

    return WSGI_OK;
}
