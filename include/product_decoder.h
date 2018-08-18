#ifndef SF_PRODUCT_DECODER
#define SF_PRODUCT_DECODER

#include "cJSON.h"

#define DELETION_REQ 2

// Product decoder outputs
typedef enum pr_dec_error {
    NO_ERROR,
    INV_FORMAT,
    UNEX_FIELDS,
    PR_NOT_FOUND
} pr_dec_error;

typedef struct product_node {
    char *code;
    int number;
    int mode;
} product_node;

typedef struct product_images { // product -> select_images
    char *front;
    char *nutrition;
    char *ingredients;
} product_images;

typedef struct product {
    product_images images;
    char *name; // product->product_name
    char *code;
    char *timestamp;
    char *quantity; // product -> nutrition_data_per
    char *ingredients; // product -> ingredients_text (can be order by another field)
    char *brands; // product -> brands
    char *expiration_date; // product -> expiration_date
    char *labels; // product -> labels_prev_tags
    char **ingredients_tags; // product -> ingredients_tags (to sorting)
    char **allergens_tags;
    char **additives_tags;
    int number;
} product;

int sf_pr_decoder(char *str_json, product **pr_dec);
int sf_pr_raw_decoder(char *str_json, cJSON **pr_dec, int number, char *code);
product_node *create_product_node(char *id, int number, int mode);
void free_product(product *pr_dec);
void free_product_node(product_node *node);

#endif
