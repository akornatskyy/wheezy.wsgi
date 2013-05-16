#ifndef _WSGI_SOCKET_H_INCLUDED_
#define _WSGI_SOCKET_H_INCLUDED_


struct wsgi_socket_s {
    const wsgi_log_t    *log;
    wsgi_addr_t         *addr;
    int                 fd;
};


wsgi_socket_t *wsgi_socket_create(wsgi_gc_t *gc);
void wsgi_socket_init(wsgi_socket_t *s, const wsgi_log_t *log);
int wsgi_socket_open(wsgi_socket_t *s, wsgi_addr_t *a);
int wsgi_socket_accept(wsgi_socket_t *s, wsgi_socket_t *peer, wsgi_gc_t *gc);
int wsgi_socket_close(wsgi_socket_t *s);

#endif /* _WSGI_SOCKET_H_INCLUDED_ */
