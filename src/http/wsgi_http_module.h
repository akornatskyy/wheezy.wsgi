#ifndef _WSGI_HTTP_MODULE_H_INCLUDED_
#define _WSGI_HTTP_MODULE_H_INCLUDED_


struct wsgi_http_ctx_s {
    wsgi_gc_t           *gc;
    wsgi_list_t         servers;
};

struct wsgi_http_server_s {
    uint                worker_connections;
    wsgi_addr_t         *listen;
    wsgi_pool_t         *pool;
    wsgi_acceptor_t     *acceptor;
};

#endif /* _WSGI_HTTP_MODULE_H_INCLUDED_ */
