#ifndef SF_CONFIG
#define SF_CONFIG

#define CONFIG_FILE "./config.yaml"

typedef enum n_config {
    N_APP,
    N_READER,
    N_DB,
    N_CONNECTIONS
} n_config;

typedef struct reader_conf {
    int id_sleep;
    int id_size;
    int pr_sleep;
    int pr_size;
    int test_sleep;
    char *test_repeat;
} reader_conf;

typedef struct connections_conf {
    unsigned int internal_port;
    char *internal_header;
    char *internal_address;
} connections_conf;

int sf_read_config(n_config type, void **config);
void free_reader_conf(reader_conf *conf);
void free_connection_conf(connections_conf *conf);

#endif
