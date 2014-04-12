#ifndef WSGI_HTTP_CONNECTION_H
#define WSGI_HTTP_CONNECTION_H


int wsgi_http_connection_open(wsgi_connection_t *c);
int wsgi_http_connection_handle_read(void *self);

#define wsgi_http_connection_config(c) \
    ((wsgi_http_server_config_t *)c->acceptor->config)


#endif /* WSGI_HTTP_CONNECTION_H */
