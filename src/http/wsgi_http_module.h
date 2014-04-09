#ifndef _WSGI_HTTP_MODULE_H_INCLUDED_
#define _WSGI_HTTP_MODULE_H_INCLUDED_


struct wsgi_http_config_s {
    uint                worker_connections;
};


struct wsgi_http_server_config_s {
    wsgi_http_config_t  *http_config;
    wsgi_addr_t         *listen;
    uint                request_header_buffer_size;
};

#endif /* _WSGI_HTTP_MODULE_H_INCLUDED_ */
