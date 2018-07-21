#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "log.h"

static char debug_level = 0;
static char foreground = 0;
typedef enum ind_tag ind_tag;
static void sf_log(ind_tag ind, const char *msg, va_list ap);

static const char *msg_tag[] = {
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG1",
    "DEBUG2"
};

enum ind_tag {
    ERROR,
    WARNING,
    INFO,
    DEBUG1,
    DEBUG2
};

void sf_set_log_file(char *file) {
    log_file = file;
}

void sf_set_node_name(char *name) {
    node_name = name;
}

void sf_set_debug() {
    debug_level++;
}

void sf_set_foreground() {
    foreground = 1;
}

void sf_log(ind_tag ind, const char *msg, va_list ap) {
    FILE *fp = NULL;
    time_t now;
    struct tm date;
    char log_msg[MAX_LOGSIZE];
    char *end_header;
    size_t size;

    now = time(NULL);
    localtime_r(&now, &date);

    size = snprintf(log_msg, MAX_LOGSIZE, LOG_FORMAT,
            date.tm_year + 1900, date.tm_mon + 1, date.tm_mday,
            date.tm_hour, date.tm_min, date.tm_sec,
            node_name, msg_tag[ind]);

    end_header = &log_msg[size];
    vsnprintf(end_header, MAX_LOGSIZE - size, msg, ap);

    if (foreground) {
        fprintf(stdout, "%s\n", log_msg);
    }

    if (fp = fopen(log_file, "a"), fp) {
        fprintf(fp, "%s\n", log_msg);
        fflush(fp);
        fclose(fp);
    }
}

void sf_error(const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    sf_log(ERROR, msg, ap);
    va_end(ap);
}

void sf_exit_error(const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    sf_log(ERROR, msg, ap);
    va_end(ap);
    exit(1);
}

void sf_info(const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    sf_log(INFO, msg, ap);
    va_end(ap);
}

void sf_warn(const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    sf_log(WARNING, msg, ap);
    va_end(ap);
}

void sf_debug1(const char *msg, ...) {
    if (debug_level) {
        va_list ap;
        va_start(ap, msg);
        sf_log(DEBUG1, msg, ap);
        va_end(ap);
    }
}

void sf_debug2(const char *msg, ...) {
    if (debug_level > 1) {
        va_list ap;
        va_start(ap, msg);
        sf_log(DEBUG2, msg, ap);
        va_end(ap);
    }
}
