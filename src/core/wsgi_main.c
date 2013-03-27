
#include <wsgi_core.h>


static void check_list(const wsgi_list_t *l);
static void test_list(const wsgi_log_t *log);


int main(int argc, char *argv[])
{
    wsgi_log_t *log;

    log = wsgi_log_init();
    log->log_level = WSGI_LOG_DEBUG;
    log->log_source = WSGI_LOG_SOURCE_ALL;
    wsgi_log_set_source(WSGI_LOG_SOURCE_ALLOC, "alloc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_GC, "gc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_CORE, "core");
    wsgi_log_set_source(WSGI_LOG_SOURCE_LIST, "list");

    test_list(log);

    return 0;
}


static void test_list(const wsgi_log_t *log)
{
    u_int i;
    u_int *item;
    wsgi_gc_t *gc;
    wsgi_list_t *l;
    wsgi_list_t d;

    gc = wsgi_gc_create(144, log);

    l = wsgi_list_create(gc, 4, sizeof(u_int));
    wsgi_list_init(&d, gc, 4, sizeof(u_int));

    for (i = 0; i < 17; i++) {
        item = wsgi_list_append(l);
        if (item == NULL) {
            break;
        }

        *item = i;

        item = wsgi_list_append(&d);
        if (item == NULL) {
            break;
        }

        *item = i;
    }

    check_list(l);
    check_list(&d);

    wsgi_gc_destroy(gc);
}


static void check_list(const wsgi_list_t *l)
{
    u_int i;
    const u_int *items = (u_int *)l->items;
    for(i = 0; i < l->length; i++) {
        assert (items[i] == i);
    }
}
