#ifndef _WSGI_HTTP_CONNECTION_H_INCLUDED_
#define _WSGI_HTTP_CONNECTION_H_INCLUDED_


int wsgi_http_connection_open(wsgi_connection_t *c);

#define wsgi_http_connection_config(c) \
    ((wsgi_http_server_config_t *)c->acceptor->config)

#endif /* _WSGI_HTTP_CONNECTION_H_INCLUDED_ */
