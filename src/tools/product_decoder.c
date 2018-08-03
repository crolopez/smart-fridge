#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cJSON.h"
#include "product_decoder.h"

static char *JSON_PRODUCT = "product";
static char *JSON_PRODUCT_IMAGES = "selected_images";
static char *JSON_IMAGES = "images";
static char *JSON_PRODUCT_NAME = "product_name";
static char *JSON_NAME = "name";
static char *JSON_PRODUCT_QUANTITY = "nutrition_data_per";
static char *JSON_QUANTITY = "quantity";
static char *JSON_PRODUCT_INGREDIENTS = "ingredients_text";
static char *JSON_INGREDIENTS = "ingredients";
static char *JSON_PRODUCT_BRANDS = "brands";
static char *JSON_PRODUCT_EXPIRATION_DATE = "expiration_date";
static char *JSON_PRODUCT_LABELS = "labels";
static char *JSON_PRODUCT_ALLERGENS_TAGS = "allergens_tags";
static char *JSON_ALLERGENS_TAGS = "allergens";
static char *JSON_PRODUCT_ADDITIVES_TAGS = "additives_tags";
static char *JSON_ADDITIVES = "additives";
static char *JSON_PRODUCT_INGREDIENTES_TAGS = "ingredients_tags";
static char *JSON_INGREDIENTES_TAGS = "ingredients";
static char *JSON_PRODUCT_IMAGES_FRONT = "front";
static char *JSON_PRODUCT_IMAGES_NUTRITION = "nutrition";
static char *JSON_PRODUCT_IMAGES_INGREDIENTS = "ingredients";
static char *JSON_PRODUCT_IMAGES_FORMAT = "display";
static char *JSON_STATUS_VERBOSE = "status_verbose";
static char *JSON_TAGS = "tags";
static char *JSON_NUMBER = "number";
static char *JSON_PRODUCT_ES = "es";
static char *JSON_PRODUCT_CODE = "code";
static char *JSON_PRODUCT_TIMESTAMP = "timestamp";
static char *JSON_PRODUCT_CATEGORIES = "categories"; // Substitute for product_name
static char *JSON_GENERIC_NAME = "generic_name"; // Substitute for product_name
static char *PRODUCT_NOT_FOUND = "product not found";

