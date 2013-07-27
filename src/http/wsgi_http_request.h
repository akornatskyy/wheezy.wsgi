#ifndef _WSGI_HTTP_REQUEST_H_INCLUDED_
#define _WSGI_HTTP_REQUEST_H_INCLUDED_


struct wsgi_http_request_s {
    const wsgi_log_t        *log;
    u_char                  *buffer_start;
    u_char                  *buffer_pos;
    u_char                  *buffer_last;
    u_char                  *buffer_end;
    int                     (*handle_read)(wsgi_http_request_t *r);

    u_char                  state;

    u_char                  *method;
    u_char                  *uri;
    u_char                  *version;
};


wsgi_http_request_t *
wsgi_http_request_create(wsgi_gc_t *gc, u_int buffer_size);

#endif /* _WSGI_HTTP_REQUEST_H_INCLUDED_ */