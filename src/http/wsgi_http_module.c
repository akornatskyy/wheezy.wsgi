
#include <wsgi_http.h>


static int wsgi_http_config_worker_connections(
        wsgi_config_t *c, wsgi_config_option_t *o);
static int wsgi_http_config_server(
        wsgi_config_t *c, wsgi_config_option_t *o);
static int wsgi_http_config_server_listen(wsgi_config_t *c,
        wsgi_config_option_t *o);
static int wsgi_http_config_server_request_header_buffer_size(
        wsgi_config_t *c, wsgi_config_option_t *o);
static int wsgi_http_config_server_runtime(
        wsgi_config_t *c, wsgi_config_option_t *o);

static void *wsgi_http_module_create(wsgi_cycle_t *cycle);
static int wsgi_http_module_init(void *self);
static int wsgi_http_module_shutdown(void *self);


typedef struct {
    const char                  *name;
    wsgi_http_runtime_create_pt *create;
} wsgi_http_runtime_factory_t;

struct wsgi_http_ctx_s {
    wsgi_cycle_t                *cycle;
    wsgi_http_config_t          config;
    wsgi_pool_t                 *pool;
    wsgi_list_t                 servers;
    wsgi_list_t                 runtimes;
};

typedef struct {
    wsgi_http_server_config_t   config;
    wsgi_acceptor_t             *acceptor;
} wsgi_http_server_t;


extern wsgi_module_t event_module;

static const wsgi_config_def_t config_defs[] = {
    { "worker_connections",
      WSGI_CONFIG_DEF_ROOT,
      wsgi_http_config_worker_connections },
    { "servers",
      WSGI_CONFIG_DEF_ROOT | WSGI_CONFIG_DEF_SEQUENCE,
      wsgi_http_config_server },
    { "listen",
      WSGI_CONFIG_DEF_SERVER,
      wsgi_http_config_server_listen },
    { "request_header_buffer_size",
      WSGI_CONFIG_DEF_SERVER,
      wsgi_http_config_server_request_header_buffer_size },
    { "runtime",
      WSGI_CONFIG_DEF_SERVER,
      wsgi_http_config_server_runtime },
    { 0, 0, 0 }
};


wsgi_module_t http_module = {
    "http", -1,
    config_defs,
    wsgi_http_module_create,
    wsgi_http_module_init,
    wsgi_http_module_shutdown
};

/* region: module context */

int
wsgi_http_ctx_add_runtime(wsgi_http_ctx_t *ctx,
                          const char *name,
                          wsgi_http_runtime_create_pt *create)
{
    wsgi_http_runtime_factory_t *f;

    f = wsgi_list_append(&ctx->runtimes);
    if (f == NULL) {
        return WSGI_ERROR;
    }

    f->name = name;
    f->create = create;

    return WSGI_OK;
}

/* region: module config */

static int
wsgi_http_config_worker_connections(
        wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_http_ctx_t *ctx;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "worker_connections: %s",
                   o->value);

    ctx = o->ctx;
    if (ctx->config.worker_connections > 0) {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "duplicate `worker_connections` directive");
        return WSGI_ERROR;
    }

    /* TODO: validate input */
    ctx->config.worker_connections = atoi((char *) o->value);

    return WSGI_OK;
}


static int
wsgi_http_config_server(wsgi_config_t *c, wsgi_config_option_t *o)
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

    memset(server, 0, sizeof(wsgi_http_server_t));

    /* TODO: **server
    server = wsgi_gc_malloc(ctx->gc, sizeof(wsgi_http_server_t));
    */
    server->config.http_config = &ctx->config;

    return WSGI_OK;
}


static int
wsgi_http_config_server_listen(wsgi_config_t *c, wsgi_config_option_t *o)
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
    if (server->config.listen != NULL) {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "duplicate `listen` directive");
        return WSGI_ERROR;
    }

    server->config.listen = wsgi_addr_resolve(ctx->cycle->gc, o->value);
    if (server->config.listen == NULL) {
        return WSGI_ERROR;
    }

    return WSGI_OK;
}


static int
wsgi_http_config_server_request_header_buffer_size(
        wsgi_config_t *c, wsgi_config_option_t *o)
{
    wsgi_http_ctx_t *ctx;
    wsgi_http_server_t *server;
    wsgi_list_t *servers;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "  request_header_buffer_size: %s",
                   o->value);

    ctx = o->ctx;
    servers = &ctx->servers;
    server = wsgi_list_last_item(servers);
    if (server->config.request_header_buffer_size != 0) {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "duplicate `request_header_buffer_size` directive");
        return WSGI_ERROR;
    }

    /* TODO: validate input */
    server->config.request_header_buffer_size = atoi((char *) o->value);

    return WSGI_OK;
}


