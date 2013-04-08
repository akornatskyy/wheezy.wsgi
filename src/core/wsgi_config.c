
#include <wsgi_core.h>
#include <yaml.h>


static wsgi_config_option_t *wsgi_config_make_mapping(
    wsgi_gc_t *gc, u_char* key, wsgi_config_option_t *value);

static wsgi_config_option_t *wsgi_config_make_scalar(
    wsgi_gc_t *gc, u_char* key, const u_char *value, u_int length);

static wsgi_config_option_t *wsgi_config_parse(
    wsgi_config_t *c, yaml_parser_t *parser);

static void wsgi_config_parser_error(yaml_parser_t *parser,
    const wsgi_log_t *log);


int wsgi_config_load(wsgi_config_t *c)
{
    FILE *f;
    yaml_parser_t parser;
    wsgi_config_option_t *o;

    wsgi_log_debug(c->gc->log, WSGI_LOG_SOURCE_CONFIG,
                   "loading \"%s\"",
                   c->filename);
    f = fopen((const char *) c->filename, "r");
    if (f == NULL) {
        wsgi_log_error(c->gc->log, WSGI_LOG_SOURCE_CONFIG,
                       "open \"%s\" failed",
                       c->filename);
        return WSGI_ERROR;
    }

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, f);
    o = wsgi_config_parse(c, &parser);
    fclose(f);
    yaml_parser_delete(&parser);

    if (o == NULL) {
        wsgi_log_error(c->gc->log, WSGI_LOG_SOURCE_CONFIG,
                       "load \"%s\" failed",
                       c->filename);
        return WSGI_ERROR;
    }

    c->options = o->d.sequence.value;
    c->options_length = o->d.sequence.length;

    wsgi_log_debug(c->gc->log, WSGI_LOG_SOURCE_CONFIG,
                   "\"%s\" loaded",
                   c->filename);
    return WSGI_OK;
}


static wsgi_config_option_t *wsgi_config_parse(
    wsgi_config_t *c, yaml_parser_t *parser)
{
    u_int done, state, length;
    u_char *name;
    wsgi_list_t l;
    yaml_event_t event;
    wsgi_gc_t *gc;
    wsgi_config_option_t **p, *o;

    done = state = 0;
    name = NULL;
    gc = c->gc;
    if (wsgi_list_init(&l, gc, 4, sizeof(wsgi_config_option_t *))
            == WSGI_ERROR) {
        return NULL;
    }

    while (!done) {
        if (!yaml_parser_parse(parser, &event)) {
            wsgi_config_parser_error(parser, gc->log);
            return NULL;
        }

        switch (event.type) {
            case YAML_SEQUENCE_START_EVENT:
            case YAML_MAPPING_START_EVENT:
                if (name) {
                    o = wsgi_config_parse(c, parser);
                    if (o == NULL) {
                        goto failed;
                    }

                    p = wsgi_list_append(&l);
                    if (p == NULL) {
                        goto failed;
                    }

                    o = wsgi_config_make_mapping(gc, name, o);
                    if (o == NULL) {
                        goto failed;
                    }

                    *p = o;
                    state = 0;
                }

                break;
            case YAML_SCALAR_EVENT:
                length = event.data.scalar.length;
                if (state) {
                    o = wsgi_config_make_scalar(
                        gc, name, event.data.scalar.value, length);
                    if (o == NULL) {
                        goto failed;
                    }

                    p = wsgi_list_append(&l);
                    if (p == NULL) {
                        goto failed;
                    }

                    *p = o;
                    state = 0;
                }
                else {
                    name = wsgi_gc_malloc(gc, length + 1);
                    if (name == NULL) {
                        goto failed;
                    }

                    memcpy(name, event.data.scalar.value, length + 1);
                    state = 1;
                }

                break;
            case YAML_STREAM_END_EVENT:
            case YAML_MAPPING_END_EVENT:
            case YAML_SEQUENCE_END_EVENT:
                done = 1;
                break;
            default:
                break;
        }

        yaml_event_delete(&event);
    }

    o = wsgi_gc_malloc(gc, sizeof(wsgi_config_option_t));
    if (o == NULL) {
        return NULL;
    }

    o->type = WSGI_SEQUENCE;
    o->d.sequence.value = l.items;
    o->d.sequence.length = l.length;
    return o;

failed:
    yaml_event_delete(&event);
    return NULL;
}


static wsgi_config_option_t *wsgi_config_make_scalar(
    wsgi_gc_t *gc, u_char* key, const u_char *value, u_int length)
{
    wsgi_config_option_t *o;
    u_char *p;

    o = wsgi_gc_malloc(gc, sizeof(wsgi_config_option_t));
    if (o == NULL) {
        return NULL;
    }

    p = wsgi_gc_malloc(gc, length + 1);
    if (p == NULL) {
        return NULL;
    }

    memcpy(p, value, length + 1);

    o->type = WSGI_SCALAR;
    o->d.scalar.value = p;
    o->d.scalar.length = length;

    return wsgi_config_make_mapping(gc, key, o);
}


static wsgi_config_option_t *wsgi_config_make_mapping(
    wsgi_gc_t *gc, u_char* key, wsgi_config_option_t *value)
{
    wsgi_config_option_t *o;

    o = wsgi_gc_malloc(gc, sizeof(wsgi_config_option_t));
    if (o == NULL) {
        return NULL;
    }

    o->type = WSGI_MAPPING;
    o->d.mapping.key = key;
    o->d.mapping.value = value;

    return o;
}


static void wsgi_config_parser_error(yaml_parser_t *parser,
                                     const wsgi_log_t *log)
{
    switch (parser->error)
    {
        case YAML_MEMORY_ERROR:
            wsgi_log_error(log, WSGI_LOG_SOURCE_CONFIG,
                "Not enough memory for parsing");
            break;

        case YAML_READER_ERROR:
            wsgi_log_error(log, WSGI_LOG_SOURCE_CONFIG,
                "%s at offset %d",
                parser->problem,
                parser->problem_offset);
            break;

        case YAML_PARSER_ERROR:
        case YAML_SCANNER_ERROR:
            if (parser->context) {
                wsgi_log_error(log, WSGI_LOG_SOURCE_CONFIG,
                    "%s at line %d, column %d",
                    parser->context,
                    parser->context_mark.line + 1,
                    parser->context_mark.column + 1);
            }

            wsgi_log_error(log, WSGI_LOG_SOURCE_CONFIG,
                "%s at line %d, column %d",
                parser->problem,
                parser->problem_mark.line + 1,
                parser->problem_mark.column + 1);
            break;

        default:
            wsgi_log_error(log, WSGI_LOG_SOURCE_CONFIG,
                "Unknown internal parser error");
            break;
    }
}
