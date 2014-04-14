#ifndef WSGI_HTTP_MODULE_H
#define WSGI_HTTP_MODULE_H


struct wsgi_http_config_s {
    u_int                   worker_connections;
};

struct wsgi_http_server_config_s {
    wsgi_http_config_t      *http_config;
    wsgi_addr_t             *listen;
    u_int                   request_header_buffer_size;
    wsgi_http_handler_pt    *process;
};


int
wsgi_http_ctx_add_handler(wsgi_http_ctx_t *ctx,
                          const char * name,
                          wsgi_http_handler_pt *process);

#endif /* WSGI_HTTP_MODULE_H */
