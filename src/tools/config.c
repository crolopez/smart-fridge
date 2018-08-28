#include <yaml.h>
#include "log.h"
#include "config.h"

static int sf_load_reader_config(reader_conf **config, yaml_parser_t *parser);
static int sf_load_db_config(db_conf **config, yaml_parser_t *parser);

// Control structures
/* scalar_converter.type
    0 -> string
    1 -> int */
typedef struct scalar_converter {
    void **data;
    char type;
} scalar_converter;

typedef enum global_section {
    global_sect,
    reader_sect
} global_section;

typedef enum reader_section {
    read_sect,
    id_sect,
    pr_sect,
    test_sect,
    coder_sect,
    rlogs_sect,
    socket_mode
} reader_section;

typedef enum database_section {
    ports,
    headers,
    addresses,
    dlogs_sect
} database_section;

typedef enum yaml_state {
    expect_key,
    expect_value,
    expect_block_mapping,
    expect_nothing
} yaml_state;

// Blocks
static const char * YAML_READER = "reader";
static const char * YAML_DATABASE = "database";
// Reader block
static const char * YAML_ID_QUEUE = "id_queue";
static const char * YAML_PROD_QUEUE = "product_queue";
static const char * YAML_TEST_MODE = "test_mode";
static const char * YAML_CODE_READER = "code_reader";
static const char * YAML_SLEEP = "sleep";
static const char * YAML_SIZE = "size";
static const char * YAML_INFINITE = "infinite";
static const char * YAML_DEVICE = "device";
static const char * YAML_PORT = "port";
static const char * YAML_SOCKET_MODE = "socket_mode";
// Database Blocks
static const char * YAML_PORTS = "ports";
static const char * YAML_ADDRESSES = "addresses";
static const char * YAML_HEADERS = "headers";
static const char * YAML_INTERNAL = "internal";
// Generic blocks
static const char * YAML_LOGS = "logs";
static const char * YAML_LOCATION = "location";

int sf_read_config(n_config type, void **config) {
    FILE *fp = NULL;
    yaml_parser_t *parser = NULL;
    yaml_token_t  token;
    int retval = 1;
    char out = 0;
    yaml_state state;
    char *scalar;

    *config = NULL;

    if (fp = fopen(CONFIG_FILE, "r"), !fp) {
        goto clean;
    }

    parser = calloc(1, sizeof(yaml_parser_t));

    yaml_parser_initialize(parser);
    yaml_parser_set_input_file(parser, fp);

    while (!out && yaml_parser_scan(parser, &token)) {

        switch(token.type) {
            case YAML_KEY_TOKEN:
                state = expect_key;
                break;
            case YAML_SCALAR_TOKEN:
                scalar = token.data.scalar.value;
                if (state == expect_key) {
                    if (!strcmp(scalar, YAML_READER)) {
                        yaml_token_delete(&token);
                        if (type == N_READER) {
                            if (sf_load_reader_config((reader_conf **)config, parser)) {
                                out = 1;
                            }
                            out = 1;
                        }
                    } else if (!strcmp(scalar, YAML_DATABASE)) {
                        yaml_token_delete(&token);
                        if (type == N_DB) {
                            if (sf_load_db_config((db_conf **)config, parser)) {
                                goto clean;
                            }
                            out = 1;
                        }
                    }
                }
                state = expect_nothing;
                break;
            default:
                state = expect_nothing;
            break;
       }

        yaml_token_delete(&token);
    }

    yaml_token_delete(&token);
    retval = 0;
clean:
    yaml_parser_delete(parser);
    free(parser);
    if (fp) {
        fclose(fp);
    }
    return retval;
}

