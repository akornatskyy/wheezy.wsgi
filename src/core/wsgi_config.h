#ifndef WSGI_CONFIG_H
#define WSGI_CONFIG_H


#define WSGI_CONFIG_DEF_SEQUENCE    0x01
#define WSGI_CONFIG_DEF_ROOT        0x10


struct wsgi_config_def_s {
    const char          * const name;
    const u_int         flags;
    int                 (* const setup)(wsgi_config_t *c,
                                        wsgi_config_option_t *o);
};

struct wsgi_config_s {
    const u_int         root_block;
    const void          *adapter;
    int                 (*const lookup)(const void *adapter,
                                        wsgi_config_option_t *o);
    const wsgi_log_t    *log;
};

struct wsgi_config_option_s {
    u_int                   block;
    const wsgi_config_def_t *def;
    const u_char            *value;
    void                    *ctx;
};

int wsgi_config_load(wsgi_config_t *c, const u_char *filename);

#endif /* WSGI_CONFIG_H */
