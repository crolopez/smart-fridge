#ifndef SF_LOG
#define SF_LOG

#include "messages.h"

#define MAX_LOGSIZE 65536 // 64 KB
#define LOG_FORMAT "%d/%02d/%02d %02d:%02d:%02d [%s][%s] -> "

const char *node_name;
const char *log_file;

void sf_error(const char *msg, ...);
void sf_exit_error(const char *msg, ...);
void sf_info(const char *msg, ...);
void sf_warn(const char *msg, ...);
void sf_debug1(const char *msg, ...);
void sf_debug2(const char *msg, ...);
void sf_set_debug();
void sf_set_foreground();
void sf_set_log_file(char *file);
void sf_set_node_name(char *name);

#endif
