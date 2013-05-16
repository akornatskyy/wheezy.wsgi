#ifndef _WSGI_ADDR_H_INCLUDED_
#define _WSGI_ADDR_H_INCLUDED_


typedef struct {
    struct sockaddr     *sa;
    socklen_t           length;
    u_char              *name;
} wsgi_addr_t;


wsgi_addr_t *wsgi_addr_resolve(wsgi_gc_t *gc, const u_char *url);

#endif /* _WSGI_ADDR_H_INCLUDED_ */