static int
wsgi_http_config_server_runtime(
        wsgi_config_t *c, wsgi_config_option_t *o)
{
    u_int n;
    wsgi_http_ctx_t *ctx;
    wsgi_http_runtime_t *r;
    wsgi_http_runtime_factory_t *f;
    wsgi_http_server_t *server;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "  runtime: %s",
                   o->value);

    ctx = o->ctx;
    server = wsgi_list_last_item(&ctx->servers);
    if (server->config.runtime != NULL) {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "duplicate `runtime` directive");
        return WSGI_ERROR;
    }

    f = ctx->runtimes.items;
    for (n = ctx->runtimes.length; n-- > 0; f++) {
        if (strcasecmp((char *) o->value, f->name) == 0) {

            r = f->create(ctx->cycle);
            if (r == NULL) {
                return WSGI_ERROR;
            }

            assert(r->load && r->process && r->unload);
            server->config.runtime = r;

            return WSGI_OK;
        }
    }

    wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "unsupported runtime: %s",
                   o->value);

    return WSGI_ERROR;
}

/* region: module lifetime */

static void *
wsgi_http_module_create(wsgi_cycle_t *cycle)
{
    wsgi_http_ctx_t *ctx;

    ctx = wsgi_gc_calloc(cycle->gc, sizeof(wsgi_http_ctx_t));
    ctx->cycle = cycle;
    if (wsgi_list_init(&ctx->servers, cycle->gc, 2,
                       sizeof(wsgi_http_server_t)) != WSGI_OK) {
        return NULL;
    }

    if (wsgi_list_init(&ctx->runtimes, cycle->gc, 2,
                       sizeof(wsgi_http_runtime_factory_t)) != WSGI_OK) {
        return NULL;
    }

    return ctx;
}


static int
wsgi_http_module_init(void *self)
{
    u_int n;
    wsgi_acceptor_t *acceptor;
    wsgi_connection_t *c;
    wsgi_gc_t *gc;
    wsgi_http_ctx_t *ctx;
    wsgi_http_runtime_t *runtime;
    wsgi_http_server_t *server;
    wsgi_pool_t *pool;
    wsgi_reactor_t *reactor;

    ctx = self;

    if (ctx->config.worker_connections == 0) {
        ctx->config.worker_connections = WSGI_DEFAULT_WORKER_CONNECTIONS;
    }

    wsgi_log_info(ctx->cycle->log, WSGI_LOG_SOURCE_HTTP,
                  "connections: %d",
                  ctx->config.worker_connections);

    pool = wsgi_pool_create(ctx->cycle->gc,
                            ctx->config.worker_connections,
                            sizeof(wsgi_connection_t));
    if (pool == NULL) {
        return WSGI_ERROR;
    }

    ctx->pool = pool;
    for (n = pool->capacity, c = pool->items; n-- > 0; c++) {
        gc = wsgi_gc_create(WSGI_DEFAULT_CONNECTION_GC_SIZE,
                            ctx->cycle->log);
        if (gc == NULL) {
            return WSGI_ERROR;
        }

        wsgi_connection_init(c, gc, wsgi_http_connection_handle_read);
    }

    reactor = wsgi_event_ctx_get_reactor(ctx->cycle->ctx[event_module.id]);
    server = ctx->servers.items;
    for (n = ctx->servers.length; n-- > 0; server++) {

        runtime = server->config.runtime;
        if (runtime == NULL) {
            wsgi_log_error(ctx->cycle->log, WSGI_LOG_SOURCE_HTTP,
                           "server: %p, runtime required",
                           server);
            return WSGI_ERROR;
        }

        if (runtime->load(runtime->self) != WSGI_OK) {
            wsgi_log_error(ctx->cycle->log, WSGI_LOG_SOURCE_HTTP,
                           "server: %p, unable to load runtime",
                           server);
            return WSGI_ERROR;
        }

        if (server->config.request_header_buffer_size == 0) {
            server->config.request_header_buffer_size =
                WSGI_DEFAULT_REQUEST_HEADER_BUFFER_SIZE;
        }

        acceptor = wsgi_acceptor_create(ctx->cycle->gc, reactor, pool,
                                        wsgi_http_connection_open,
                                        &server->config);

        if (acceptor == NULL) {
            return WSGI_ERROR;
        }

        if (wsgi_acceptor_open(acceptor, server->config.listen) != WSGI_OK) {
            wsgi_acceptor_close(acceptor);
            return WSGI_ERROR;
        }

        server->acceptor = acceptor;
    }

    return WSGI_OK;
}


static int
wsgi_http_module_shutdown(void *self)
{
    u_int n;
    wsgi_acceptor_t *acceptor;
    wsgi_connection_t *c;
    wsgi_http_ctx_t *ctx;
    wsgi_http_runtime_t *runtime;
    wsgi_http_server_t *server;
    wsgi_pool_t *pool;

    ctx = self;
    server = ctx->servers.items;
    for (n = ctx->servers.length; n-- > 0; server++) {

        runtime = server->config.runtime;
        runtime->unload(runtime->self);

        acceptor = server->acceptor;
        server->acceptor = NULL;
        if (wsgi_acceptor_close(acceptor) != WSGI_OK) {
            return WSGI_ERROR;
        }
    }

    pool = ctx->pool;
    ctx->pool = NULL;

    for (n = pool->capacity, c = pool->items; n-- > 0; c++) {
        wsgi_connection_close(c);
        wsgi_gc_destroy(c->gc);
    }

    return WSGI_OK;
}
