#ifndef WSGI_HTTP_MODULE_H
#define WSGI_HTTP_MODULE_H


struct wsgi_http_config_s {
    uint                worker_connections;
};


struct wsgi_http_server_config_s {
    wsgi_http_config_t  *http_config;
    wsgi_addr_t         *listen;
    uint                request_header_buffer_size;
};

#endif /* WSGI_HTTP_MODULE_H */
