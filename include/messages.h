#ifndef SF_MESSAGES
#define SF_MESSAGES

#define LOAD_CONFIG_ERROR         "(000) Could not load the configuration."
#define THREADS_INIT_ERROR        "(001) The threads could not be started."
#define INIT_DECODER              "(002) Starting decoder."
#define INIT_STORER               "(003) Starting storer."
#define INV_CONFIG                "(004) The configuration could not be readed due to invalid formatting."
#define INV_CONFIG_TAG            "(005) The configuration could not be loaded because an invalid tag (%s) was found."
#define INV_CONFIG_TAG_BLOCK      "(006) The configuration could not be loaded because an invalid tag (%s) was found in '%s' block."
#define INV_CONFIG_IN_BLOCK       "(007) The configuration could not be loaded because the block '%s' has a invalid content."
#define SMALL_QUEUE_SIZE          "(008) The %s queue has been assigned too small: %i."
#define LARGE_QUEUE_SIZE          "(009) The %s queue has been assigned too large: %i."
#define FULL_QUEUE                "(010) The ID '%s' is discarded because the queue is full."
#define INIT_BAR_HAND             "(011) Starting bar handler."
#define EMPTY_QUEUE               "(012) The %s queue is empty."
#define CURL_EXPAND_ERROR         "(013) Failed to expand buffer in curl_callback."
#define DOWNLOAD_FILE_ERROR       "(014) Could not download from '%s'."
#define INVALID_FILE              "(015) The downloaded file with the code '%s' is invalid."
#define INVALID_FILE_FORMAT       "(016) The file is not json format."
#define UNEXPECTED_FIELDS         "(017) Unexpected fields when parsing the file."
#define INIT_BAR_HAND_TEST        "(018) Starting bar handler in test mode with %d seconds period. Feed: '%s'."
#define TEST_FILE_OPEN_ERROR      "(019) The test file could not be opened."
#define CODE_READ                 "(020) The '%s' code has been read. Elements: %d."
#define DATABASE_CREATE_ERROR     "(021) The database could not be created."
#define DECODED_JSON              "(022) Decoded JSON: '%s'."
#define PRODUCT_SEND_ERROR        "(023) The product could not be sent."
#define BIND_ERROR                "(024) The %d port could not be binded. Error: %d."
#define INCOMING_CON_ERROR        "(025) The incoming connection could not be accepted."
#define MSG_RECEIVED              "(026) Message received: '%s'."
#define RECEIVE_ERROR             "(027) One of the messages could not be received."
#define DECODER_ERROR             "(028) Message received could not be decoded."
#define CONNECT_SERVER_ERROR      "(029) Could not connect to the server."
#define UNKNOWN_MSG_FORMAT        "(030) Unknown message format."
#define PRODUCT_INSERT_ERROR      "(031) The product could not be inserted into the database."
#define TAGS_INSERT_ERROR         "(032) %s tags could not be inserted."
#define PRODUCT_NOT_FOUND         "(033) No information could be extracted of the product with code '%s'."
#define TEST_MODE_END             "(034) Finishing test mode."
#define DECODER_END               "(035) Finishing product decoding."
#define STORER_END                "(036) Finishing product storing."
#define LISTEN_PORT               "(037) Listening on port %d."
#define DB_COPY_SENT              "(038) A copy of the database has been sent."
#define DB_COPY_SENT_ERROR        "(039) A database copy could not be sent."
#define DB_SIZE_MSG               "(040) Database size: %s."
#define DB_CHUNK_REMAINING        "(041) %d bytes have been sent to the client. %d to go."

#endif
