
#include <wsgi_core.h>


int main(int argc, char *argv[])
{
    wsgi_log_t *log;
    wsgi_gc_t *gc;

    log = wsgi_log_init();
    log->log_level = WSGI_LOG_DEBUG;
    log->log_source = WSGI_LOG_SOURCE_ALL;
    wsgi_log_set_source(WSGI_LOG_SOURCE_ALLOC, "alloc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_GC, "gc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_CORE, "core");

    gc = wsgi_create_gc(512, log);
    wsgi_gc_malloc(gc, 1024);
    wsgi_gc_malloc(gc, 485);
    wsgi_gc_malloc(gc, 10);
    wsgi_gc_malloc(gc, 485);

    wsgi_reset_gc(gc);
    wsgi_destroy_gc(gc);

    return 0;
}