int sf_pr_raw_decoder(char *str_json, cJSON **pr_dec, int number, char *code) {
    cJSON *obj, *json_product, *json_images, *json_it, *json_it_aux;
    cJSON *decoded = NULL;
    cJSON *decoded_images = NULL;
    cJSON *tags = NULL;
    cJSON *tags_additives = NULL;
    cJSON *tags_allergens = NULL;
    cJSON *tags_ingredients = NULL;
    char str_timestamp[26];
    struct tm *timestamp;
    time_t t_now;
    int retval = NO_ERROR;
    int i;
    *pr_dec = NULL;

    if (obj = cJSON_Parse(str_json), !obj) {
        retval = INV_FORMAT;
        goto end;
    }

    if (decoded = cJSON_CreateObject(), !decoded) {
      goto end;
    }

    if (json_product = cJSON_GetObjectItem(obj, JSON_PRODUCT), !json_product) {
        // May not have found it
        if ((json_product = cJSON_GetObjectItem(obj, JSON_STATUS_VERBOSE)) &&
            !strcmp(json_product->valuestring, PRODUCT_NOT_FOUND)) {
            retval = PR_NOT_FOUND;
        } else {
            retval = UNEX_FIELDS;
        }
        goto end;
    }

    if (!(tags = cJSON_CreateObject())          ||
        !(tags_additives = cJSON_CreateArray()) ||
        !(tags_allergens = cJSON_CreateArray())  ||
        !(tags_ingredients = cJSON_CreateArray())) {
        retval = UNEX_FIELDS;
        goto end;
    }

    // Get the product name
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_NAME), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else if (*json_it->valuestring == '\0') {
        // Empty field. Try the generic name
        if (json_it = cJSON_GetObjectItem(json_product, JSON_GENERIC_NAME), !json_it) {
            retval = UNEX_FIELDS;
            goto end;
        } else if (*json_it->valuestring == '\0') {
            // Empty field. Try the categories as name
            if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_CATEGORIES), !json_it) {
                retval = UNEX_FIELDS;
                goto end;
            } else if (*json_it->valuestring == '\0') {
                retval = UNEX_FIELDS;
                goto end;
            }
        }
    }
    cJSON_AddStringToObject(decoded, "name", json_it->valuestring);

    // End of attempts to obtain the product name

    // Add the elements number
    cJSON_AddNumberToObject(decoded, "number", (double) number);

    // Get the product quantity
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_QUANTITY), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        cJSON_AddStringToObject(decoded, "quantity", json_it->valuestring);
    }

    // Get the product ingredients
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_INGREDIENTS), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        cJSON_AddStringToObject(decoded, "ingredients", json_it->valuestring);
    }

    // Get the product brands
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_BRANDS), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        cJSON_AddStringToObject(decoded, "brands", json_it->valuestring);
    }

    // Get the product expiration date
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_EXPIRATION_DATE), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        cJSON_AddStringToObject(decoded, "expiration_date", json_it->valuestring);
    }

    // Get the product labels
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_LABELS), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        cJSON_AddStringToObject(decoded, "labels", json_it->valuestring);
    }

    // Get the product additives tags
    if (json_it_aux = cJSON_GetObjectItem(json_product, JSON_PRODUCT_ADDITIVES_TAGS), !json_it_aux) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        for (i = 0, json_it = json_it_aux->child; json_it; json_it = json_it->next, i++) {
            cJSON_AddItemToArray(tags_additives, cJSON_CreateString(json_it->valuestring));
        }
        cJSON_AddItemToObject(tags, "additives", tags_additives);
        tags_additives = NULL;
    }

    // Get the product allergens tags
    if (json_it_aux = cJSON_GetObjectItem(json_product, JSON_PRODUCT_ALLERGENS_TAGS), !json_it_aux) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        for (i = 0, json_it = json_it_aux->child; json_it; json_it = json_it->next, i++) {
            cJSON_AddItemToArray(tags_allergens, cJSON_CreateString(json_it->valuestring));
        }
        cJSON_AddItemToObject(tags, "allergens", tags_allergens);
        tags_allergens = NULL;
    }

    // Get the product ingredients tags
    if (json_it_aux = cJSON_GetObjectItem(json_product, JSON_PRODUCT_INGREDIENTES_TAGS), !json_it_aux) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        for (i = 0, json_it = json_it_aux->child; json_it; json_it = json_it->next, i++) {
            cJSON_AddItemToArray(tags_ingredients, cJSON_CreateString(json_it->valuestring));
        }
        cJSON_AddItemToObject(tags, "ingredients", tags_ingredients);
        tags_ingredients = NULL;
    }

    cJSON_AddItemToObject(decoded, "tags", tags);
    tags = NULL;

    // Get the product images
    if (json_images = cJSON_GetObjectItem(json_product, JSON_PRODUCT_IMAGES), !json_images) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        if (decoded_images = cJSON_CreateObject(), !decoded_images) {
          goto end;
        }
        // Get the front image of the product
        if ((json_it_aux = cJSON_GetObjectItem(json_images, JSON_PRODUCT_IMAGES_FRONT)) &&
            (json_it_aux = cJSON_GetObjectItem(json_it_aux, JSON_PRODUCT_IMAGES_FORMAT))) {
            // Get the url
            if (json_it = cJSON_GetObjectItem(json_it_aux, JSON_PRODUCT_ES), !json_it) {
                if (!json_it_aux->child || !json_it_aux->child->valuestring) {
                    retval = UNEX_FIELDS;
                    goto end;
                }
                cJSON_AddStringToObject(decoded_images, "front", json_it_aux->child->valuestring);
            } else {
                cJSON_AddStringToObject(decoded_images, "front", json_it->valuestring);
            }
        }

        // Get the nutrition image of the product
        if ((json_it_aux = cJSON_GetObjectItem(json_images, JSON_PRODUCT_IMAGES_NUTRITION)) &&
            (json_it_aux = cJSON_GetObjectItem(json_it_aux, JSON_PRODUCT_IMAGES_FORMAT))) {
            // Get the url
            if (json_it = cJSON_GetObjectItem(json_it_aux, JSON_PRODUCT_ES), !json_it) {
                if (!json_it_aux->child || !json_it_aux->child->valuestring) {
                    retval = UNEX_FIELDS;
                    goto end;
                }
                cJSON_AddStringToObject(decoded_images, "front", json_it_aux->child->valuestring);
            } else {
                cJSON_AddStringToObject(decoded_images, "nutrition", json_it->valuestring);
            }
        }

        // Get the ingredients image of the product
        if ((json_it_aux = cJSON_GetObjectItem(json_images, JSON_PRODUCT_IMAGES_INGREDIENTS)) &&
            (json_it_aux = cJSON_GetObjectItem(json_it_aux, JSON_PRODUCT_IMAGES_FORMAT))) {
            // Get the url
            if (json_it = cJSON_GetObjectItem(json_it_aux, JSON_PRODUCT_ES), !json_it) {
                if (!json_it_aux->child || !json_it_aux->child->valuestring) {
                    retval = UNEX_FIELDS;
                    goto end;
                }
                cJSON_AddStringToObject(decoded_images, "front", json_it_aux->child->valuestring);
            } else {
                cJSON_AddStringToObject(decoded_images, "ingredients", json_it->valuestring);
            }
        }
        cJSON_AddItemToObject(decoded, "images", decoded_images);
        decoded_images = NULL;
    }

    t_now = time(NULL);
    timestamp = localtime(&t_now);
    strftime(str_timestamp, 26, "%Y-%m-%d %H:%M:%S", timestamp);
    cJSON_AddStringToObject(decoded, "timestamp", str_timestamp);

    cJSON_AddStringToObject(decoded, "code", code);

    *pr_dec = decoded;
    decoded = NULL;
