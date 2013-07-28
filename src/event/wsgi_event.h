#ifndef _WSGI_EVENT_H_INCLUDED_
#define _WSGI_EVENT_H_INCLUDED_


typedef struct wsgi_acceptor_s wsgi_acceptor_t;
typedef struct wsgi_connection_s wsgi_connection_t;
typedef struct wsgi_event_ctx_s wsgi_event_ctx_t;
typedef struct wsgi_event_handler_s wsgi_event_handler_t;
typedef struct wsgi_event_loop_s wsgi_event_loop_t;
typedef struct wsgi_reactor_s wsgi_reactor_t;


#define WSGI_DEFAULT_CONNECTION_GC_SIZE 256
#define WSGI_DEFAULT_EVENTS 16

#define WSGI_LOG_SOURCE_EVENT       1 << 10
#define WSGI_LOG_SOURCE_ACCEPTOR    1 << 11


#include <wsgi_core.h>
#include <wsgi_acceptor.h>
#include <wsgi_connection.h>
#include <wsgi_event_module.h>
#include <wsgi_reactor.h>

#endif /* _WSGI_EVENT_H_INCLUDED_ */
