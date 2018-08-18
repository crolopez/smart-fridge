#include <curl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#ifdef CAMERA_ENABLED
#include <zbar.h>
#endif
#include "config.h"
#include "bar.h"
#include "product_decoder.h"
#include "downloader.h"
#include "queue.h"
#include "log.h"
#include "sf_reader.h"

static queue *idq;
static queue *prq;
static char *test_mode_file = NULL;
static int test_mode_speed = 3;
static char *internal_header;
static int int_header_size;
static char *int_address;
static int int_port;
static int execution_ends = 0;
static char db_mode = DB_MODE_ADD;

product_node *sf_scan_code() {
    // Check MAX_ID_LEN here
    return NULL;
}

#ifdef CAMERA_ENABLED
void z_handler (zbar_image_t *im, const void *data) {
    product_node *node;
    const char *code;
    const zbar_symbol_t *symbol;

	symbol = zbar_image_first_symbol(im);
	for(; symbol; symbol = zbar_symbol_next(symbol)) {
		zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
		code = zbar_symbol_get_data(symbol);
		sf_debug2(ZBAR_DEBUG_MSG, zbar_get_symbol_name(typ), code);
        node = create_product_node(strdup(code), 1, db_mode);
        if (sf_queue_add(idq, node)) {
            sf_warn(FULL_QUEUE, node->code);
            free_product_node(node);
        }
    }
}
#endif

static void key_handler(int signum) {
    if (signum == SIGTSTP) {
        if (db_mode == DB_MODE_ADD) {
            db_mode = DB_MODE_REMOVE;
        } else {
            db_mode = DB_MODE_ADD;
        }
        sf_info(KEY_PRESSED, (db_mode == DB_MODE_ADD) ? "ADD" : "REMOVE");
    }
    return;
}

void *sf_decoder(void *conf) {
    product_node *node;
    int result;
    char url[100];
    size_t size;
    char *buffer;
    char *str_json;
    char *str_product;
    cJSON *decoded_pr;
    reader_conf *config = (reader_conf *) conf;
    sf_info(INIT_DECODER);

    while (1) {
        buffer = NULL;
        str_product = NULL;
        if (node = sf_queue_get(idq), !node) {
            if (execution_ends) {
                sf_info(DECODER_END);
                break;
            }
            sf_debug2(EMPTY_QUEUE, "ID");
            sleep(config->pr_sleep);
            continue;
        }

        if (node->mode == DB_MODE_REMOVE) {
            snprintf(url, 100, REMOVE_MSG, node->code);
            if (sf_queue_add(prq, strdup(url))) {
                sf_warn(FULL_QUEUE, str_product);
            } else {
                sf_debug1(DELETION_REQUEST, node->code);
            }
            goto next_it;
        }

        snprintf(url, 100, OF_API, node->code);

        if (sf_download_file(url, &size, &buffer)) {
            sf_error(DOWNLOAD_FILE_ERROR, url);
            goto next_it;
        }
        result = 0;
        if (!(str_json = strchr(buffer, '{')) || (result = sf_pr_raw_decoder(str_json, &decoded_pr, node->number, node->code)) !=  NO_ERROR) {
            switch (result) {
                case INV_FORMAT:
                    sf_error(INVALID_FILE_FORMAT);
                break;
                case UNEX_FIELDS:
                    sf_error(UNEXPECTED_FIELDS);
                break;
                case PR_NOT_FOUND:
                    sf_info(PRODUCT_NOT_FOUND, node->code);
                    goto next_it;
                break;
            }
            sf_error(INVALID_FILE, node->code);
            goto next_it;
        }

        str_product = cJSON_PrintUnformatted(decoded_pr);
        if (sf_queue_add(prq, str_product)) {
            sf_warn(FULL_QUEUE, str_product);
            free(str_product);
        }
        sf_debug2(DECODED_JSON, str_product);
next_it:
        free_product_node(node);
        free(buffer);
    }
}

int sf_send_product(char *data) {
    int retval = 0;
    int sock = -1;
    struct sockaddr_in server;
    static char message[MAX_LOGSIZE + 1];

    if (sock = socket(AF_INET , SOCK_STREAM , 0), sock == -1) {
        return 1;
    } else {
        server.sin_addr.s_addr = inet_addr(int_address);
        server.sin_family = AF_INET;
        server.sin_port = htons(int_port);

        if (connect(sock, (struct sockaddr *)&server , sizeof(server)) < 0) {
            sf_error(CONNECT_SERVER_ERROR);
            retval = 1;
        } else {
            snprintf(message, MAX_LOGSIZE, "%s%s", internal_header, data);
            if(send(sock, message, strlen(message), 0) < 0) {
                retval = 1;
            }
        }
    }

    close(sock);
    return retval;
}

void *sf_storer(void *conf) {
    reader_conf *config = (reader_conf *) conf;
    sf_info(INIT_STORER);
    char *data = NULL;

    while (1) {
        sleep(config->pr_sleep);
        if (!data && !(data = sf_queue_get(prq))) {
            if (execution_ends) {
                sf_info(STORER_END);
                break;
            }
            sf_debug2(EMPTY_QUEUE, "products");
            continue;
        }
        if (sf_send_product(data)) {
            sf_error(PRODUCT_SEND_ERROR);
            continue;
        }
        free(data);
        data = NULL;
    }
}

