
#include <sys/epoll.h>
#include <wsgi_event.h>
#include <wsgi_epoll.h>


static int wsgi_epoll_add(void *self, wsgi_event_handler_t *h);
static int wsgi_epoll_del(void *self, wsgi_event_handler_t *h);
static int wsgi_epoll_wait(void *self, int timeout);
static int wsgi_epoll_close(void *self);


typedef struct wsgi_epoll_s {
    const wsgi_log_t        *log;
    wsgi_event_loop_t       loop;
    int                     fd;
    struct epoll_event      *events;
    int                     events_length;
} wsgi_epoll_t;


wsgi_event_loop_t *
wsgi_epoll_create(wsgi_gc_t *gc, u_int length)
{
    wsgi_epoll_t *e;
    wsgi_event_loop_t *l;

    wsgi_log_info(gc->log, WSGI_LOG_SOURCE_EVENT,
                   "using epoll(%d)",
                   length);

    e = wsgi_gc_malloc(gc, sizeof(wsgi_epoll_t));
    if (e == NULL) {
        return NULL;
    }

    e->log = gc->log;
    e->fd = epoll_create(1);
    if (e->fd == -1) {
        wsgi_log_error(e->log, WSGI_LOG_SOURCE_EVENT,
                       "epoll create: errno %d: %s",
                       errno, strerror(errno));
        return NULL;
    }

    e->events_length = length;
    e->events = wsgi_gc_malloc(gc, length * sizeof(struct epoll_event));
    if (e->events == NULL) {
        return NULL;
    }

    l = &e->loop;
    l->self = e;
    l->add = wsgi_epoll_add;
    l->del = wsgi_epoll_del;
    l->wait = wsgi_epoll_wait;
    l->close = wsgi_epoll_close;

    return l;
}


static int
wsgi_epoll_add(void *self, wsgi_event_handler_t *h)
{
    wsgi_epoll_t *e;
    struct epoll_event ev;
    int fd;

    e = self;
    fd = h->get_handle(h->self);

    wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                   "epoll add: %p, fd: %d",
                   h->self, fd);

    memset(&ev, 0, sizeof(struct epoll_event));
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = h;

    if (epoll_ctl(e->fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        wsgi_log_error(e->log, WSGI_LOG_SOURCE_EVENT,
                       "epoll add: errno %d: %s",
                       errno, strerror(errno));
        return WSGI_ERROR;
    }

    return WSGI_OK;
}


static int
wsgi_epoll_del(void *self, wsgi_event_handler_t *h)
{
    wsgi_epoll_t *e;
    struct epoll_event ev;
    int fd;

    e = self;
    fd = h->get_handle(h->self);

    wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                   "epoll del: %p, fd: %d",
                   h->self, fd);

    ev.events = 0;
    ev.data.ptr = NULL;

    if (epoll_ctl(e->fd, EPOLL_CTL_DEL, fd, &ev) == -1) {
        wsgi_log_error(e->log, WSGI_LOG_SOURCE_EVENT,
                       "epoll del: errno %d: %s",
                       errno, strerror(errno));
        return WSGI_ERROR;
    }

    return WSGI_OK;
}


static int
wsgi_epoll_wait(void *self, int timeout)
{
    int ready;
    wsgi_epoll_t *e;
    struct epoll_event *ev;
    uint32_t events;
    wsgi_event_handler_t *h;

    e = self;

    wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                   "epoll wait timeout: %d",
                   timeout);

    ready = epoll_wait(e->fd, e->events, e->events_length, timeout);
    if (ready == -1) {
        wsgi_log_error(e->log, WSGI_LOG_SOURCE_EVENT,
                       "epoll wait: errno %d: %s",
                       errno, strerror(errno));
        return WSGI_ERROR;
    }

    if (ready == 0) {
        wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                       "epoll wait timed out");
    }
    else {
        wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                       "epoll processing %d event(s)",
                       ready);
    }

    for (ev = e->events; ready--; ev++) {
        h = ev->data.ptr;
        events = ev->events;

        wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                       "epoll processing for %p, events: 0x%02X",
                       h->self, events);

        if (events & (EPOLLERR|EPOLLHUP)) {
            wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                           "epoll wait error for %p, fd: %d",
                           h->self, h->get_handle(h->self));
        }

        h->handle_event(h->self);
    }

    return WSGI_OK;
}


static int
wsgi_epoll_close(void *self)
{
    wsgi_epoll_t *e;
    int fd;

    e = self;
    fd = e->fd;
    e->fd = -1;

    wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                   "epoll closing");

    if (close(fd) == -1) {
        wsgi_log_error(e->log, WSGI_LOG_SOURCE_EVENT,
                       "epoll close: %d, errno %d: %s",
                       fd, errno, strerror(errno));
        return WSGI_ERROR;
    }

    return WSGI_OK;
}
