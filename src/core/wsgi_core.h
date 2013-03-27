#ifndef _WSGI_CORE_H_INCLUDED_
#define _WSGI_CORE_H_INCLUDED_


typedef struct wsgi_log_s wsgi_log_t;
typedef struct wsgi_gc_s wsgi_gc_t;
typedef struct wsgi_list_s wsgi_list_t;


#define WSGI_OK                 0
#define WSGI_ERROR             -1

#define WSGI_LOG_SOURCE_CORE    1
#define WSGI_LOG_SOURCE_ALLOC   2
#define WSGI_LOG_SOURCE_GC      3
#define WSGI_LOG_SOURCE_LIST    4


#include <wsgi_alloc.h>
#include <wsgi_gc.h>
#include <wsgi_list.h>
#include <wsgi_log.h>

#endif /* _WSGI_CORE_H_INCLUDED_ */
