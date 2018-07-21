#include <stdlib.h>
#include <log.h>
#include <string.h>
#include <curl.h>
#include <stdio.h>
#include "downloader.h"

size_t sf_curl_callback (void *contents, size_t size, size_t nmemb, void *collected) {
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

int sf_download_file(char *url, size_t *size, char **buffer) {
    CURL *c_handler = NULL;
    struct curl_slist *headers = NULL;
    data_fetch data;
    static char init_global = 0;
	int retval = 1;
    if (!init_global) {
        init_global = 1;
        curl_global_init(CURL_GLOBAL_ALL);
    }
	memset(&data, 0, sizeof(data_fetch));

	if (c_handler = curl_easy_init(), !c_handler) {
		goto end;
    }

	// At the moment we will only support JSON file downloads
	headers = curl_slist_append(NULL , "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");

	curl_easy_setopt(c_handler, CURLOPT_URL, url);
	curl_easy_setopt(c_handler, CURLOPT_WRITEFUNCTION, sf_curl_callback);
	curl_easy_setopt(c_handler, CURLOPT_WRITEDATA, (void *) &data);
	curl_easy_setopt(c_handler, CURLOPT_HEADER, headers);

	if (curl_easy_perform(c_handler) != CURLE_OK) {
		goto end;
	}

	*buffer = data.payload;
	*size = data.size;

	retval = 0;
end:
	if (c_handler) {
	    curl_multi_cleanup(c_handler);
	}
	if (headers) {
		curl_slist_free_all(headers);
	}
	curl_global_cleanup();
	if (retval) {
		*size = 0;
		buffer = NULL;
	}
    return retval;
}
