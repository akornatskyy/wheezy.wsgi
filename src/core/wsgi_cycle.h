#ifndef WSGI_CYCLE_H
#define WSGI_CYCLE_H


struct wsgi_cycle_s {
    const wsgi_log_t        *log;
    wsgi_gc_t               *gc;
    const u_char            *filename;
    void                    **ctx;
};

struct wsgi_module_s {
    const char              *name;
    u_int                   id;
    const wsgi_config_def_t *defs;
    void                    *(* const create)(wsgi_cycle_t *cycle);
    int                     (* const init)(void *self);
    int                     (* const shutdown)(void *self);
};


wsgi_cycle_t *wsgi_cycle_create(const wsgi_log_t* log);
void wsgi_cycle_destroy(wsgi_cycle_t *cycle);
int wsgi_cycle_init(wsgi_cycle_t *cycle);
int wsgi_cycle_shutdown(wsgi_cycle_t *cycle);

extern wsgi_module_t* modules[];
extern const u_int modules_count;

#endif /* WSGI_CYCLE_H */
