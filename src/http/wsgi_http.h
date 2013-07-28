#ifndef _WSGI_HTTP_H_INCLUDED_
#define _WSGI_HTTP_H_INCLUDED_


typedef struct wsgi_http_header_pair_s wsgi_http_header_pair_t;
typedef struct wsgi_http_request_s wsgi_http_request_t;


#define WSGI_DEFAULT_REQUEST_HEADERS_CAPACITY 12
#define WSGI_DEFAULT_REQUEST_HEADER_BUFFER_SIZE 1024
#define WSGI_DEFAULT_WORKER_CONNECTIONS 16

#define WSGI_LOG_SOURCE_HTTP        1 << 15


#include <wsgi_event.h>
#include <wsgi_http_connection.h>
#include <wsgi_http_module.h>
#include <wsgi_http_request.h>

#endif /* _WSGI_HTTP_H_INCLUDED_ */
