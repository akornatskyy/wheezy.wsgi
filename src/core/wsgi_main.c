
#include <wsgi_core.h>


int main(int argc, char *argv[])
{
    u_int i;
    wsgi_log_t *log;

    log = wsgi_log_init();
    log->log_level = WSGI_LOG_DEBUG;
    wsgi_log_set_source(WSGI_LOG_SOURCE_CORE, "core");

    for (i = 0; i < 10; i++) {
        wsgi_log_info(log, WSGI_LOG_SOURCE_CORE, "Hello %s!", "World");
        wsgi_sleep(1);
    }

    return 0;
}
