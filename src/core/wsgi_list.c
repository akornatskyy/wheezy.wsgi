
#include <wsgi_core.h>


wsgi_list_t *
wsgi_list_create(wsgi_gc_t *gc, u_int capacity, size_t size)
{
    wsgi_list_t *list;

    list = wsgi_gc_malloc(gc, sizeof(wsgi_list_t));
    if (list == NULL) {
        return NULL;
    }

    if (wsgi_list_init(list, gc, capacity, size) != WSGI_OK) {
        return NULL;
    }

    return list;
}


int
wsgi_list_init(wsgi_list_t *list, wsgi_gc_t *gc, u_int capacity, size_t size)
{
    list->items = wsgi_gc_malloc(gc, capacity * size);
    if (list->items == NULL) {
        return WSGI_ERROR;
    }

    list->size = size;
    list->length = 0;
    list->capacity = capacity;
    list->gc = gc;

    wsgi_log_debug(gc->log, WSGI_LOG_SOURCE_LIST,
                   "init: %p, capacity: %d, size: %d",
                   list, capacity, size);

    return WSGI_OK;
}


void *
wsgi_list_append(wsgi_list_t *list)
{
    u_char *p;
    size_t size;
    wsgi_gc_block_t *b;

    if (list->length == list->capacity) {

        size = list->capacity * list->size;
        b = list->gc->current;

        if ((u_char *) list->items + size == b->current && b->left >= size)
        {
            wsgi_log_debug(list->gc->log, WSGI_LOG_SOURCE_LIST,
                           "extend: %p, block: %p, size: %d",
                           list, b, size);
            b->current += size;
            b->left -= size;
        }
        else
        {
            p = wsgi_gc_malloc(list->gc, size * 2);
            if (p == NULL) {
                return NULL;
            }

            if ((u_char *) list->items + size == b->current)
            {
                wsgi_log_debug(list->gc->log, WSGI_LOG_SOURCE_LIST,
                               "release: %p, block: %p, size: %d",
                               list, b, size);
                b->current -= size;
                b->left += size;
                b->fails--;
            }

            wsgi_log_debug(list->gc->log, WSGI_LOG_SOURCE_LIST,
                           "realloc: %p, items: %p",
                           list, p);

            list->items = memcpy(p, list->items, size);
        }

        list->capacity *= 2;
    }

    p = (u_char *) list->items + list->size * list->length++;

    wsgi_log_debug(list->gc->log, WSGI_LOG_SOURCE_LIST,
                   "append: %p, item: %p",
                   list, p);

    return p;
}
