
#include <wsgi_event.h>

#if WSGI_HAVE_KQUEUE

#include <sys/event.h>

static wsgi_event_loop_t * wsgi_kqueue_create(wsgi_gc_t *gc, u_int length);
static void *wsgi_kqueue_module_create(wsgi_cycle_t *cycle);
static int wsgi_kqueue_add(void *self, wsgi_event_handler_t *h);
static int wsgi_kqueue_del(void *self, wsgi_event_handler_t *h);
static int wsgi_kqueue_wait(void *self, int timeout);
static int wsgi_kqueue_close(void *self);


typedef struct {
    const wsgi_log_t        *log;
    wsgi_event_loop_t       loop;
    int                     fd;
    struct kevent           *changes;
    struct kevent           *events;
    int                     index;
    int                     events_length;
} wsgi_kqueue_t;


extern wsgi_module_t event_module;

const wsgi_module_t kqueue_module = {
    "kqueue", -1,
    NULL,
    wsgi_kqueue_module_create,
    NULL,
    NULL
};


static wsgi_event_loop_t *
wsgi_kqueue_create(wsgi_gc_t *gc, u_int length)
{
    wsgi_kqueue_t *e;
    wsgi_event_loop_t *l;

    wsgi_log_info(gc->log, WSGI_LOG_SOURCE_EVENT,
                   "using kqueue(%d)",
                   length);

    e = wsgi_gc_malloc(gc, sizeof(wsgi_kqueue_t));
    if (e == NULL) {
        return NULL;
    }

    e->log = gc->log;
    e->fd = kqueue();
    if (e->fd == -1) {
        wsgi_log_error(e->log, WSGI_LOG_SOURCE_EVENT,
                       "kqueue create: errno %d: %s",
                       errno, strerror(errno));
        return NULL;
    }

    e->index = 0;
    e->events_length = length;
    e->changes = wsgi_gc_malloc(gc, length * sizeof(struct kevent));
    if (e->changes == NULL) {
        return NULL;
    }

    e->events = wsgi_gc_malloc(gc, length * sizeof(struct kevent));
    if (e->events == NULL) {
        return NULL;
    }

    l = &e->loop;
    l->self = e;
    l->add = wsgi_kqueue_add;
    l->del = wsgi_kqueue_del;
    l->wait = wsgi_kqueue_wait;
    l->close = wsgi_kqueue_close;

    return l;
}


static void *
wsgi_kqueue_module_create(wsgi_cycle_t *cycle)
{
    if (wsgi_event_ctx_add_event_loop(
            cycle->ctx[event_module.id],
            kqueue_module.name, wsgi_kqueue_create) != WSGI_OK) {
        return NULL;
    }

    return cycle;
}


static int
wsgi_kqueue_add(void *self, wsgi_event_handler_t *h)
{
    wsgi_kqueue_t *e;
    struct kevent *ev;
    int fd;

    e = self;
    fd = h->get_handle(h->self);

    wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                   "kqueue add: %p, fd: %d",
                   h->self, fd);

    ev = &e->changes[e->index++];
    ev->ident = fd;
    ev->filter = EVFILT_READ;
    ev->flags = EV_ADD|EV_ENABLE;
    ev->udata = h;

    return WSGI_OK;
}


static int
wsgi_kqueue_del(void *self, wsgi_event_handler_t *h)
{
    wsgi_kqueue_t *e;
    struct kevent *changes;
    uintptr_t fd;
    int i;

    e = self;
    fd = h->get_handle(h->self);

    wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                   "kqueue del: %p, fd: %d",
                   h->self, fd);

    e->index--;
    changes = e->changes;
    for (i = e->index; i >= 0; i--) {
        if (changes[i].ident == fd) {
            if (i < e->index) {
                changes[i] = changes[e->index];
            }

            return WSGI_OK;
        }
    }

    wsgi_log_error(e->log, WSGI_LOG_SOURCE_EVENT,
                   "kqueue del: %p, fd: %d, not found",
                   h->self, fd);

    return WSGI_ERROR;
}


static int
wsgi_kqueue_wait(void *self, int timeout)
{
    int ready;
    wsgi_kqueue_t *e;
    struct kevent *ev;
    wsgi_event_handler_t *h;

    e = self;

    wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                   "kqueue wait timeout: %d",
                   timeout);

    ready = kevent(e->fd, e->changes, e->index,
                   e->events, e->events_length, NULL);
    if (ready == -1) {
        wsgi_log_error(e->log, WSGI_LOG_SOURCE_EVENT,
                       "kqueue wait: errno %d: %s",
                       errno, strerror(errno));
        return WSGI_ERROR;
    }

    if (ready == 0) {
        wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                       "kqueue wait timed out");
    }
    else {
        wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                       "kqueue processing %d event(s)",
                       ready);
    }

    for (ev = e->events; ready--; ev++) {
        h = ev->udata;

        wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                       "kqueue processing for %p, flags: 0x%02X",
                       h->self, ev->flags);

        if (ev->flags & EV_ERROR) {
            wsgi_log_warn(e->log, WSGI_LOG_SOURCE_EVENT,
                          "kqueue wait error for %p, fd: %d",
                          h->self, h->get_handle(h->self));
        }

        h->handle_event(h->self);
    }

    return WSGI_OK;
}


static int
wsgi_kqueue_close(void *self)
{
    wsgi_kqueue_t *e;
    int fd;

    e = self;
    fd = e->fd;
    e->fd = -1;

    wsgi_log_debug(e->log, WSGI_LOG_SOURCE_EVENT,
                   "kqueue closing");

    if (close(fd) == -1) {
        wsgi_log_error(e->log, WSGI_LOG_SOURCE_EVENT,
                       "kqueue close: %d, errno %d: %s",
                       fd, errno, strerror(errno));
        return WSGI_ERROR;
    }

    return WSGI_OK;
}

#endif
