#ifndef _WSGI_CORE_H_INCLUDED_
#define _WSGI_CORE_H_INCLUDED_


typedef struct wsgi_log_s wsgi_log_t;
typedef struct wsgi_gc_s wsgi_gc_t;
typedef struct wsgi_list_s wsgi_list_t;
typedef struct wsgi_pool_s wsgi_pool_t;
typedef struct wsgi_config_def_s wsgi_config_def_t;
typedef struct wsgi_config_s wsgi_config_t;
typedef struct wsgi_config_option_s wsgi_config_option_t;
typedef struct wsgi_cycle_s wsgi_cycle_t;
typedef struct wsgi_module_s wsgi_module_t;
typedef struct wsgi_socket_s wsgi_socket_t;


#define WSGI_OK                     0
#define WSGI_ERROR                  -1

#define CR                          (u_char) 13
#define LF                          (u_char) 10

#define WSGI_LOG_SOURCE_CORE        1 << 0
#define WSGI_LOG_SOURCE_ALLOC       1 << 1
#define WSGI_LOG_SOURCE_GC          1 << 2
#define WSGI_LOG_SOURCE_LIST        1 << 3
#define WSGI_LOG_SOURCE_POOL        1 << 4
#define WSGI_LOG_SOURCE_CONFIG      1 << 5
#define WSGI_LOG_SOURCE_SOCKET      1 << 6


#include <wsgi_os.h>

#include <wsgi_addr.h>
#include <wsgi_alloc.h>
#include <wsgi_config.h>
#include <wsgi_cycle.h>
#include <wsgi_gc.h>
#include <wsgi_list.h>
#include <wsgi_log.h>
#include <wsgi_pool.h>
#include <wsgi_signal.h>
#include <wsgi_socket.h>

#endif /* _WSGI_CORE_H_INCLUDED_ */
