
#include <wsgi_event.h>


wsgi_acceptor_t *
wsgi_acceptor_create(wsgi_gc_t *gc, wsgi_reactor_t *r, wsgi_pool_t *p,
                     int (*handle_open)(wsgi_connection_t *c),
                     void *config)
{
    wsgi_acceptor_t *a;

    a = wsgi_gc_malloc(gc, sizeof(wsgi_acceptor_t));
    if (a == NULL) {
        return NULL;
    }

    wsgi_socket_init(&a->socket, gc->log);

    a->log = gc->log;
    a->reactor = r;
    a->pool = p;
    a->event_handler.self = a;
    a->event_handler.get_handle = wsgi_acceptor_get_handle;
    a->event_handler.handle_event = wsgi_acceptor_handle_event;
    a->handle_open = handle_open;
    a->config = config;

    return a;
}


int
wsgi_acceptor_open(wsgi_acceptor_t *a, wsgi_addr_t *addr)
{
    wsgi_log_debug(a->log, WSGI_LOG_SOURCE_ACCEPTOR,
                   "open: %p",
                   a);

    if (addr->sa->sa_family == AF_UNIX) {
        unlink(((struct sockaddr_un*)addr->sa)->sun_path);
    }

    if (wsgi_socket_open(&a->socket, addr) != WSGI_OK) {
        return WSGI_ERROR;
    }

    if (wsgi_reactor_register(a->reactor, &a->event_handler) != WSGI_OK) {
        return WSGI_ERROR;
    }

    return WSGI_OK;
}


int
wsgi_acceptor_close(wsgi_acceptor_t *a)
{
    wsgi_log_debug(a->log, WSGI_LOG_SOURCE_ACCEPTOR,
                   "closing: %p",
                   a);

    if (wsgi_reactor_unregister(a->reactor, &a->event_handler) != WSGI_OK) {
        return WSGI_ERROR;
    }

    if (wsgi_socket_close(&a->socket) != WSGI_OK) {
        return WSGI_ERROR;
    }

    if (a->socket.addr->sa->sa_family == AF_UNIX) {
        if (unlink(((struct sockaddr_un *) a->socket.addr->sa)->sun_path)
                == -1) {
            wsgi_log_error(a->log, WSGI_LOG_SOURCE_SOCKET,
                           "unlink: %s, errno %d: %s",
                           a->socket.addr->name, errno, strerror(errno));
            return WSGI_ERROR;
        }
    }

    return WSGI_OK;
}


int
wsgi_acceptor_get_handle(void *self)
{
    return ((wsgi_acceptor_t *) self)->socket.fd;
}


int
wsgi_acceptor_handle_event(void *self)
{
    int fd;
    wsgi_acceptor_t *a;
    wsgi_connection_t *c;

    a = self;

    c = wsgi_pool_acquire(a->pool);
    if (c == NULL) {
        wsgi_log_error(a->log, WSGI_LOG_SOURCE_ACCEPTOR,
                       "insufficient connections, dropping");

        fd = accept(a->socket.fd, NULL, NULL);
        if (fd == -1) {
            wsgi_log_error(a->log, WSGI_LOG_SOURCE_ACCEPTOR,
                           "drop errno %d: %s",
                           errno, strerror(errno));
            return WSGI_ERROR;
        }

        if (close(fd) == -1) {
            wsgi_log_error(a->log, WSGI_LOG_SOURCE_ACCEPTOR,
                           "failed to drop, errno %d, %s",
                           errno, strerror(errno));
        }

        return WSGI_ERROR;
    }

    wsgi_log_debug(a->log, WSGI_LOG_SOURCE_ACCEPTOR,
                   "handling event: %p, connection: %p",
                   a, c);

    if (wsgi_socket_accept(&a->socket, &c->socket, c->gc) != WSGI_OK) {

        wsgi_pool_get_back(a->pool, c);

        return WSGI_ERROR;
    }

    c->acceptor = a;
    return a->handle_open(c);
}
