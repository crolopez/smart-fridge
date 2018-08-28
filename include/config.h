#ifndef SF_CONFIG
#define SF_CONFIG

#define CONFIG_FILE "./config.yaml"

typedef enum n_config {
    N_READER,
    N_DB
} n_config;

typedef struct reader_conf {
    int id_sleep;
    int id_size;
    int pr_sleep;
    int pr_size;
    int test_sleep;
    char *test_repeat;
    char *device;
    char *log_location;
} reader_conf;

typedef struct db_conf {
    unsigned int internal_port;
    char *internal_header;
    char *internal_address;
    char *log_location;
} db_conf;

int sf_read_config(n_config type, void **config);
void free_reader_conf(reader_conf *conf);
void free_db_conf(db_conf *conf);

#endif
