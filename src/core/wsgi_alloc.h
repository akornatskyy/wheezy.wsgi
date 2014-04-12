#ifndef WSGI_ALLOC_H
#define WSGI_ALLOC_H


void *wsgi_malloc(size_t size, const wsgi_log_t *log);
#define wsgi_free   free


#endif /* WSGI_ALLOC_H */
