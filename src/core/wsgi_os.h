#ifndef _WSGI_OS_H_INCLUDED_
#define _WSGI_OS_H_INCLUDED_


#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


#define wsgi_sleep(s)       sleep(s)
#define wsgi_msleep(ms)     usleep(ms * 1000)

#endif /* _WSGI_OS_H_INCLUDED_ */
