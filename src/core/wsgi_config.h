#ifndef _WSGI_CONFIG_H_INCLUDED_
#define _WSGI_CONFIG_H_INCLUDED_


#define WSGI_DEBUG 1

typedef enum {
    WSGI_SCALAR,
    WSGI_SEQUENCE,
    WSGI_MAPPING
} wsgi_config_option_type_t;


struct wsgi_config_option_s {
    wsgi_config_option_type_t type;

    union {
        struct {
            u_int length;
            u_char *value;
        } scalar;

        struct {
            u_char *key;
            wsgi_config_option_t *value;
        } mapping;

        struct {
            u_int length;
            wsgi_config_option_t **value;
        } sequence;
    } d;
};

struct wsgi_config_s {
    wsgi_gc_t               *gc;
    u_char*                 filename;
    wsgi_config_option_t    **options;
    u_int                   options_length;
};


int wsgi_config_load(wsgi_config_t *c);

#define wsgi_config_key(o) o->d.mapping.key
#define wsgi_config_scalar(o) o->d.mapping.value->d.scalar.value
#define wsgi_config_sequence(o) o->d.mapping.value->d.sequence.value

#endif /* _WSGI_CONFIG_H_INCLUDED_ */
