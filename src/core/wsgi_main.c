
#include <wsgi_core.h>


#define WSGI_CONFIG_DEF_SERVER 0x20

static void *wsgi_sample_create(wsgi_cycle_t *cycle);
static int wsgi_sample_init(wsgi_cycle_t *cycle, void *ctx);

static int wsgi_server_add(wsgi_config_t *c, wsgi_config_option_t *o);

static int wsgi_server_listen(wsgi_config_t *c, wsgi_config_option_t *o);

static int wsgi_server_application(wsgi_config_t *c, wsgi_config_option_t *o);


static wsgi_config_def_t sample_config_defs[] = {
    { "servers",
      WSGI_CONFIG_DEF_ROOT | WSGI_CONFIG_DEF_SEQUENCE,
      wsgi_server_add },
    { "listen",
      WSGI_CONFIG_DEF_SERVER,
      wsgi_server_listen },
    { "application",
      WSGI_CONFIG_DEF_SERVER,
      wsgi_server_application },
    { NULL }
};

static wsgi_module_t sample_module = {
    "sample", -1,
    sample_config_defs,
    wsgi_sample_create,
    wsgi_sample_init
};

wsgi_module_t* modules[] = {
    &sample_module
};

const u_int modules_count = sizeof(modules) / sizeof(wsgi_module_t *);


int main(int argc, char *argv[])
{
    u_int i;
    wsgi_log_t *log;
    wsgi_cycle_t* cycle;

    log = wsgi_log_init();
    log->log_level = WSGI_LOG_DEBUG;
    log->log_source = WSGI_LOG_SOURCE_CORE
        | WSGI_LOG_SOURCE_ALLOC
        | WSGI_LOG_SOURCE_GC
        | WSGI_LOG_SOURCE_LIST
        | WSGI_LOG_SOURCE_CONFIG;
    wsgi_log_set_source(WSGI_LOG_SOURCE_ALLOC, "alloc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_GC, "gc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_CORE, "core");
    wsgi_log_set_source(WSGI_LOG_SOURCE_LIST, "list");
    wsgi_log_set_source(WSGI_LOG_SOURCE_CONFIG, "config");

    for (i = 0; i < modules_count; i++) {
        modules[i]->id = i;
    }

    cycle = wsgi_cycle_create(log);
    cycle->filename = (u_char*) "conf/wsgi.yaml";

    if (wsgi_cycle_init(cycle) != WSGI_OK) {
        wsgi_cycle_destroy(cycle);
        return 1;
    }

    wsgi_cycle_destroy(cycle);
    return 0;
}

static int wsgi_server_add(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CORE,
                   "add server");
    o->block = WSGI_CONFIG_DEF_SERVER;
    return WSGI_OK;
}

static int wsgi_server_listen(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CORE,
                   "  listen: %s",
                   o->value);
    return WSGI_OK;
}

static int wsgi_server_application(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CORE,
                   "  application: %s",
                   o->value);
    return WSGI_OK;
}

static void *wsgi_sample_create(wsgi_cycle_t *cycle)
{
    wsgi_log_debug(cycle->log, WSGI_LOG_SOURCE_CORE,
                   "create sample config context");
    return "1";
}

static int wsgi_sample_init(wsgi_cycle_t *cycle, void *ctx)
{
    return WSGI_OK;
}
