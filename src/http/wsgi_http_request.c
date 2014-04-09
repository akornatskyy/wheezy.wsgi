
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

    if (wsgi_list_init(&r->headers, gc,
                       WSGI_DEFAULT_REQUEST_HEADERS_CAPACITY,
                       sizeof(wsgi_http_header_pair_t)) != WSGI_OK) {
        return NULL;
    }

    return r;
}


static int
wsgi_http_request_parse_request_line(wsgi_http_request_t *r)
{
    u_char *p;
    enum {
        START = 0, METHOD, SPACE1, URI, MARK, QUERY, SPACE2, PROTOCOL, END
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
                if (*p == ' ') {
                    *p = '\0'; state = SPACE1;
                    wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                                   "  REQUEST_METHOD: %s",
                                   r->method);
                }

                break;

            case SPACE1:
                r->path_info = p; state = URI;
                break;

            case URI:
                switch (*p) {
                    case '?':
                        *p = '\0'; state = MARK;
                        break;
                    case ' ':
                        r->query_string = (u_char *) "";
                        *p = '\0'; state = SPACE2;
                        break;
                }

                break;

            case MARK:
                r->query_string = p; state = QUERY;
                break;

            case QUERY:
                if (*p == ' ') {
                    *p = '\0'; state = SPACE2;
                }

                break;

            case SPACE2:
                wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                               "  PATH_INFO: %s, QUERY_STRING: %s",
                               r->path_info, r->query_string);
                r->protocol = p; state = PROTOCOL;
                break;

            case PROTOCOL:
                if (*p == CR) {
                    *p = '\0'; state = END;
                    wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                                   "  SERVER_PROTOCOL: %s",
                                   r->protocol);
                }

                break;

            case END:
                if (*p == LF) goto done;
                wsgi_log_error(r->log, WSGI_LOG_SOURCE_HTTP,
                               "request line: expecting LF after CR, "
                               "%#x found", *p);
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


static int
wsgi_http_request_parse_headers(wsgi_http_request_t *r)
{
    u_char *p, *name, *name_end, *value, *value_end, *pos;
    wsgi_http_header_pair_t *h;
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
                state = START;

                h = wsgi_list_append(&r->headers);
                h->name = name; h->value = value;

                switch (name_end - name) {
                    case 12:
                        if (*name == 'C' && *(name + 9) == 'y') {
                            r->content_type = value;
                        }

                        break;
                    case 14:
                        if (*name == 'C' && *(name + 9) == 'e') {
                            r->content_length = value;
                        }

                        break;
                }

                wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                               "  %s: %s",
                               name, value);
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

#if WSGI_DEBUG
    if (r->content_length || r->content_type) {
        wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                       "CONTENT_LENGTH: %s, CONTENT_TYPE: %s",
                       r->content_length, r->content_type);
    }
#endif

    r->handle_read = wsgi_http_request_process;

    return wsgi_http_request_process(r);
}


static int
wsgi_http_request_process(wsgi_http_request_t *r)
{
    wsgi_log_debug(r->log, WSGI_LOG_SOURCE_HTTP,
                   "request: %p, processing",
                   r);
    return WSGI_OK;
}
