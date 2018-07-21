// https://curl.haxx.se/libcurl/c/10-at-a-time.html

#include <log.h>
#include <curl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define URL_FORMAT "https://world.openfoodfacts.org/api/v0/product/%s.json"

const char *node_name = "curl_json_test";
const char *log_file = NULL;

typedef struct data_fetch {
    char *payload;
    size_t size;
} data_fetch;

size_t curl_callback (void *contents, size_t size, size_t nmemb, void *collected) {
	size_t realsize = size * nmemb;
	data_fetch *fetched = (data_fetch *) collected;

	fetched->payload = (char *) realloc(fetched->payload, fetched->size + realsize + 1);

	if (!fetched->payload) {
		sf_error(CURL_EXPAND_ERROR);
		free(fetched->payload);
		return 0;
	}

	memcpy(&(fetched->payload[fetched->size]), contents, realsize);
	fetched->size += realsize;
	fetched->payload[fetched->size] = 0;

	return realsize;
}

void sft_help() {
    fprintf(stdout, "\n%s: <product id>\n", node_name);
}

int main(int argc, char **argv) {
    CURL *c_handler;
    struct curl_slist *headers;
    char url[100];
    data_fetch data;

    if (argc < 2) {
        sft_help();
    }

    data.payload = NULL;
    data.size = 0;

    snprintf(url, 100, URL_FORMAT, argv[1]);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    if (c_handler = curl_easy_init(), !c_handler) {
        return -1;
    }

    headers = curl_slist_append(NULL , "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charsets: utf-8");

    curl_easy_setopt(c_handler, CURLOPT_URL, url);
    curl_easy_setopt(c_handler, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(c_handler, CURLOPT_WRITEDATA, (void *) &data);
    curl_easy_setopt(c_handler, CURLOPT_HEADER, headers);

    if (curl_easy_perform(c_handler) != CURLE_OK) {
        return -1;
    }

    return 0;
}
