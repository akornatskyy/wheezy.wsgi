#ifndef WSGI_HTTP_REQUEST_H
#define WSGI_HTTP_REQUEST_H


struct wsgi_http_header_pair_s {
    u_char                  *name;
    u_char                  *value;
};

struct wsgi_http_request_s {
    wsgi_connection_t       *connection;
    u_char                  *buffer_start;
    u_char                  *buffer_pos;
    u_char                  *buffer_last;
    u_char                  *buffer_end;
    int                     (*handle_read)(wsgi_http_request_t *r);

    u_char                  *method;
    u_char                  *path_info;
    u_char                  *query_string;
    u_char                  *protocol;

    u_char                  *content_type;
    u_char                  *content_length;

    wsgi_list_t             headers;

    u_char                  state;
};

struct wsgi_http_runtime_s {
    void                    *self;
    int                     (*load)(void *self);
    int                     (*process)(void *self, wsgi_http_request_t *r);
    int                     (*unload)(void *self);
};


wsgi_http_request_t *
wsgi_http_request_create(wsgi_connection_t *c);

#endif /* WSGI_HTTP_REQUEST_H */
