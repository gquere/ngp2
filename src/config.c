#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "config.h"


/* PARSING ********************************************************************/
static uint8_t parse_config(struct config *this, int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "ie")) != -1) {
        switch (opt) {
        case 'i':
            this->insensitive_search = 1;
            break;

        case 'e':
            this->regex_search = 1;
            break;

        default:
            return EXIT_FAILURE;
        }
    }

    if (argc - optind == 1) {
        this->pattern = strdup(argv[optind++]);
        this->directory = strdup(".");
    } else if (argc - optind == 2) {
        this->pattern = strdup(argv[optind++]);
        this->directory = strdup(argv[optind++]);
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


/* CONTRUCTOR *****************************************************************/
struct config * config_new(int argc, char *argv[])
{
    struct config *this = calloc(1, sizeof(struct config));

    parse_config(this, argc, argv);

    return this;
}

void config_delete(struct config *this)
{
    free(this->pattern);
    free(this->directory);
    free(this);
}
