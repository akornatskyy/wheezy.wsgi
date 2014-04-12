#ifndef WSGI_ADDR_H
#define WSGI_ADDR_H


typedef struct {
    struct sockaddr     *sa;
    socklen_t           length;
    u_char              *name;
} wsgi_addr_t;


wsgi_addr_t *wsgi_addr_resolve(wsgi_gc_t *gc, const u_char *url);

#endif /* WSGI_ADDR_H */
