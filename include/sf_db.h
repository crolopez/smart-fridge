#ifndef SF_DB
#define SF_DB

#define MAX_QUERY_SIZE 65536 // 64 KB
#define DB_SYNC_HEADER "!#2+"
#define MAX_MSG_CHUNK 6144
#define CONTINUE_IT 32

#define NODE_NAME "DATABASE_MANAGER"
#define LOG_FILE "./sf.log"
extern const char *database_schema;

int start_daemon();
void sf_dhelp();
int create_db();
int msg_decoder(int client_sock, char *msg);
int db_insert(product *pr_dec);
int db_remove(product *pr_dec);
int insert_tags(sqlite3 *db, char **array, char *product_name, int type);
int insert_images(sqlite3 *db, product_images *images, char *product_code);
int database_send(int sock);

typedef enum QUERY_TYPE {
    INSERT_PRODUCT_DATA,
    INC_PRODUCT_NUMBER,
    INSERT_INGREDIENT_TAG,
    INSERT_ALLERGEN_TAG,
    INSERT_ADDITIVE_TAG,
    INSERT_IMAGES,
    REMOVE_PRODUCT,
    CLEAN_EMPTY,
    BEGIN_TRANSACTION,
    END_TRANSACTION
} QUERY_TYPE;

const char *DB_QUERIES[] = {
    "INSERT INTO PRODUCTS_DATA VALUES(?,?,?,?,?,?,?,?,?);",
    "UPDATE PRODUCTS_DATA SET ELEMENTS = ELEMENTS + ? WHERE NAME = ?;",
    "INSERT INTO INGREDIENTS_TAGS VALUES(?,?,?);",
    "INSERT INTO ALLERGENS_TAGS VALUES(?,?,?);",
    "INSERT INTO ADDITIVES_TAGS VALUES(?,?,?);",
    "INSERT INTO IMAGES VALUES(?,?,?,?);",
    "UPDATE PRODUCTS_DATA SET ELEMENTS = ELEMENTS - 1 WHERE CODE = ?;",
    "DELETE FROM PRODUCTS_DATA WHERE ELEMENTS = 0;",
    "BEGIN TRANSACTION;",
    "END TRANSACTION;"
};

#endif
