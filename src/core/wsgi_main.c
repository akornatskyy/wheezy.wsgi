
#include <wsgi_core.h>


static void check_list(const wsgi_list_t *l);
static void test_list(void);
static void test_config(void);

static wsgi_log_t *log;


int main(int argc, char *argv[])
{
    log = wsgi_log_init();
    log->log_level = WSGI_LOG_DEBUG;
    log->log_source = WSGI_LOG_SOURCE_CORE
        | WSGI_LOG_SOURCE_ALLOC
        | WSGI_LOG_SOURCE_GC
        | WSGI_LOG_SOURCE_LIST;
    wsgi_log_set_source(WSGI_LOG_SOURCE_ALLOC, "alloc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_GC, "gc");
    wsgi_log_set_source(WSGI_LOG_SOURCE_CORE, "core");
    wsgi_log_set_source(WSGI_LOG_SOURCE_LIST, "list");

    test_list();
    test_config();

    return 0;
}


static void test_config(void)
{
    u_int i;
    wsgi_gc_t *gc;
    wsgi_config_t *c;
    wsgi_config_option_t *o, *s;

    gc = wsgi_gc_create(512, log);
    c = wsgi_gc_malloc(gc, sizeof(wsgi_config_t));
    c->gc = gc;
    c->filename = (u_char *) "conf/wsgi.yaml";

    wsgi_config_load(c);

    s = c->options[0];
    wsgi_log_debug(log, WSGI_LOG_SOURCE_CORE, "%s: %s",
                   wsgi_config_key(s),
                   wsgi_config_scalar(s));
    s = c->options[1];
    wsgi_log_debug(log, WSGI_LOG_SOURCE_CORE, "%s:",
                   wsgi_config_key(s));
    s = wsgi_config_sequence(s)[0];
    wsgi_log_debug(log, WSGI_LOG_SOURCE_CORE, " %s: %s",
                   wsgi_config_key(s),
                   wsgi_config_scalar(s));

    s = c->options[2];
    wsgi_log_debug(log, WSGI_LOG_SOURCE_CORE, "%s:",
                   wsgi_config_key(s));
    s = wsgi_config_sequence(s)[0];
    wsgi_log_debug(log, WSGI_LOG_SOURCE_CORE, " %s: %s",
                   wsgi_config_key(s),
                   wsgi_config_scalar(s));

    s = c->options[2];
    s = wsgi_config_sequence(s)[1];
    wsgi_log_debug(log, WSGI_LOG_SOURCE_CORE, " %s",
                   s->d.mapping.key);
    for (i = 0; i < 2; i++) {
        o = wsgi_config_sequence(s)[i];
        assert(o);
        wsgi_log_debug(log, WSGI_LOG_SOURCE_CORE, "  %s: %s",
                       wsgi_config_key(o),
                       wsgi_config_scalar(o));
    }

    wsgi_gc_destroy(gc);
}


static void test_list(void)
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
