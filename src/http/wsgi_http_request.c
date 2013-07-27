
#include <wsgi_http.h>


static int wsgi_http_request_parse_request_line(wsgi_http_request_t *r);
static int wsgi_http_request_parse_headers(wsgi_http_request_t *r);
static int wsgi_http_request_process(wsgi_http_request_t *r);


wsgi_http_request_t *
wsgi_http_request_create(wsgi_gc_t *gc, u_int buffer_size)
{
    u_char *b;
    wsgi_http_request_t *r;

    b = wsgi_gc_malloc_ref(gc, buffer_size);
    if (b == NULL) {
        return NULL;
    }

    r = wsgi_gc_calloc(gc, sizeof(wsgi_http_request_t));
    if (r == NULL) {
        return NULL;
    }

    r->log = gc->log;
    r->buffer_start = r->buffer_last = r->buffer_pos = b;
    r->buffer_end = b + buffer_size;
    r->handle_read = wsgi_http_request_parse_request_line;

    return r;
}


static int wsgi_http_request_parse_request_line(wsgi_http_request_t *r)
{
    u_char *p;
    enum {
        START = 0, METHOD, SPACE1, URI, SPACE2, VERSION, END
    } state;

    wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                   "request: %p, parsing request line",
                   r);

    state = r->state;
    for (p = r->buffer_pos; p < r->buffer_last; p++) {
        switch (state) {
            case START:
                r->method = p; state = METHOD;
                break;

            case METHOD:
                if (*p == ' ') { *p = '\0'; state = SPACE1; }
                break;

            case SPACE1:
                r->uri = p; state = URI;
                break;

            case URI:
                if (*p == ' ') { *p = '\0'; state = SPACE2; }
                break;

            case SPACE2:
                r->version = p; state = VERSION;
                break;

            case VERSION:
                if (*p == CR) { *p = '\0'; state = END; }
                break;

            case END:
                if (*p == LF) goto done;
                return WSGI_ERROR;
        }
    }

    r->buffer_pos = p;
    r->state = state;

    return WSGI_OK;

done:

    r->buffer_pos = ++p;
    r->handle_read = wsgi_http_request_parse_headers;

    return wsgi_http_request_parse_headers(r);
}


static int wsgi_http_request_parse_headers(wsgi_http_request_t *r)
{
    u_char *p, *name, *name_end, *value, *value_end, *pos;
    enum {
        START = 0, NAME, COLON, SPACE, VALUE, END, DONE
    } state;

    wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                   "request: %p, parsing headers",
                   r);

    name = name_end = value = value_end = NULL;
    state = START;
    for (p = pos = r->buffer_pos; p < r->buffer_last; p++) {
        switch (state) {
            case START:
                if (*p == CR) {
                    state = DONE;
                }
                else {
                    name = pos = p; state = NAME;
                }

                break;

            case NAME:
                if (*p == ':') { name_end = p; state = COLON; }
                break;

            case COLON:
                if (*p != ' ') {
                    wsgi_log_error(r->log, WSGI_LOG_SOURCE_HTTP,
                                   "header: expecting space after colon, "
                                   "%#x found", *p);
                    return WSGI_ERROR;
                }

                state = SPACE;
                break;

            case SPACE:
                if (*p != ' ') { value = p; state = VALUE; }
                break;

            case VALUE:
                if (*p == CR) { value_end = p; state = END; }
                break;

            case END:
                if (*p != LF) {
                    wsgi_log_error(r->log, WSGI_LOG_SOURCE_HTTP,
                                   "header: expecting LF after CR, "
                                   "%#x found", *p);
                    return WSGI_ERROR;
                }

                *name_end = '\0'; *value_end = '\0';
                wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                               "  %s: %s",
                               name, value);
                state = START;
                break;

            case DONE:
                if (*p == LF) goto done;
                wsgi_log_error(r->log, WSGI_LOG_SOURCE_HTTP,
                               "header: expecting LF after CR, "
                               "%#x found", *p);
                return WSGI_ERROR;
        }
    }

    r->buffer_pos = pos;

    return WSGI_OK;

done:
    r->handle_read = wsgi_http_request_process;
    return wsgi_http_request_process(r);
}


static int wsgi_http_request_process(wsgi_http_request_t *r)
{
    wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                   "request: %p, processing",
                   r);
    return WSGI_OK;
}
