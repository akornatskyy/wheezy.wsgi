
#include <wsgi_http.h>
#include <wsgi_http_sample_module.h>


static void *wsgi_http_sample_module_create(wsgi_cycle_t *cycle);
static wsgi_http_runtime_t *wsgi_http_sample_runtime_create(wsgi_gc_t *gc);
static int wsgi_http_sample_runtime_load(void *self);
static int
wsgi_http_sample_runtime_process(void *self, wsgi_http_request_t *r);
static int wsgi_http_sample_runtime_unload(void *self);


typedef struct {
    wsgi_http_runtime_t     runtime;
} wsgi_http_sample_t;


extern wsgi_module_t http_module;

const wsgi_module_t http_sample_module = {
    "sample", -1,
    NULL,
    wsgi_http_sample_module_create,
    NULL,
    NULL
};


static void *
wsgi_http_sample_module_create(wsgi_cycle_t *cycle)
{
    if (wsgi_http_ctx_add_runtime(
            cycle->ctx[http_module.id],
            http_sample_module.name,
            wsgi_http_sample_runtime_create) != WSGI_OK) {
        return NULL;
    }

    return cycle;
}


static wsgi_http_runtime_t *
wsgi_http_sample_runtime_create(wsgi_gc_t *gc)
{
    wsgi_http_sample_t *s;
    wsgi_http_runtime_t *r;

    s = wsgi_gc_malloc(gc, sizeof(wsgi_http_sample_t));

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
    return WSGI_OK;
}


static int
wsgi_http_sample_runtime_process(void *self, wsgi_http_request_t *r)
{
    return WSGI_OK;
}


static int
wsgi_http_sample_runtime_unload(void *self)
{
    return WSGI_OK;
}
