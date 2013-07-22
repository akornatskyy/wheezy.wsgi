
#include <wsgi_event.h>
#include <wsgi_epoll.h>


static int wsgi_events(wsgi_config_t *c, wsgi_config_option_t *o);

static void *wsgi_event_create(wsgi_cycle_t *cycle);
static int wsgi_event_init(wsgi_cycle_t *cycle, void *ctx);
static int wsgi_event_shutdown(wsgi_cycle_t *cycle, void *ctx);


static wsgi_config_def_t config_defs[] = {
    { "events",
      WSGI_CONFIG_DEF_ROOT,
      wsgi_events },
    { 0 }
};


wsgi_module_t event_module = {
    "event", -1,
    config_defs,
    wsgi_event_create,
    wsgi_event_init,
    wsgi_event_shutdown
};


static int
wsgi_events(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_event_ctx_t *ctx;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "events: %s",
                   o->value);

    ctx = o->ctx;

    // TODO: validate input
    ctx->events = atoi((char *) o->value);

    return WSGI_OK;
}


static void *
wsgi_event_create(wsgi_cycle_t *cycle)
{
    wsgi_event_ctx_t *ctx;

    ctx = wsgi_gc_malloc(cycle->gc, sizeof(wsgi_event_ctx_t));
    ctx->gc = cycle->gc;
    ctx->events = 16;

    return ctx;
}


static int
wsgi_event_init(wsgi_cycle_t *cycle, void *c)
{
    wsgi_event_ctx_t *ctx;
    wsgi_event_loop_t *l;

    ctx = c;
    l = wsgi_epoll_create(ctx->gc, ctx->events);
    if (l == NULL) {
        return WSGI_ERROR;
    }

    ctx->reactor = wsgi_reactor_create(ctx->gc, l);

    return WSGI_OK;
}


static int
wsgi_event_shutdown(wsgi_cycle_t *cycle, void *c)
{
    wsgi_event_ctx_t *ctx;
    wsgi_reactor_t *reactor;

    ctx = c;
    reactor = ctx->reactor;
    ctx->reactor = NULL;

    return wsgi_reactor_destroy(reactor);
}
