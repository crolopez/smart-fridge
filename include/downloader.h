#ifndef SF_DOWN
#define SF_DOWN

typedef struct data_fetch {
    char *payload;
    size_t size;
} data_fetch;

size_t sf_curl_callback (void *contents, size_t size, size_t nmemb, void *collected);
int sf_download_file(char *url, size_t *size, char **buffer);

#endif
