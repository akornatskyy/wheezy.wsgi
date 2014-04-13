
#include <wsgi_core.h>


static int
wsgi_cycle_lookup_config(const void *adapter, wsgi_config_option_t *c);


wsgi_cycle_t *
wsgi_cycle_create(const wsgi_log_t* log)
{
    wsgi_gc_t *gc;
    wsgi_cycle_t *cycle;

    gc = wsgi_gc_create(256, log);
    cycle = wsgi_gc_malloc(gc, sizeof(wsgi_cycle_t));
    cycle->log = log;
    cycle->gc = gc;

    return cycle;
}


void
wsgi_cycle_destroy(wsgi_cycle_t *cycle)
{
    wsgi_gc_destroy(cycle->gc);
}


int
wsgi_cycle_init(wsgi_cycle_t *cycle)
{
    u_int i;
    const wsgi_module_t *m;
    void *ctx;
    wsgi_config_t c = {
        .root_block = WSGI_CONFIG_DEF_ROOT,
        .adapter = cycle,
        .lookup = wsgi_cycle_lookup_config,
        .log = cycle->log
    };

    cycle->ctx = wsgi_gc_malloc(cycle->gc, modules_count * sizeof(void *));
    for (i = 0; i < modules_count; i++) {
        m = modules[i];
        if (m->create) {
            ctx = m->create(cycle);
            if (ctx == NULL) {
                wsgi_log_error(cycle->log, WSGI_LOG_SOURCE_CONFIG,
                    "unable to create context for module: %s",
                    m->name);
                return WSGI_ERROR;
            }

            cycle->ctx[m->id] = ctx;
        }
        else {
            cycle->ctx[m->id] = NULL;
        }
    }

    if (wsgi_config_load(&c, cycle->filename) != WSGI_OK) {
        return WSGI_ERROR;
    }

    for (i = 0; i < modules_count; i++) {
        m = modules[i];
        if (m->init) {
            wsgi_log_debug(cycle->log, WSGI_LOG_SOURCE_CONFIG,
                   "initializing module: %s", m->name);
            if (m->init(cycle->ctx[m->id]) != WSGI_OK) {
                wsgi_log_error(cycle->log, WSGI_LOG_SOURCE_CONFIG,
                    "unable to initialize module: %s",
                    m->name);
                return WSGI_ERROR;
            }
        }
    }

    wsgi_log_debug(cycle->log, WSGI_LOG_SOURCE_CONFIG,
                   "initialized");

    return WSGI_OK;
}


int
wsgi_cycle_shutdown(wsgi_cycle_t *cycle)
{
    u_int i;
    const wsgi_module_t *m;

    i = modules_count;
    while (i > 0) {
        m = modules[--i];
        if (m->shutdown) {
            wsgi_log_debug(cycle->log, WSGI_LOG_SOURCE_CONFIG,
                   "shutting down module: %s", m->name);
            if (m->shutdown(cycle->ctx[m->id]) != WSGI_OK) {
                wsgi_log_error(cycle->log, WSGI_LOG_SOURCE_CONFIG,
                    "unable to shutdown module: %s",
                    m->name);
                return WSGI_ERROR;
            }
        }
    }

    wsgi_log_debug(cycle->log, WSGI_LOG_SOURCE_CONFIG,
                   "shutted down");

    return WSGI_OK;
}


int
wsgi_cycle_lookup_config(const void *adapter, wsgi_config_option_t *o)
{
    u_int i;
    const wsgi_cycle_t *cycle;
    const wsgi_config_def_t *d;

    cycle = adapter;
    for (i = 0; i < modules_count; i++) {
        for (d = modules[i]->defs; d && d->name; d++) {
            if ((d->flags & o->block)
                && strcmp((char *)d->name, (char *)o->value) == 0) {
                o->def = d;
                o->ctx = cycle->ctx[modules[i]->id];
                return WSGI_OK;
            }
        }
    }

    return WSGI_ERROR;
}
