
#include <wsgi_core.h>
#include <wsgi_event.h>
#include <wsgi_epoll.h>
#include <wsgi_http.h>


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


int test_cycle(wsgi_log_t *log)
{
    u_int i;
    wsgi_cycle_t* cycle;

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


int test_acceptor(wsgi_log_t *log)
{
    wsgi_gc_t *gc;
    wsgi_addr_t *addr;
    wsgi_event_loop_t *l;
    wsgi_reactor_t *r;
    wsgi_pool_t pool;
    wsgi_acceptor_t *a;

    gc = wsgi_gc_create(512, log);

    if (wsgi_signal_init(log) != WSGI_OK) {
        goto failed;
    }

    addr = wsgi_addr_resolve(gc, (u_char *) "unix:/tmp/sample.sock");
    //addr = wsgi_addr_resolve(gc, (u_char *) "127.0.0.1:8080");
    //addr = wsgi_addr_resolve(gc, (u_char *) "[::1]:8080");
    if (addr == NULL) {
        goto failed;
    }

    l = wsgi_epoll_create(gc, 16);
    if (l == NULL) {
        goto failed;
    }

    if (wsgi_pool_init(&pool, gc, 4, sizeof(wsgi_connection_t)) != WSGI_OK) {
        goto failed;
    }

    if (wsgi_http_connection_pool_init(&pool, log) != WSGI_OK) {
        goto failed;
    }

    r = wsgi_reactor_create(gc, l);

    a = wsgi_acceptor_create(gc, r, &pool);
    if (a == NULL) {
        goto failed;
    }

    a->handle_open = wsgi_http_connection_open;
    if (wsgi_acceptor_open(a, addr) != WSGI_OK) {
        wsgi_acceptor_close(a);
        goto failed;
    }

    wsgi_reactor_wait_for_events(r);
    //wsgi_sleep(10);
    wsgi_acceptor_close(a);
    wsgi_http_connection_pool_close(&pool);
    wsgi_reactor_destroy(r);

failed:
    wsgi_gc_destroy(gc);

    return 0;
}


int main(int argc, char *argv[])
{
    wsgi_log_t *log;

    log = wsgi_log_init();
    log->log_level = WSGI_LOG_DEBUG;
    log->log_source = WSGI_LOG_SOURCE_CORE
        | WSGI_LOG_SOURCE_ALLOC
        | WSGI_LOG_SOURCE_GC
        | WSGI_LOG_SOURCE_LIST
        | WSGI_LOG_SOURCE_POOL
        | WSGI_LOG_SOURCE_CONFIG
        | WSGI_LOG_SOURCE_SOCKET
        | WSGI_LOG_SOURCE_EVENT
        | WSGI_LOG_SOURCE_ACCEPTOR
        | WSGI_LOG_SOURCE_HTTP;
    wsgi_log_set_source(WSGI_LOG_SOURCE_ALLOC, "alloc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_GC, "gc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_CORE, "core");
    wsgi_log_set_source(WSGI_LOG_SOURCE_LIST, "list");
    wsgi_log_set_source(WSGI_LOG_SOURCE_POOL, "pool");
    wsgi_log_set_source(WSGI_LOG_SOURCE_CONFIG, "config");
    wsgi_log_set_source(WSGI_LOG_SOURCE_SOCKET, "socket");
    wsgi_log_set_source(WSGI_LOG_SOURCE_EVENT, "event");
    wsgi_log_set_source(WSGI_LOG_SOURCE_ACCEPTOR, "acceptor");
    wsgi_log_set_source(WSGI_LOG_SOURCE_HTTP, "http");

    //test_cycle(log);
    test_acceptor(log);
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