void sf_bar_handler(reader_conf *config) {
    product_node *node;
    char *pr_id = NULL;
    int number;
    int repeat;
    int result;

    if (!test_mode_file) {
        sf_info(INIT_BAR_HAND);
#ifdef CAMERA_ENABLED
        // Setting the key handler
        if (signal(SIGTSTP, key_handler) == SIG_ERR) {
            sf_exit_error(KEY_HANDLER_ERROR);
        }

        zbar_processor_t *proc = zbar_processor_create(1);
        zbar_processor_set_data_handler(proc, z_handler, NULL);

        //zbar_processor_set_config(proc, 0, ZBAR_CFG_ENABLE, 1);
        if (result = zbar_processor_init(proc, config->device, 1), result < 0) {
            sf_exit_error(DEVICE_OPEN_ERROR, config->device);
        };

        zbar_processor_set_visible(proc, 1);
        zbar_processor_set_active(proc, 1);
        zbar_processor_user_wait(proc, -1);

        zbar_processor_destroy(proc);
#endif
    } else { // (TBD)
        FILE *fp;
        char *found;
        size_t read_size = 0;
        sf_info(INIT_BAR_HAND_TEST, test_mode_speed, test_mode_file);

        if (!strcasecmp(config->test_repeat, "yes")) {
            repeat = 1;
        } else {
            repeat = 0;
        }

        if (fp = fopen(test_mode_file, "r"), !fp) {
            sf_exit_error(TEST_FILE_OPEN_ERROR);
        }

        while (1) {
            if (getline(&pr_id, &read_size, fp) < 1) {
                if (!repeat) {
                    sf_info(TEST_MODE_END);
                    fclose(fp);
                    break;
                }
                fseek(fp, 0, SEEK_SET);
                continue;
            }
            // Find comments
            if (found = strchr(pr_id, '#'), found) {
                if (found == pr_id) {
                    continue;
                }
                *found = '\0';
            }
            if (found = strchr(pr_id, '\n'), found) {
                *found = '\0';
            }
            // Get the elements
            if (found = strchr(pr_id, ':'), found) {
                *(found++) = '\0';
                number = strtol(found, NULL, 10);
            } else {
                number = 1;
            }
            sf_debug1(CODE_READ, pr_id, number);
            node = create_product_node(strdup(pr_id), number, db_mode);
            if (sf_queue_add(idq, node)) {
                sf_warn(FULL_QUEUE, node->code);
                free_product_node(node);
            }
            sleep(config->test_sleep);
        }
    }
}

void sf_rhelp() {
    fprintf(stdout, "\n%s: -[ddf]\n" \
                    "\t-d\t\tDebug mode\n" \
                    "\t-f\t\tForeground mode\n" \
                    "\t-t\t\tTest mode\n" \
                    "\t-p\t\tTest mode speed\n",
                    node_name);
}

int main(int argc, char **argv) {
    pthread_t de_t, st_t;
    reader_conf *config;
    connections_conf *config_con;
    int option;
    int result;
    static queue id_queue;
    static queue pr_queue;

    while (option = getopt(argc, argv, "ddft:p:"), option != -1) {
        switch (option) {
            case 'd':
                sf_set_debug();
            break;
            case 'f':
                sf_set_foreground();
            break;
            case 't':
                test_mode_file = strdup(optarg);
            break;
            case 'p':
                test_mode_speed = strtol(optarg, NULL, 10);
            break;
            default:
                sf_rhelp();
                return 1;
        }
    }

    sf_set_node_name(NODE_NAME);
    sf_set_log_file(LOG_FILE);

    if (sf_read_config(N_READER, (void **)&config)) {
        sf_exit_error(LOAD_CONFIG_ERROR);
    }

    if (sf_read_config(N_CONNECTIONS, (void **)&config_con)) {
        sf_exit_error(LOAD_CONFIG_ERROR);
    }

    internal_header = config_con->internal_header;
    int_header_size = strlen(internal_header);
    int_address = config_con->internal_address;
    int_port = config_con->internal_port;

    if (result = sf_queue_set(&id_queue, config->id_size), result == 1) {
        sf_exit_error(SMALL_QUEUE_SIZE, "ID", config->id_size);
    } else if (result == 2) {
        sf_exit_error(LARGE_QUEUE_SIZE, "ID", config->id_size);
    }

    if (result = sf_queue_set(&pr_queue, config->pr_size), result == 1) {
        sf_exit_error(SMALL_QUEUE_SIZE, "PRODUCT", config->pr_size);
    } else if (result == 2) {
        sf_exit_error(LARGE_QUEUE_SIZE, "PRODUCT", config->pr_size);
    }

    // Set the global queue
    idq = &id_queue;
    prq = &pr_queue;

    if (pthread_create(&de_t, NULL, sf_decoder, config) ||
        pthread_create(&st_t, NULL, sf_storer, config)) {
        sf_exit_error(THREADS_INIT_ERROR);
    }

    sf_bar_handler(config);
    execution_ends = 1;
    pthread_join(de_t, NULL);
    pthread_join(st_t, NULL);

    free_reader_conf(config);
    free_connection_conf(config_con);
    return 0;
}
