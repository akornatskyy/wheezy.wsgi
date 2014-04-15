#ifndef WSGI_HTTP_MODULE_H
#define WSGI_HTTP_MODULE_H


typedef wsgi_http_runtime_t* (wsgi_http_runtime_create_pt)(wsgi_gc_t *gc);

struct wsgi_http_config_s {
    u_int                   worker_connections;
};

struct wsgi_http_server_config_s {
    wsgi_http_config_t      *http_config;
    wsgi_addr_t             *listen;
    u_int                   request_header_buffer_size;
    wsgi_http_runtime_t     *runtime;
};


int
wsgi_http_ctx_add_runtime(wsgi_http_ctx_t *ctx,
                          const char *name,
                          wsgi_http_runtime_create_pt *create);

#endif /* WSGI_HTTP_MODULE_H */
