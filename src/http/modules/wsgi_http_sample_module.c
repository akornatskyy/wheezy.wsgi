
#include <wsgi_http.h>
#include <wsgi_http_sample_module.h>


static int
wsgi_http_config_sample_option(wsgi_config_t *c, wsgi_config_option_t *o);
static void *
wsgi_http_sample_module_create(wsgi_cycle_t *cycle);
static wsgi_http_runtime_t *
wsgi_http_sample_runtime_create(wsgi_cycle_t *gc);
static int
wsgi_http_sample_runtime_load(void *self);
static int
wsgi_http_sample_runtime_process(void *self, wsgi_http_request_t *r);
static int
wsgi_http_sample_runtime_unload(void *self);


typedef struct {
    const wsgi_log_t        *log;
    u_char                  *sample_option;
    wsgi_http_runtime_t     runtime;
} wsgi_http_sample_t;

typedef struct {
    wsgi_list_t             runtimes;
} wsgi_http_sample_ctx_t;


extern wsgi_module_t http_module;

static const wsgi_config_def_t config_defs[] = {
    { "sample_option",
      WSGI_CONFIG_DEF_SERVER,
      wsgi_http_config_sample_option },
    { 0, 0, 0 }
};

wsgi_module_t http_sample_module = {
    "sample", -1,
    config_defs,
    wsgi_http_sample_module_create,
    NULL,
    NULL
};

/* region: module config */

static int
wsgi_http_config_sample_option(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_http_sample_ctx_t *ctx;
    wsgi_http_sample_t *s;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "sample_option: %s",
                   o->value);

    ctx = o->ctx;
    s = wsgi_list_last_item(&ctx->runtimes);

    s->sample_option = (u_char *) strdup((char *) o->value);

    return WSGI_OK;
}

/* region: module lifetime */

static void *
wsgi_http_sample_module_create(wsgi_cycle_t *cycle)
{
    wsgi_http_sample_ctx_t *ctx;

    if (wsgi_http_ctx_add_runtime(
            cycle->ctx[http_module.id],
            http_sample_module.name,
            wsgi_http_sample_runtime_create) != WSGI_OK) {
        return NULL;
    }

    ctx = wsgi_gc_calloc(cycle->gc, sizeof(wsgi_http_sample_ctx_t));

    if (wsgi_list_init(&ctx->runtimes, cycle->gc, 2,
                       sizeof(wsgi_http_sample_t)) != WSGI_OK) {
        return NULL;
    }

    return ctx;
}


static wsgi_http_runtime_t *
wsgi_http_sample_runtime_create(wsgi_cycle_t *cycle)
{
    wsgi_http_runtime_t *r;
    wsgi_http_sample_ctx_t *ctx;
    wsgi_http_sample_t *s;

    ctx = cycle->ctx[http_sample_module.id];
    s = wsgi_list_append(&ctx->runtimes);
    if (s == NULL) {
        return NULL;
    }

    s->log = cycle->log;
    s->sample_option = NULL;
    r = &s->runtime;
    r->self = s;
    r->load = wsgi_http_sample_runtime_load;
    r->process = wsgi_http_sample_runtime_process;
    r->unload = wsgi_http_sample_runtime_unload;

    return r;
}


static int
wsgi_http_sample_runtime_load(void *self)
{
    wsgi_http_sample_t *s;

    s = self;
    if (s->sample_option == NULL) {
        wsgi_log_error(s->log, WSGI_LOG_SOURCE_HTTP,
                       "sample: %p, sample_option required",
                        s);
        return WSGI_ERROR;
    }

    wsgi_log_debug(s->log, WSGI_LOG_SOURCE_HTTP,
                   "load: %s",
                   s->sample_option);

    return WSGI_OK;
}


static int
wsgi_http_sample_runtime_process(void *self, wsgi_http_request_t *r)
{
    wsgi_http_sample_t *s;

    s = self;

    wsgi_log_debug(s->log, WSGI_LOG_SOURCE_HTTP,
                   "process: %s",
                   s->sample_option);

    return WSGI_OK;
}


static int
wsgi_http_sample_runtime_unload(void *self)
{
    wsgi_http_sample_t *s;

    s = self;

    wsgi_log_debug(s->log, WSGI_LOG_SOURCE_HTTP,
                   "unload: %s",
                   s->sample_option);

    return WSGI_OK;
}
