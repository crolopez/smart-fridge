#include "log.h"
#include "bar.h"
#include <unistd.h>
#include <stdio.h>

const char *node_name = "bar_test";
const char *log_file = "./test.log";

void sft_help() {
    fprintf(stdout, "\n%s: -[ddf]\n" \
                    "\t-d\t\tDebug mode\n" \
                    "\t-f\t\tForeground mode\n",
                    node_name);
}

int main(int argc, char **argv) {
    int option;
    char *code;

    while (option = getopt(argc, argv, "ddf"), option != -1) {
        switch (option) {
            case 'd':
                sf_set_debug();
            break;
            case 'f':
                sf_set_foreground();
            break;
            default:
                sft_help();
                return 1;
        }
    }

    if (code = sf_read_code(), code) {
        sf_info("Code readed: '%s'.", code);
    } else {
        sf_error("The code could not be read.");
    }


    return 0;
}
