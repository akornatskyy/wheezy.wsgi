
#include <wsgi_core.h>

// 32bit - unix: 2, tcp4: 16, tcp6: 28
#define WSGI_ACCEPT_SOCKLEN 28


wsgi_socket_t *
wsgi_socket_create(wsgi_gc_t *gc)
{
    wsgi_socket_t *s;

    s = wsgi_gc_malloc(gc, sizeof(wsgi_socket_t));
    if (s == NULL) {
        return NULL;
    }

    wsgi_socket_init(s, gc->log);

    return s;
}


void
wsgi_socket_init(wsgi_socket_t *s, const wsgi_log_t *log)
{
    s->log = log;
    s->addr = NULL;
    s->fd = -1;
}


int
wsgi_socket_open(wsgi_socket_t *s, wsgi_addr_t *a)
{
    int fd, reuseaddr;

    reuseaddr = 1;

    wsgi_log_debug(s->log, WSGI_LOG_SOURCE_SOCKET,
                   "opening socket: %s",
                   a->name);

    fd = socket(a->sa->sa_family, SOCK_STREAM, 0);
    if (fd == -1) {
        wsgi_log_error(s->log, WSGI_LOG_SOURCE_SOCKET,
                       "socket: %s, errno %d: %s",
                       a->name, errno, strerror(errno));
        return WSGI_ERROR;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                   sizeof(reuseaddr)) == -1) {
        close(fd);
        wsgi_log_error(s->log, WSGI_LOG_SOURCE_SOCKET,
                       "reuseraddr: %s, errno %d: %s",
                       a->name, errno, strerror(errno));
        return WSGI_ERROR;
    }

    if (bind(fd, a->sa, a->length) == -1) {
        close(fd);
        wsgi_log_error(s->log, WSGI_LOG_SOURCE_SOCKET,
                       "bind: %s, errno %d: %s",
                       a->name, errno, strerror(errno));
        return WSGI_ERROR;
    }

    if (listen(fd, 100 /* TODO */) == -1) {
        close(fd);
        wsgi_log_error(s->log, WSGI_LOG_SOURCE_SOCKET,
                       "listen: %s, errno %d: %s",
                       a->name, errno, strerror(errno));
        return WSGI_ERROR;
    }

    s->fd = fd;
    s->addr = a;

    wsgi_log_info(s->log, WSGI_LOG_SOURCE_SOCKET,
                  "listening on: %s, fd: %d",
                  a->name, s->fd);

    return WSGI_OK;
}


int
wsgi_socket_accept(wsgi_socket_t *s, wsgi_socket_t *peer, wsgi_gc_t *gc)
{
    int fd;
    socklen_t length = WSGI_ACCEPT_SOCKLEN;
    u_char sa[WSGI_ACCEPT_SOCKLEN];

    memset(sa, 0, WSGI_ACCEPT_SOCKLEN);

    fd = accept(s->fd, (struct sockaddr *) sa, &length);
    if (fd == -1) {
        wsgi_log_error(s->log, WSGI_LOG_SOURCE_SOCKET,
                       "accept: %s, errno %d: %s",
                       s->addr->name, errno, strerror(errno));
        return WSGI_ERROR;
    }

    wsgi_log_debug(s->log, WSGI_LOG_SOURCE_SOCKET,
                   "accepted on: %s, fd: %d",
                   s->addr->name, fd);

    peer->addr = wsgi_gc_malloc(gc, sizeof(wsgi_addr_t));
    peer->addr->sa = wsgi_gc_malloc(gc, length);

    peer->fd = fd;
    peer->addr->name = NULL;
    peer->addr->length = length;
    memcpy(peer->addr->sa, sa, length);

    return WSGI_OK;
}


int
wsgi_socket_close(wsgi_socket_t *s)
{
    int fd;

    fd = s->fd;

    if (fd != -1) {
        wsgi_log_debug(s->log, WSGI_LOG_SOURCE_SOCKET,
                       "closing socket, fd: %d",
                       fd);

        s->fd = -1;

        if (close(fd) == -1) {
            wsgi_log_error(s->log, WSGI_LOG_SOURCE_SOCKET,
                           "socket close, fd: %s, errno %d: %s",
                           fd, errno, strerror(errno));
            return WSGI_ERROR;
        }
    }

    return WSGI_OK;
}
