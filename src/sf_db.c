#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "product_decoder.h"
#include "sqlite3.h"
#include "config.h"
#include "sf_db.h"
#include "log.h"

static const char *internal_header;
static int int_header_size;

void sf_dhelp() {
    fprintf(stdout, "\n%s: -[ddf]\n" \
                    "\t-d\t\tDebug mode\n" \
                    "\t-f\t\tForeground mode\n",
                    node_name);
}

int start_daemon(connections_conf *config) {
    int client_sock;
    int internal_sock_desc;
    struct sockaddr_in server, client;
    char msg[MAX_LOGSIZE];
    size_t sock_size = sizeof(struct sockaddr_in);
    int read_size;

    internal_sock_desc = socket(AF_INET , SOCK_STREAM , 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(config->internal_port);

    if(bind(internal_sock_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
        sf_error(BIND_ERROR, config->internal_port, -2);
        return 1;
    }

    listen(internal_sock_desc , 1);

    while (1) {
        if (client_sock = accept(internal_sock_desc, (struct sockaddr *)&client, (socklen_t*)&sock_size), client_sock < 0) {
            sf_error(INCOMING_CON_ERROR);
        } else {
            while(read_size = recv(client_sock , msg , MAX_LOGSIZE , 0), read_size > 0) {
                msg[read_size] = '\0';
                sf_debug1(MSG_RECEIVED, msg);
                // Decode the message
                if (msg_decoder(msg)) {
                    sf_error(DECODER_ERROR);
                }
            }

            if (read_size < 0) {
                sf_error(RECEIVE_ERROR);
            }

            close(client_sock);
        }

    }
}

int msg_decoder(char *msg) {
    int retval = 1;
    product *pr_dec = NULL;

    if (!strncmp(msg, internal_header, int_header_size)) {
        msg += int_header_size;
        if (sf_pr_decoder(msg, &pr_dec), !pr_dec) {
            goto end;
        }
        if (db_insert(pr_dec)) {
            sf_error(PRODUCT_INSERT_ERROR);
            goto end;
        }
    } else {
        sf_error(UNKNOWN_MSG_FORMAT);
        goto end;
    }
    retval = 0;
end:
    if (pr_dec) {
        free_product(pr_dec);
    }
    return retval;
}

int db_insert(product *pr_dec) {
    sqlite3 *db = NULL;
    const char *tail;
    sqlite3_stmt *stmt = NULL;
    int retval = 1;
    int result;
    int i;
    char *name_tag;
    char *language_tag;

    if (sqlite3_open_v2(DB_LOCATION, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
        goto end;
    }

    if (result = sqlite3_prepare_v2(db, DB_QUERIES[BEGIN_TRANSACTION], -1, &stmt, NULL), result != SQLITE_OK) {
        goto end;
    }
    sqlite3_finalize(stmt);

    // Insert the product data
    if (result = sqlite3_prepare_v2(db, DB_QUERIES[INSERT_PRODUCT_DATA], -1, &stmt, NULL), result != SQLITE_OK) {
        goto end;
    }
    sqlite3_bind_text(stmt, 1, pr_dec->name, -1, NULL);
    sqlite3_bind_text(stmt, 2, pr_dec->quantity, -1, NULL);
    sqlite3_bind_text(stmt, 3, pr_dec->ingredients, -1, NULL);
    sqlite3_bind_text(stmt, 4, pr_dec->brands, -1, NULL);
    sqlite3_bind_text(stmt, 5, pr_dec->expiration_date, -1, NULL);
    sqlite3_bind_text(stmt, 6, pr_dec->labels, -1, NULL);
    sqlite3_bind_int(stmt, 7, pr_dec->number);
    if (result = sqlite3_step(stmt), result != SQLITE_DONE) {
        if (result == SQLITE_CONSTRAINT) {
            sqlite3_finalize(stmt);
            // If the product already exists, increase its number
            if (result = sqlite3_prepare_v2(db, DB_QUERIES[INC_PRODUCT_NUMBER], -1, &stmt, NULL), result != SQLITE_OK) {
                goto end;
            }
            sqlite3_bind_int(stmt, 1, pr_dec->number);
            sqlite3_bind_text(stmt, 2, pr_dec->name, -1, NULL);
            if (result = sqlite3_step(stmt), result != SQLITE_DONE) {
                goto end;
            }
        } else {
            goto end;
        }
    }
    sqlite3_finalize(stmt);
    stmt = NULL;

    // Insert the tags
    if (insert_tags(db, pr_dec->ingredients_tags, pr_dec->name, INSERT_INGREDIENT_TAG)) {
        sf_error(TAGS_INSERT_ERROR, "Ingredients");
        goto end;
    }
    if (insert_tags(db, pr_dec->allergens_tags, pr_dec->name, INSERT_ALLERGEN_TAG)) {
        sf_error(TAGS_INSERT_ERROR, "Allergens");
        goto end;
    }
    if (insert_tags(db, pr_dec->additives_tags, pr_dec->name, INSERT_ADDITIVE_TAG)) {
        sf_error(TAGS_INSERT_ERROR, "Additives");
        goto end;
    }

    retval = 0;
end:
    if (stmt) {
        sqlite3_finalize(stmt);
    }
    sqlite3_prepare_v2(db, DB_QUERIES[END_TRANSACTION], -1, &stmt, NULL);
    if (db) {
        sqlite3_close_v2(db);
    }
    return retval;
}

int insert_tags(sqlite3 *db, char **array, char *product_name, int type) {
    int retval = 1;
    int i;
    char *name_tag;
    char *language_tag;
    int result;
    const char *tail;
    sqlite3_stmt *stmt = NULL;

    if (array) {
        for (i = 0; array[i]; i++) {
            if (result = sqlite3_prepare_v2(db, DB_QUERIES[type], -1, &stmt, NULL), result != SQLITE_OK) {
                goto end;
            }
            sqlite3_bind_text(stmt, 1, product_name, -1, NULL);
            if (name_tag = strchr(array[i], ':'), name_tag) {
                *(name_tag++) = '\0';
                language_tag = array[i];
            } else {
                name_tag = array[i];
                language_tag = NULL;
            }

            sqlite3_bind_text(stmt, 2, name_tag, -1, NULL);
            sqlite3_bind_text(stmt, 3, language_tag, -1, NULL);
            if (result = sqlite3_step(stmt), result != SQLITE_DONE && result != SQLITE_CONSTRAINT) {
                goto end;
            }
            sqlite3_finalize(stmt);
            stmt = NULL;
        }
    }
    retval = 0;
end:
    if(stmt) {
        sqlite3_finalize(stmt);
    }
    return retval;
}

int create_db() {
    const char *sql;
    const char *tail;
    sqlite3_stmt *stmt;
    sqlite3 *db;

    if (sqlite3_open_v2(DB_LOCATION, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
        return 1;
    }

    for (sql = database_schema; sql && *sql; sql = tail) {
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, &tail) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return 1;
        }

        switch (sqlite3_step(stmt)) {
        case SQLITE_MISUSE:
            break;
        case SQLITE_ROW:
            break;
        case SQLITE_DONE:
            break;
        default:
            sqlite3_finalize(stmt);
            return 1;
        }

        sqlite3_finalize(stmt);
    }

    sqlite3_close_v2(db);
    return 0;
}

int main(int argc, char **argv) {
    connections_conf *config;
    int option;

    while (option = getopt(argc, argv, "ddf"), option != -1) {
        switch (option) {
            case 'd':
                sf_set_debug();
            break;
            case 'f':
                sf_set_foreground();
            break;
            default:
                sf_dhelp();
                return 1;
        }
    }

    if (sf_read_config(N_CONNECTIONS, (void **)&config)) {
        sf_exit_error(LOAD_CONFIG_ERROR);
    }

    internal_header = config->internal_header;
    int_header_size = strlen(internal_header);

    if (create_db()) {
        sf_error(DATABASE_CREATE_ERROR);
    }

    start_daemon(config);

    return 0;
}
