
#include <wsgi_http.h>
#include <wsgi_http_sample_module.h>


static void *wsgi_http_sample_module_create(wsgi_cycle_t *cycle);
static int wsgi_http_sample_hander(wsgi_http_request_t *r);


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
    if (wsgi_http_ctx_add_handler(
            cycle->ctx[http_module.id],
            "sample",
            wsgi_http_sample_hander) != WSGI_OK) {
        return NULL;
    }

    return cycle;
}


static int
wsgi_http_sample_hander(wsgi_http_request_t *r)
{
    wsgi_log_debug(r->connection->gc->log, WSGI_LOG_SOURCE_HTTP,
                   "request: %p, sample processing",
                   r);
    return WSGI_OK;
}
