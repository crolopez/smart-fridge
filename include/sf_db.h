#ifndef SF_DB
#define SF_DB

#define MAX_QUERY_SIZE 65536 // 64 KB

const char *node_name = "database server";
const char *log_file = "./sf.log";
extern const char *database_schema;

int start_daemon();
void sf_dhelp();
int create_db();
int msg_decoder(char *msg);
int db_insert(product *pr_dec);
int insert_tags(sqlite3 *db, char **array, char *product_name, int type);

typedef enum QUERY_TYPE {
    INSERT_PRODUCT_DATA,
    INC_PRODUCT_NUMBER,
    INSERT_INGREDIENT_TAG,
    INSERT_ALLERGEN_TAG,
    INSERT_ADDITIVE_TAG,
    BEGIN_TRANSACTION,
    END_TRANSACTION
} QUERY_TYPE;

const char *DB_QUERIES[] = {
    "INSERT INTO PRODUCTS_DATA VALUES(?,?,?,?,?,?,?);",
    "UPDATE PRODUCTS_DATA SET ELEMENTS = ELEMENTS + ? WHERE NAME = ?;",
    "INSERT INTO INGREDIENTS_TAGS VALUES(?,?,?);",
    "INSERT INTO ALLERGENS_TAGS VALUES(?,?,?);",
    "INSERT INTO ADDITIVES_TAGS VALUES(?,?,?);",
    "BEGIN TRANSACTION;",
    "END TRANSACTION;"
};

#endif
