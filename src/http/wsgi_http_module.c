
#include <wsgi_http.h>


#define WSGI_CONFIG_DEF_SERVER 0x20


static int wsgi_server_add(wsgi_config_t *c, wsgi_config_option_t *o);
static int wsgi_server_listen(wsgi_config_t *c, wsgi_config_option_t *o);
static int wsgi_worker_connections(wsgi_config_t *c, wsgi_config_option_t *o);

static void *wsgi_http_create(wsgi_cycle_t *cycle);
static int wsgi_http_init(wsgi_cycle_t *cycle, void *ctx);
static int wsgi_http_shutdown(wsgi_cycle_t *cycle, void *ctx);


extern wsgi_module_t event_module;

static wsgi_config_def_t config_defs[] = {
    { "servers",
      WSGI_CONFIG_DEF_ROOT | WSGI_CONFIG_DEF_SEQUENCE,
      wsgi_server_add },
    { "listen",
      WSGI_CONFIG_DEF_SERVER,
      wsgi_server_listen },
    { "worker_connections",
      WSGI_CONFIG_DEF_SERVER,
      wsgi_worker_connections },
    { 0 }
};


wsgi_module_t http_module = {
    "http", -1,
    config_defs,
    wsgi_http_create,
    wsgi_http_init,
    wsgi_http_shutdown
};


static int
wsgi_server_add(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_http_ctx_t *ctx;
    wsgi_http_server_t *server;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "add server");

    ctx = o->ctx;
    o->block = WSGI_CONFIG_DEF_SERVER;
    server = wsgi_list_append(&ctx->servers);
    if (server == NULL) {
        return WSGI_ERROR;
    }

    // TODO: **server
    //server = wsgi_gc_malloc(ctx->gc, sizeof(wsgi_http_server_t));
    memset(server, 0, sizeof(wsgi_http_server_t));

    return WSGI_OK;
}


static int
wsgi_server_listen(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_http_ctx_t *ctx;
    wsgi_list_t *servers;
    wsgi_http_server_t *server;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "  listen: %s",
                   o->value);

    ctx = o->ctx;
    servers = &ctx->servers;
    server = wsgi_list_last_item(servers);
    if (server->listen != NULL) {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "duplicate `listen` directive");
        return WSGI_ERROR;
    }

    server->listen = wsgi_addr_resolve(ctx->gc, o->value);
    if (server->listen == NULL) {
        return WSGI_ERROR;
    }

    return WSGI_OK;
}


static int
wsgi_worker_connections(wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_http_ctx_t *ctx;
    wsgi_list_t *servers;
    wsgi_http_server_t *server;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "  worker_connections: %s",
                   o->value);

    ctx = o->ctx;
    servers = &ctx->servers;
    server = wsgi_list_last_item(servers);
    if (server->worker_connections > 0) {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "duplicate `worker_connections` directive");
        return WSGI_ERROR;
    }

    // TODO: validate input
    server->worker_connections = atoi((char *) o->value);

    return WSGI_OK;
}


static void *
wsgi_http_create(wsgi_cycle_t *cycle)
{
    wsgi_http_ctx_t *ctx;

    ctx = wsgi_gc_malloc(cycle->gc, sizeof(wsgi_http_ctx_t));
    ctx->gc = cycle->gc;
    if (wsgi_list_init(&ctx->servers, cycle->gc, 2,
                       sizeof(wsgi_http_server_t)) != WSGI_OK) {
        return NULL;
    }

    return ctx;
}


static int
wsgi_http_init(wsgi_cycle_t *cycle, void *c)
{
    u_int n;
    wsgi_http_ctx_t *ctx;
    wsgi_pool_t *pool;
    wsgi_http_server_t *server;
    wsgi_reactor_t *reactor;
    wsgi_acceptor_t *acceptor;

    ctx = c;
    reactor = ((wsgi_event_ctx_t *)(cycle->ctx[event_module.id]))->reactor;

    server = ctx->servers.items;
    for (n = ctx->servers.length; n-- > 0; server++) {
        if (server->worker_connections == 0) {
            server->worker_connections = 16;
        }

        pool = wsgi_pool_create(ctx->gc,
                                server->worker_connections,
                                sizeof(wsgi_connection_t));
        if (pool == NULL) {
            return WSGI_ERROR;
        }

        if (wsgi_http_connection_pool_init(pool, cycle->log) != WSGI_OK) {
            return WSGI_ERROR;
        }

        server->pool = pool;

        acceptor = wsgi_acceptor_create(ctx->gc, reactor, pool);
        if (acceptor == NULL) {
            return WSGI_ERROR;
        }

        acceptor->handle_open = wsgi_http_connection_open;
        if (wsgi_acceptor_open(acceptor, server->listen) != WSGI_OK) {
            wsgi_acceptor_close(acceptor);
            return WSGI_ERROR;
        }

        server->acceptor = acceptor;
    }

    return WSGI_OK;
}


static int
wsgi_http_shutdown(wsgi_cycle_t *cycle, void *c)
{
    u_int n;
    wsgi_http_ctx_t *ctx;
    wsgi_http_server_t *server;
    wsgi_acceptor_t *acceptor;
    wsgi_pool_t *pool;

    ctx = c;
    server = ctx->servers.items;
    for (n = ctx->servers.length; n-- > 0; server++) {
        acceptor = server->acceptor;
        server->acceptor = NULL;
        if (wsgi_acceptor_close(acceptor) != WSGI_OK) {
            return WSGI_ERROR;
        }

        pool = server->pool;
        server->pool = NULL;
        if (wsgi_http_connection_pool_close(pool) != WSGI_OK) {
            return WSGI_ERROR;
        }
    }

    return WSGI_OK;
}