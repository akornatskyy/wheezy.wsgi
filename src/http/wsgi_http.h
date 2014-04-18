#ifndef WSGI_HTTP_H
#define WSGI_HTTP_H


typedef struct wsgi_http_config_s wsgi_http_config_t;
typedef struct wsgi_http_ctx_s wsgi_http_ctx_t;
typedef struct wsgi_http_runtime_s wsgi_http_runtime_t;
typedef struct wsgi_http_header_pair_s wsgi_http_header_pair_t;
typedef struct wsgi_http_request_s wsgi_http_request_t;
typedef struct wsgi_http_server_config_s wsgi_http_server_config_t;


#define WSGI_CONFIG_DEF_SERVER 0x20

#define WSGI_DEFAULT_REQUEST_HEADERS_CAPACITY 12
#define WSGI_DEFAULT_REQUEST_HEADER_BUFFER_SIZE 1024
#define WSGI_DEFAULT_WORKER_CONNECTIONS 16

#define WSGI_LOG_SOURCE_HTTP        1 << 15


#include <wsgi_event.h>
#include <wsgi_http_connection.h>
#include <wsgi_http_module.h>
#include <wsgi_http_request.h>

#endif /* WSGI_HTTP_H */
