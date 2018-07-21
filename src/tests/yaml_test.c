#include <yaml.h>
#include <unistd.h>
#include <stdio.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

const char *node_name = "yaml_test";
const char *log_file = NULL;

void sft_help() {
    fprintf(stdout, "\n%s: <config file>\n", node_name);
}

int main(int argc, char **argv) {
    FILE *fp = NULL;
    yaml_parser_t parser;
    yaml_token_t  token;
    int retval = 1;
    int inc = 4;
    int tabs = inc;

    if (argc < 2) {
        sft_help();
        goto clean;
    }

    if (!(fp = fopen(argv[1], "r")) || !yaml_parser_initialize(&parser)) {
        printf("Error oppening the file.");
        goto clean;
    }

    yaml_parser_set_input_file(&parser, fp);

    while (yaml_parser_scan(&parser, &token), token.type != YAML_STREAM_END_TOKEN) {
        switch(token.type)
        {
        /* Stream start/end */
        case YAML_STREAM_START_TOKEN: printf(ANSI_COLOR_RED "********* STREAM START *********\n" ANSI_COLOR_RESET); break;
        /* Token types (read before actual token) */
        case YAML_KEY_TOKEN: printf(ANSI_COLOR_GREEN "\n%*c(Key) " ANSI_COLOR_RESET, tabs, ' '); break;
        case YAML_VALUE_TOKEN: printf(ANSI_COLOR_BLUE "(Value) " ANSI_COLOR_RESET); break;
        /* Block delimeters */
        case YAML_BLOCK_SEQUENCE_START_TOKEN: printf("Start Block (Sequence)\n"); break;
        case YAML_BLOCK_ENTRY_TOKEN: printf("\n%*cStart Block (Entry)", tabs, ' '); break;
        case YAML_BLOCK_END_TOKEN: tabs -= inc; printf("\n%*cEnd block", tabs, ' '); break;
        /* Data */
        case YAML_BLOCK_MAPPING_START_TOKEN: tabs += inc; printf("\n%*c[Block mapping]", tabs, ' ');            break;
        case YAML_SCALAR_TOKEN:  printf("%s ", token.data.scalar.value); break;
        /* Others */
        default:
          printf("Got token of type %d\n", token.type);
        }

        yaml_token_delete(&token);
    }
    printf("\n");


    retval = 0;
    yaml_parser_delete(&parser);
clean:
    if (fp) {
        fclose(fp);
    }
    return retval;
}