end:
    if (obj) {
        cJSON_free(obj);
    }
    if (decoded) {
        cJSON_free(decoded);
    }
    if (decoded_images) {
        cJSON_free(decoded_images);
    }
    if (tags_additives) {
        cJSON_free(decoded_images);
    }
    if (tags_allergens) {
        cJSON_free(decoded_images);
    }
    if (tags_ingredients) {
        cJSON_free(decoded_images);
    }
    if (tags) {
        cJSON_free(tags);
    }
    return retval;
}

int sf_pr_decoder(char *str_json, product **pr_dec) {
    cJSON *tags, *json_product, *json_images, *json_it, *json_it_aux;
    product *decoded = NULL;
    int retval = NO_ERROR;
    int i;
    *pr_dec = NULL;

    if (json_product = cJSON_Parse(str_json), !json_product) {
        retval = INV_FORMAT;
        goto end;
    }

    decoded = calloc(1, sizeof(product));
    memset(decoded, 0, sizeof(product));

    // Get the product name
    if (json_it = cJSON_GetObjectItem(json_product, JSON_NAME), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        decoded->name = strdup(json_it->valuestring);
    }

    // Get elements
    if (json_it = cJSON_GetObjectItem(json_product, JSON_NUMBER), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        decoded->number = json_it->valueint;
    }

    // Get the product quantity
    if (json_it = cJSON_GetObjectItem(json_product, JSON_QUANTITY), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        if (*json_it->valuestring != '\0') {
            decoded->quantity = strdup(json_it->valuestring);
        }
    }

    // Get the product ingredients
    if (json_it = cJSON_GetObjectItem(json_product, JSON_INGREDIENTS), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        if (*json_it->valuestring != '\0') {
            decoded->ingredients = strdup(json_it->valuestring);
        }
    }

    // Get the product brands
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_BRANDS), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        if (*json_it->valuestring != '\0') {
            decoded->brands = strdup(json_it->valuestring);
        }
    }

    // Get the product expiration date
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_EXPIRATION_DATE), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        if (*json_it->valuestring != '\0') {
            decoded->expiration_date = strdup(json_it->valuestring);
        }
    }

    // Get the product code
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_CODE), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        if (*json_it->valuestring != '\0') {
            decoded->code = strdup(json_it->valuestring);
        }
    }

    // Get the timestamp
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_TIMESTAMP), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        if (*json_it->valuestring != '\0') {
            decoded->timestamp = strdup(json_it->valuestring);
        }
    }

    // Get the product labels
    if (json_it = cJSON_GetObjectItem(json_product, JSON_PRODUCT_LABELS), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        if (*json_it->valuestring != '\0') {
            decoded->labels = strdup(json_it->valuestring);
        }
    }

    // Get the product tags
    if (tags = cJSON_GetObjectItem(json_product, JSON_TAGS), !json_it) {
        retval = UNEX_FIELDS;
        goto end;
    }

    // Get the product additives tags
    if (json_it_aux = cJSON_GetObjectItem(tags, JSON_ADDITIVES), !json_it_aux) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        for (i = 0, json_it = json_it_aux->child; json_it; json_it = json_it->next, i++) {
            decoded->additives_tags = realloc(decoded->additives_tags, sizeof(char *) * (i + 2));
            decoded->additives_tags[i] = strdup(json_it->valuestring);
            decoded->additives_tags[i + 1] = NULL;
        }
    }

    // Get the product allergens tags
    if (json_it_aux = cJSON_GetObjectItem(tags, JSON_ALLERGENS_TAGS), !json_it_aux) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        for (i = 0, json_it = json_it_aux->child; json_it; json_it = json_it->next, i++) {
            decoded->allergens_tags = realloc(decoded->allergens_tags, sizeof(char *) * (i + 2));
            decoded->allergens_tags[i] = strdup(json_it->valuestring);
            decoded->allergens_tags[i + 1] = NULL;
        }
    }

    // Get the product ingredients tags
    if (json_it_aux = cJSON_GetObjectItem(tags, JSON_INGREDIENTES_TAGS), !json_it_aux) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        for (i = 0, json_it = json_it_aux->child; json_it; json_it = json_it->next, i++) {
            decoded->ingredients_tags = realloc(decoded->ingredients_tags, sizeof(char *) * (i + 2));
            decoded->ingredients_tags[i] = strdup(json_it->valuestring);
            decoded->ingredients_tags[i + 1] = NULL;
        }
    }

    // Get the product images
    if (json_images = cJSON_GetObjectItem(json_product, JSON_IMAGES), !json_images) {
        retval = UNEX_FIELDS;
        goto end;
    } else {
        // Get the front image of the product
        if (json_it= cJSON_GetObjectItem(json_images, JSON_PRODUCT_IMAGES_FRONT), json_it) {
            decoded->images.front = strdup(json_it->valuestring);
        }

        // Get the nutrition image of the product
        if (json_it = cJSON_GetObjectItem(json_images, JSON_PRODUCT_IMAGES_NUTRITION), json_it) {
            decoded->images.nutrition = strdup(json_it->valuestring);
        }

        // Get the ingredients image of the product
        if (json_it = cJSON_GetObjectItem(json_images, JSON_PRODUCT_IMAGES_INGREDIENTS), json_it) {
            decoded->images.ingredients = strdup(json_it->valuestring);
        }
    }

    *pr_dec = decoded;
    decoded = NULL;
