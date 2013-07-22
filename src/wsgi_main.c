
#include <wsgi_http.h>


extern wsgi_module_t event_module;
extern wsgi_module_t http_module;

wsgi_module_t* modules[] = {
    &event_module,
    &http_module,
};

const uint modules_count = sizeof(modules) / sizeof(wsgi_module_t *);


static int
run_cycle(wsgi_log_t *log)
{
    u_int i;
    wsgi_cycle_t *cycle;
    wsgi_reactor_t *reactor;

    for (i = 0; i < modules_count; i++) {
        modules[i]->id = i;
    }

    cycle = wsgi_cycle_create(log);
    cycle->filename = (u_char*) "conf/wsgi.yaml";

    if (wsgi_cycle_init(cycle) != WSGI_OK) {
        wsgi_cycle_destroy(cycle);
        return 1;
    }

    reactor = ((wsgi_event_ctx_t *)(cycle->ctx[event_module.id]))->reactor;
    wsgi_reactor_wait_for_events(reactor);

    wsgi_cycle_shutdown(cycle);
    wsgi_cycle_destroy(cycle);

    return 0;
}


int
main(int argc, char *argv[])
{
    wsgi_log_t *log;

    log = wsgi_log_init();
    log->log_level = WSGI_LOG_DEBUG;
    log->log_source = WSGI_LOG_SOURCE_CORE
        //| WSGI_LOG_SOURCE_ALLOC
        //| WSGI_LOG_SOURCE_GC
        //| WSGI_LOG_SOURCE_LIST
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

    if (wsgi_signal_init(log) != WSGI_OK) {
        return 1;
    }

    return run_cycle(log);
}
