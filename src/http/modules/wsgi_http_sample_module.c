
#include <wsgi_http.h>
#include <wsgi_http_sample_module.h>


static void *wsgi_http_sample_module_create(wsgi_cycle_t *cycle);


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
    return cycle;
}