int sf_load_reader_config(reader_conf **config, yaml_parser_t *parser) {
    yaml_token_t  token;
    char *scalar;
    scalar_converter conv;
    reader_section sect;
    yaml_state state;
    int retval = 1;
    char out = 0;

    *config = calloc(1, sizeof(reader_conf));
    memset(&conv, 0, sizeof(scalar_converter));

    state = expect_block_mapping;
    conv.data = NULL;

    while (!out && yaml_parser_scan(parser, &token)) {
        switch(token.type) {
            case YAML_KEY_TOKEN:
                state = expect_key;
                break;
            case YAML_VALUE_TOKEN:
                if (state != expect_block_mapping) {
                    state = expect_value;
                }
                break;
            case YAML_BLOCK_MAPPING_START_TOKEN:
                if (state != expect_block_mapping) {
                    sf_error(INV_CONFIG_IN_BLOCK, YAML_READER);
                    goto clean;
                }
                break;
            case YAML_SCALAR_TOKEN:
                scalar = token.data.scalar.value;
                if (state == expect_key) {
                    if (!strcmp(scalar, YAML_SIZE)) {
                        if (sect == id_sect) {
                            conv.data = (void **)&(*config)->id_size;
                        } else if (sect == pr_sect) {
                            conv.data = (void **)&(*config)->pr_size;
                        }
                        conv.type = 1;
                    } else if (!strcmp(scalar, YAML_SLEEP)) {
                        if (sect == id_sect) {
                            conv.data = (void **)&(*config)->id_sleep;
                        } else if (sect == pr_sect) {
                            conv.data = (void **)&(*config)->pr_sleep;
                        } else if (sect == test_sect) {
                            conv.data = (void **)&(*config)->test_sleep;
                        }
                        conv.type = 1;
                    } else if (!strcmp(scalar, YAML_INFINITE)) {
                        conv.data = (void **)&(*config)->test_repeat;
                        conv.type = 0;
                    } else if (!strcmp(scalar, YAML_DEVICE)) {
                        conv.data = (void **)&(*config)->device;
                        conv.type = 0;
                    } else if (!strcmp(scalar, YAML_PORT)) {
                        conv.data = (void **)&(*config)->socket_mode_port;
                        conv.type = 1;
                    } else if (!strcmp(scalar, YAML_LOCATION)) {
                        conv.data = (void **)&(*config)->log_location;
                        conv.type = 0;
                    } else if (!strcmp(scalar, YAML_ID_QUEUE)) {
                        sect = id_sect;
                        state = expect_block_mapping;
                    } else if (!strcmp(scalar, YAML_PROD_QUEUE)) {
                        sect = pr_sect;
                        state = expect_block_mapping;
                    } else if (!strcmp(scalar, YAML_TEST_MODE)) {
                        sect = test_sect;
                        state = expect_block_mapping;
                    } else if (!strcmp(scalar, YAML_CODE_READER)) {
                        sect = coder_sect;
                        state = expect_block_mapping;
                    } else if (!strcmp(scalar, YAML_LOGS)) {
                        sect = rlogs_sect;
                        state = expect_block_mapping;
                    } else if (!strcmp(scalar, YAML_SOCKET_MODE)) {
                        sect = socket_mode;
                        state = expect_block_mapping;
                    } else {
                        sf_error(INV_CONFIG_TAG_BLOCK, scalar, YAML_READER);
                        goto clean;
                    }
                } else {
                    if (state == expect_value) {
                        if (conv.data) {
                            if (!conv.type) {
                                *conv.data = strdup(scalar);
                            } else if (conv.type == 1) {
                                int *integer = (int *) conv.data;
                                *integer = strtol(scalar, NULL, 10);
                            }
                            conv.data = NULL;
                        }
                    }
                }
                break;
            default:
            break;
       }

        if (token.type == YAML_BLOCK_END_TOKEN) {
            if (sect != read_sect) {
                sect = read_sect;
            } else {
                out = 1;
            }
        }
        yaml_token_delete(&token);
    }

    retval = 0;
clean:
    yaml_token_delete(&token);
    return retval;
}


int sf_load_db_config(db_conf **config, yaml_parser_t *parser) {
    yaml_token_t  token;
    char *scalar;
    scalar_converter conv;
    database_section sect;
    yaml_state state;
    int retval = 1;
    char out = 0;

    *config = calloc(1, sizeof(db_conf));
    memset(&conv, 0, sizeof(scalar_converter));

    state = expect_block_mapping;

    while (!out && yaml_parser_scan(parser, &token)) {
        switch(token.type) {
            case YAML_KEY_TOKEN:
                state = expect_key;
                break;
            case YAML_VALUE_TOKEN:
            if (state != expect_block_mapping) {
                state = expect_value;
            }
                break;
            case YAML_BLOCK_MAPPING_START_TOKEN:
                if (state != expect_block_mapping) {
                    sf_error(INV_CONFIG_IN_BLOCK, YAML_DATABASE);
                    goto clean;
                }
                break;
            case YAML_SCALAR_TOKEN:
                scalar = token.data.scalar.value;
                if (state == expect_key) {
                    if (!strcmp(scalar, YAML_INTERNAL)) {
                        if (sect == ports) {
                            conv.data = (void **)&(*config)->internal_port;
                            conv.type = 1;
                        } else if (sect == headers) {
                            conv.data = (void **)&(*config)->internal_header;
                            conv.type = 2;
                        } else if (sect == addresses) {
                            conv.data = (void **)&(*config)->internal_address;
                            conv.type = 2;
                        }
                        sect = -1;
                    } else if (!strcmp(scalar, YAML_LOCATION)) {
                        conv.data = (void **)&(*config)->log_location;
                        conv.type = 2;
                    } else if (!strcmp(scalar, YAML_PORTS)) {
                        sect = ports;
                        state = expect_block_mapping;
                    } else if (!strcmp(scalar, YAML_HEADERS)) {
                        sect = headers;
                        state = expect_block_mapping;
                    } else if (!strcmp(scalar, YAML_ADDRESSES)) {
                        sect = addresses;
                        state = expect_block_mapping;
                    } else if (!strcmp(scalar, YAML_LOGS)) {
                        sect = dlogs_sect;
                        state = expect_block_mapping;
                    } else {
                        sf_error(INV_CONFIG_TAG_BLOCK, scalar, YAML_DATABASE);
                        goto clean;
                    }
                } else {
                    if (state == expect_value) {
                        if (conv.type == 1) {
                            *conv.data = (void *) strtol(scalar, NULL, 10);
                        } else if (conv.type == 2) {
                            *conv.data = strdup(scalar);
                        }
                    }
                }
                break;
            default:
            break;
       }

        if (token.type == YAML_BLOCK_END_TOKEN) {
            if (sect != read_sect) {
                sect = read_sect;
            } else {
                out = 1;
            }
        }
        yaml_token_delete(&token);
    }

    retval = 0;
clean:
    yaml_token_delete(&token);
    return retval;
}

void free_reader_conf(reader_conf *conf) {
    if (conf) {
        free(conf->test_repeat);
        free(conf->log_location);
        free(conf);
    }
}

void free_db_conf(db_conf *conf) {
    if (conf) {
        free(conf->internal_header);
        free(conf->internal_address);
        free(conf->log_location);
        free(conf);
    }
}
