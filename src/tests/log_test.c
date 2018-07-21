#include "log.h"
#include <unistd.h>
#include <stdio.h>

const char *node_name = "log_test";
const char *log_file = "./test.log";

void sft_help() {
    fprintf(stdout, "\n%s: -[ddf]\n" \
                    "\t-d\t\tDebug mode\n" \
                    "\t-f\t\tForeground mode\n",
                    node_name);
}

int main(int argc, char **argv) {
    int option;

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

    sf_error("Test error %i %s", 0, ".");
    sf_debug1("Test debug %i %s", 1, ".");
    sf_debug2("Test debug %i %s", 2, ".");
    sf_warn("Test warning %i %s", 0, ".");
    sf_info("Test info %i %s", 0, ".");

    return 0;
}
