#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "config.h"


/* PARSING ********************************************************************/
static uint8_t parse_config(struct config *this)
{
    char *extensions = ".c .h .cpp .py .S .pl .sh .php";    //TODO: get this from config file

    if (this->file_extensions == NULL) {
        this->file_extensions = strdup(extensions);
    }

    return EXIT_SUCCESS;
}

static uint8_t parse_arguments(struct config *this, int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "ierfo:")) != -1) {
        switch (opt) {
        case 'i':
            this->insensitive_search = 1;
            break;

        case 'e':
            this->regex_search = 1;
            break;

        case 'r':
            this->raw_search = 1;
            break;

        case 'f':
            this->follow_symlinks = 1;
            break;

        case 'o':
            this->file_extensions = strdup(optarg);
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

    if (parse_arguments(this, argc, argv) == EXIT_FAILURE) {
        printf("Failed parsing arguments\n");
        free(this);
        return NULL;
    }

    if (parse_config(this) == EXIT_FAILURE) {
        printf("Failed parsing config\n");
        free(this);
        return NULL;
    }

    return this;
}

void config_delete(struct config *this)
{
    free(this->pattern);
    free(this->directory);
    free(this->file_extensions);
    free(this);
}