end:
    if (decoded) {
        free(decoded);
    }
    if (json_product) {
        cJSON_free(json_product);
    }
    return retval;
}

void free_product(product *pr_dec) {
    int i;
    free(pr_dec->name);
    free(pr_dec->code);
    free(pr_dec->timestamp);
    free(pr_dec->quantity);
    free(pr_dec->ingredients);
    free(pr_dec->brands);
    free(pr_dec->expiration_date);
    free(pr_dec->labels);
    free(pr_dec->images.front);
    free(pr_dec->images.nutrition);
    free(pr_dec->images.ingredients);
    if (pr_dec->ingredients_tags) {
        for (i = 0; pr_dec->ingredients_tags[i]; i++) {
            free(pr_dec->ingredients_tags[i]);
        }
        free(pr_dec->ingredients_tags);
    }

    if (pr_dec->allergens_tags) {
        for (i = 0; pr_dec->allergens_tags[i]; i++) {
            free(pr_dec->allergens_tags[i]);
        }
        free(pr_dec->allergens_tags);
    }

    if (pr_dec->additives_tags) {
        for (i = 0; pr_dec->additives_tags[i]; i++) {
            free(pr_dec->additives_tags[i]);
        }
        free(pr_dec->additives_tags);
    }
    free(pr_dec);
}

void free_product_node(product_node *node) {
    if (node) {
        free(node->code);
        free(node);
    }
}

product_node *create_product_node(char *id, int number) {
    product_node *node;
    node = calloc(1, sizeof(product_node));
    node->code = id;
    node->number = number;
    return node;
}
