#ifndef _WSGI_OS_H_INCLUDED_
#define _WSGI_OS_H_INCLUDED_


#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>


#define WSGI_LISTEN_BACKLOG     128

#define wsgi_sleep(s)           sleep(s)
#define wsgi_msleep(ms)         usleep(ms * 1000)

#endif /* _WSGI_OS_H_INCLUDED_ */
