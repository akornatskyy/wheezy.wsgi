#ifndef _WSGI_CONFIG_H_INCLUDED_
#define _WSGI_CONFIG_H_INCLUDED_


#define WSGI_DEBUG                  1

#define WSGI_CONFIG_DEF_SEQUENCE    0x01
#define WSGI_CONFIG_DEF_ROOT        0x10


struct wsgi_config_def_s {
    const char          * const name;
    const u_int         flags;
    int                 (* const setup)(wsgi_config_t *c,
                                        wsgi_config_option_t *o);
};

struct wsgi_config_s {
    u_int               root_block;
    void                *adapter;
    int                 (*lookup)(void *adapter, wsgi_config_option_t *o);
    wsgi_log_t          *log;
};

struct wsgi_config_option_s {
    wsgi_log_t              *log;
    u_char                  *value;
    wsgi_config_def_t       *def;
    void                    *ctx;
    u_int                   block;
};

int wsgi_config_load(wsgi_config_t *c, const u_char *filename);

#endif /* _WSGI_CONFIG_H_INCLUDED_ */
