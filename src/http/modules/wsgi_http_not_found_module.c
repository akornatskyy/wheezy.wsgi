
#include <wsgi_http.h>
#include <wsgi_http_not_found_module.h>


static void *wsgi_http_not_found_module_create(wsgi_cycle_t *cycle);


wsgi_module_t http_not_found_module = {
    "not_found", -1,
    NULL,
    wsgi_http_not_found_module_create,
    NULL,
    NULL
};


static void *
wsgi_http_not_found_module_create(wsgi_cycle_t *cycle)
{
    return cycle;
}
