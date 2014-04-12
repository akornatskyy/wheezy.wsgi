#ifndef WSGI_CYCLE_H
#define WSGI_CYCLE_H


struct wsgi_cycle_s {
    wsgi_log_t              *log;
    wsgi_gc_t               *gc;
    u_char                  *filename;
    void                    **ctx;
};

struct wsgi_module_s {
    char                    *name;
    u_int                   id;
    wsgi_config_def_t       *defs;
    void                    *(*create)(wsgi_cycle_t *cycle);
    int                     (*init)(void *self);
    int                     (*shutdown)(void *self);
};


wsgi_cycle_t *wsgi_cycle_create(wsgi_log_t* log);
void wsgi_cycle_destroy(wsgi_cycle_t *cycle);
int wsgi_cycle_init(wsgi_cycle_t *cycle);
int wsgi_cycle_shutdown(wsgi_cycle_t *cycle);

extern wsgi_module_t* modules[];
extern const u_int modules_count;

#endif /* WSGI_CYCLE_H */
