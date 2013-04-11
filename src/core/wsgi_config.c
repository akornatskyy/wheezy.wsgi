
#include <wsgi_core.h>
#include <yaml.h>


static int wsgi_config_yaml_parse(
    wsgi_config_t *c, yaml_parser_t *parser, u_int block);

static void wsgi_config_yaml_parse_error(yaml_parser_t *parser,
    const wsgi_log_t *log);


int wsgi_config_load(wsgi_config_t *c, const u_char *filename)
{
    u_int s;
    FILE *f;
    yaml_parser_t parser;

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
                   "loading \"%s\"",
                   filename);
    f = fopen((const char *) filename, "r");
    if (f == NULL) {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "open \"%s\" failed",
                       filename);
        return WSGI_ERROR;
    }

    if (!yaml_parser_initialize(&parser))
    {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                       "yaml parser initialization failed",
                       filename);
        return WSGI_ERROR;
    }

    yaml_parser_set_input_file(&parser, f);

    s = wsgi_config_yaml_parse(c, &parser, c->root_block);

    fclose(f);
    yaml_parser_delete(&parser);

    if (s != WSGI_OK) {
        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
            "load \"%s\" failed",
            filename);
        return WSGI_ERROR;
    }

    wsgi_log_debug(c->log, WSGI_LOG_SOURCE_CONFIG,
        "\"%s\" loaded",
        filename);

    return WSGI_OK;
}


static int wsgi_config_yaml_parse(
    wsgi_config_t *c, yaml_parser_t *parser, u_int block)
{
    u_int done, key_found;
    yaml_event_t event;
    wsgi_config_option_t o;

    done = key_found = 0;
    o.block = block;

    while (!done) {
        if (!yaml_parser_parse(parser, &event)) {
            wsgi_config_yaml_parse_error(parser, c->log);
            return WSGI_ERROR;
        }

        switch (event.type) {
            case YAML_MAPPING_START_EVENT:
                if (key_found) {
                    if (o.def->setup(c, &o) != WSGI_OK) {
                        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                            "unable to add a sequence item "
                            "at line %d, column %d",
                            event.start_mark.line + 1,
                            event.start_mark.column + 1);
                        goto failed;
                    }

                    if (o.block == block) {
                        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                            "undefined sequence block for \"%s\"",
                            o.def->name);
                        goto failed;
                    }

                    if (wsgi_config_yaml_parse(c, parser, o.block)
                            != WSGI_OK) {
                        goto failed;
                    }

                    o.block = block;
                }

                break;

            case YAML_SEQUENCE_START_EVENT:
                if (key_found
                    && !(o.def->flags & WSGI_CONFIG_DEF_SEQUENCE)) {
                    wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                        "expected scalar at line %d, column %d",
                        event.start_mark.line + 1,
                        event.start_mark.column + 1);
                    goto failed;
                }

                break;

            case YAML_SCALAR_EVENT:
                if (key_found) {
                    if (o.def->flags & WSGI_CONFIG_DEF_SEQUENCE) {
                        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                            "expected sequence at line %d, column %d",
                            event.start_mark.line + 1,
                            event.start_mark.column + 1);
                        goto failed;
                    }

                    o.value = event.data.scalar.value;
                    if (o.def->setup(c, &o) != WSGI_OK) {
                        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                            "unable to set scalar value \"%s\" "
                            "at line %d, column %d",
                            event.data.scalar.value,
                            event.start_mark.line + 1,
                            event.start_mark.column + 1);
                        goto failed;
                    }

                    key_found = 0;
                }
                else {
                    o.value = event.data.scalar.value;
                    if (c->lookup(c->adapter, &o) != WSGI_OK) {
                        wsgi_log_error(c->log, WSGI_LOG_SOURCE_CONFIG,
                            "unexpected \"%s\" at line %d, column %d",
                            event.data.scalar.value,
                            event.start_mark.line + 1,
                            event.start_mark.column + 1);
                        goto failed;
                    }

                    key_found = 1;
                }

                break;

            case YAML_SEQUENCE_END_EVENT:
                key_found = 0;
                break;

            case YAML_MAPPING_END_EVENT:
                done = 1;
                break;

            default:
                break;
        }

        o.value = NULL;
        yaml_event_delete(&event);
    }

    return WSGI_OK;

failed:
    yaml_event_delete(&event);
    return WSGI_ERROR;
}


static void wsgi_config_yaml_parse_error(
        yaml_parser_t *parser, const wsgi_log_t *log)
{
    switch (parser->error)
    {
        case YAML_MEMORY_ERROR:
            wsgi_log_error(log, WSGI_LOG_SOURCE_CONFIG,
                "not enough memory for parsing");
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
                "unknown internal parser error");
            break;
    }
}
