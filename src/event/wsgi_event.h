#ifndef _WSGI_EVENT_H_INCLUDED_
#define _WSGI_EVENT_H_INCLUDED_


typedef struct wsgi_acceptor_s wsgi_acceptor_t;
typedef struct wsgi_connection_s wsgi_connection_t;
typedef struct wsgi_event_ctx_s wsgi_event_ctx_t;
typedef struct wsgi_event_handler_s wsgi_event_handler_t;
typedef struct wsgi_event_loop_s wsgi_event_loop_t;
typedef struct wsgi_reactor_s wsgi_reactor_t;


#include <wsgi_core.h>
#include <wsgi_acceptor.h>
#include <wsgi_connection.h>
#include <wsgi_event_module.h>
#include <wsgi_reactor.h>

#endif /* _WSGI_EVENT_H_INCLUDED_ */
