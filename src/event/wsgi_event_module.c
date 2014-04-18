
#include <wsgi_event.h>


static int wsgi_event_config_use(
        wsgi_config_t *c, wsgi_config_option_t *o);
static int wsgi_event_config_events(
        wsgi_config_t *c, wsgi_config_option_t *o);

static void *wsgi_event_module_create(wsgi_cycle_t *cycle);
static int wsgi_event_module_init(void *self);
static int wsgi_event_module_shutdown(void *self);


typedef struct {
    const char                  *name;
    wsgi_event_loop_create_pt   *create;
} wsgi_event_loop_factory_t;

struct wsgi_event_ctx_s {
    wsgi_gc_t                   *gc;
    u_int                       events;
    wsgi_list_t                 event_loops;
    wsgi_event_loop_factory_t   *event_loop_factory;
    wsgi_reactor_t              *reactor;
};


static const wsgi_config_def_t config_defs[] = {
    { "use",
      WSGI_CONFIG_DEF_ROOT,
      wsgi_event_config_use },
    { "events",
      WSGI_CONFIG_DEF_ROOT,
      wsgi_event_config_events },
    { 0, 0, 0 }
};


wsgi_module_t event_module = {
    "event", -1,
    config_defs,
    wsgi_event_module_create,
    wsgi_event_module_init,
    wsgi_event_module_shutdown
};

/* region: module context */

wsgi_reactor_t*
wsgi_event_ctx_get_reactor(wsgi_event_ctx_t* ctx)
{
    assert(ctx->reactor != NULL);
    return ctx->reactor;
}


int
wsgi_event_ctx_add_event_loop(wsgi_event_ctx_t* ctx, const char *name,
                              wsgi_event_loop_create_pt create)
{
    wsgi_event_loop_factory_t *f;

    f = wsgi_list_append(&ctx->event_loops);
    if (f == NULL) {
        return WSGI_ERROR;
    }

    f->name = name;
    f->create = create;

    return WSGI_OK;
}

/* region: module config */

static int
wsgi_event_config_use(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_event_ctx_t *ctx;
    wsgi_event_loop_factory_t *f;
    u_int i;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "use: %s",
                   o->value);

    ctx = o->ctx;
    f = ctx->event_loops.items;

    if (strcasecmp((char *) o->value, "auto") == 0
        && ctx->event_loops.length > 0) {

        wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "using: %s",
                       f->name);

        ctx->event_loop_factory = f;
        return WSGI_OK;
    }

    for (i = 0; i < ctx->event_loops.length; i++, f++) {
        if (strcasecmp((char *) o->value, f->name) == 0) {
            ctx->event_loop_factory = f;
            return WSGI_OK;
        }
    }

    wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "unsupported event loop: %s",
                   o->value);

    return WSGI_ERROR;
}


static int
wsgi_event_config_events(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_event_ctx_t *ctx;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "events: %s",
                   o->value);

    ctx = o->ctx;
    if (ctx->events > 0) {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "duplicate `events` directive");
        return WSGI_ERROR;
    }

    /* TODO: validate input */
    ctx->events = atoi((char *) o->value);

    return WSGI_OK;
}

/* region: module lifetime */

static void *
wsgi_event_module_create(wsgi_cycle_t *cycle)
{
    wsgi_event_ctx_t *ctx;

    ctx = wsgi_gc_calloc(cycle->gc, sizeof(wsgi_event_ctx_t));
    ctx->gc = cycle->gc;
    wsgi_list_init(&ctx->event_loops, cycle->gc,
                   3, sizeof(wsgi_event_loop_factory_t));

    return ctx;
}


static int
wsgi_event_module_init(void *self)
{
    wsgi_event_ctx_t *ctx;
    wsgi_event_loop_t *l;

    ctx = self;

    if (ctx->event_loop_factory == NULL) {
        if (ctx->event_loops.length == 0) {
            wsgi_log_error(ctx->gc->log, WSGI_LOG_SOURCE_CONFIG,
                           "no event loops");
            return WSGI_ERROR;
        }

        ctx->event_loop_factory = ctx->event_loops.items;
    }

    if (ctx->events == 0) {
        ctx->events = WSGI_DEFAULT_EVENTS;
    }

    l = ctx->event_loop_factory->create(ctx->gc, ctx->events);
    if (l == NULL) {
        return WSGI_ERROR;
    }

    ctx->reactor = wsgi_reactor_create(ctx->gc, l);
    if (ctx->reactor == NULL) {
        return WSGI_ERROR;
    }

    return WSGI_OK;
}


static int
wsgi_event_module_shutdown(void *self)
{
    wsgi_event_ctx_t *ctx;
    wsgi_reactor_t *reactor;

    ctx = self;
    reactor = ctx->reactor;
    ctx->reactor = NULL;

    return wsgi_reactor_destroy(reactor);
}
