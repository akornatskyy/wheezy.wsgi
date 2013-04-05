#ifndef _WSGI_ALLOC_H_INCLUDED_
#define _WSGI_ALLOC_H_INCLUDED_


void *wsgi_malloc(size_t size, const wsgi_log_t *log);
#define wsgi_free   free


#endif /* _WSGI_ALLOC_H_INCLUDED_ */
