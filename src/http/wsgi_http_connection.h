#ifndef _WSGI_HTTP_CONNECTION_H_INCLUDED_
#define _WSGI_HTTP_CONNECTION_H_INCLUDED_


int wsgi_http_connection_open(wsgi_connection_t *c);
int wsgi_http_connection_close(wsgi_connection_t *c);
int wsgi_http_connection_pool_init(wsgi_pool_t *p, wsgi_log_t *log);
int wsgi_http_connection_pool_close(wsgi_pool_t *p);

#endif /* _WSGI_HTTP_CONNECTION_H_INCLUDED_ */
