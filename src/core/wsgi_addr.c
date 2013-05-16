
#include <wsgi_core.h>


static int wsgi_init_unix(wsgi_addr_t *a, wsgi_gc_t *gc, const u_char *url);
static int wsgi_init_inet(wsgi_addr_t *s, wsgi_gc_t *gc, const u_char *url);
static int wsgi_sockaddr_ntop(const struct sockaddr *sa, const u_char *port,
                              wsgi_gc_t *gc, u_char **name);
static int wsgi_inet_ntop(const struct in_addr *ia, const u_char *port,
                          wsgi_gc_t *gc, u_char **name);
static int wsgi_inet6_ntop(const struct in6_addr *ia, const u_char *port,
                           wsgi_gc_t *gc, u_char **name);


wsgi_addr_t *wsgi_addr_resolve(wsgi_gc_t *gc, const u_char *url)
{
    wsgi_addr_t *a;

    a = wsgi_gc_malloc(gc, sizeof(wsgi_addr_t));
    if (a == NULL) {
        return NULL;
    }

    if (strncmp("unix:", (char *) url, 5) == 0) {
        if (wsgi_init_unix(a, gc, url) != WSGI_OK) {
            return NULL;
        }
    } else {
        if (wsgi_init_inet(a, gc, url) != WSGI_OK) {
            return NULL;
        }
    }

    return a;
}


static int wsgi_init_unix(wsgi_addr_t *a, wsgi_gc_t *gc, const u_char *url)
{
    u_int l;
    struct sockaddr_un *sa;

    l = strlen((char *) url);
    l -= 5;

    if (l == 0) {
        wsgi_log_error(gc->log, WSGI_LOG_SOURCE_SOCKET,
                       "unix domain socket: empty path");
        return WSGI_ERROR;
    }

    if (l >= sizeof(sa->sun_path)) {
        wsgi_log_error(gc->log, WSGI_LOG_SOURCE_SOCKET,
                       "unix domain socket: too long path");
        return WSGI_ERROR;
    }

    sa = wsgi_gc_calloc(gc, sizeof(struct sockaddr_un));
    if (sa == NULL) {
        return WSGI_ERROR;
    }

    sa->sun_family = AF_UNIX;
    memcpy(sa->sun_path, url + 5, l);

    a->sa = (struct sockaddr *) sa;
    a->length = sizeof(struct sockaddr_un);
    l += 6;
    a->name = wsgi_gc_malloc(gc, l);
    if (a->name == NULL) {
        return WSGI_ERROR;
    }

    memcpy(a->name, url, l);

    return WSGI_OK;
}


static int wsgi_init_inet(wsgi_addr_t *a, wsgi_gc_t *gc, const u_char *url)
{
    u_char *host;
    const u_char *port;
    int n;
    struct addrinfo hints, *res;
    struct sockaddr *sa;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    port = (u_char *) strrchr((char *) url, ':');
    if (port == NULL) {
        port = url;
        n = getaddrinfo(NULL, (char *) port, &hints, &res);
    } else {
        n = port - url;
        if (url[0] == '[') {
            if (url[--n] != ']') {
                wsgi_log_error(gc->log, WSGI_LOG_SOURCE_SOCKET,
                               "invalid ipv6 address, %s",
                               url);
                return WSGI_ERROR;
            }

            host = wsgi_malloc(n--, gc->log);
            if (host == NULL) {
                return WSGI_ERROR;
            }

            memcpy(host, url + 1, n);
        }
        else {
            host = wsgi_malloc(n + 1, gc->log);
            if (host == NULL) {
                return WSGI_ERROR;
            }

            memcpy(host, url, n);
        }

        host[n] = '\0';
        port++;

        n = getaddrinfo((char *) host, (char *) port, &hints, &res);
        wsgi_free(host);
    }

    if (n != 0) {
        wsgi_log_error(gc->log, WSGI_LOG_SOURCE_SOCKET,
                       "inet socket: %d, %s",
                       n, gai_strerror(n));
        return WSGI_ERROR;
    }

    sa = wsgi_gc_malloc(gc, res->ai_addrlen);
    if (sa == NULL) {
        goto failed;
    }

    a->sa = memcpy(sa, res->ai_addr, res->ai_addrlen);
    a->length = res->ai_addrlen;

    if (wsgi_sockaddr_ntop(sa, port, gc, &a->name) != WSGI_OK) {
        goto failed;
    }

    freeaddrinfo(res);

    return WSGI_OK;

failed:

    freeaddrinfo(res);

    return WSGI_ERROR;
}


static int wsgi_sockaddr_ntop(const struct sockaddr *sa, const u_char *port,
                              wsgi_gc_t *gc, u_char **name)
{
    switch(sa->sa_family) {
        case AF_INET:
            return wsgi_inet_ntop(&(((struct sockaddr_in *)sa)->sin_addr),
                                  port, gc, name);
        case AF_INET6:
            return wsgi_inet6_ntop(&(((struct sockaddr_in6 *)sa)->sin6_addr),
                                   port, gc, name);
        default:
            return WSGI_ERROR;
    }
}


static int wsgi_inet_ntop(const struct in_addr *ia, const u_char *port,
                          wsgi_gc_t *gc, u_char **name)
{
    char b[INET_ADDRSTRLEN];
    char *p;
    u_int l, n;

    if (inet_ntop(AF_INET, ia, b, INET_ADDRSTRLEN) == NULL) {
        return WSGI_ERROR;
    }

    n = strlen(b);
    l = strlen((char *) port) + 1;
    p = wsgi_gc_malloc(gc, n + l + 1);
    if (p == NULL) {
        return WSGI_ERROR;
    }

    *name = (u_char *) p;

    memcpy(p, b, n);
    p += n;
    *p++ = ':';
    memcpy(p, port, l);

    return WSGI_OK;
}


static int wsgi_inet6_ntop(const struct in6_addr *ia, const u_char *port,
                           wsgi_gc_t *gc, u_char **name)
{
    char b[INET6_ADDRSTRLEN];
    char *p;
    u_int l, n;

    if (inet_ntop(AF_INET6, ia, b, INET6_ADDRSTRLEN) == NULL) {
        return WSGI_ERROR;
    }

    n = strlen(b);
    l = strlen((char *) port) + 1;
    p = wsgi_gc_malloc(gc, n + l + 3);
    if (p == NULL) {
        return WSGI_ERROR;
    }

    *name = (u_char *) p;

    *p++ = '[';
    memcpy(p, b, n);
    p += n;
    *p++ = ']';
    *p++ = ':';
    memcpy(p, port, l);

    return WSGI_OK;
}
