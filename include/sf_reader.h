#ifndef SF_READER
#define SF_READER

#include "config.h"
#include "cJSON.h"

#define MAX_ID_LEN 50
#define MAX_PRODUCT_PRINT_SIZE 1024
#define OF_API "https://world.openfoodfacts.org/api/v0/product/%s.json"
#define NODE_NAME "reader"
#define LOG_FILE INSTALL_FOLDER "/sf.log"
#define DB_MODE_ADD 1
#define DB_MODE_REMOVE 2
#define REMOVE_MSG "{\"code\":\"%s\",\"remove\":\"yes\"}"

void sf_rhelp();
product_node *sf_scan_code();
int sf_send_product(char *data);
static void key_handler(int signum);
#ifdef CAMERA_ENABLED
void z_handler (zbar_image_t *im, const void *data);
#endif

void *sf_decoder(void *conf);
void *sf_storer(void *conf);
void sf_bar_handler(reader_conf *config);

#endif
