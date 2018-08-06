#ifndef SF_DB
#define SF_DB

#define MAX_QUERY_SIZE 65536 // 64 KB
#define DB_SYNC_HEADER "!#2+"
#define MAX_MSG_CHUNK 6144
#define CONTINUE_IT 32

const char *node_name = "database server";
const char *log_file = "./sf.log";
extern const char *database_schema;

int start_daemon();
void sf_dhelp();
int create_db();
int msg_decoder(int client_sock, char *msg);
int db_insert(product *pr_dec);
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
    "BEGIN TRANSACTION;",
    "END TRANSACTION;"
};

#endif
