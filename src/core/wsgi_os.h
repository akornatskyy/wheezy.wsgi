#ifndef WSGI_OS_H
#define WSGI_OS_H


#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>


#define WSGI_LISTEN_BACKLOG     128

#define wsgi_sleep(s)           sleep(s)
#define wsgi_msleep(ms)         usleep(ms * 1000)

#endif /* WSGI_OS_H */
