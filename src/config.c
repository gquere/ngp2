#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "config.h"


/* UTILS **********************************************************************/
char * strcpycat(char *destination, const char *source)
{
    size_t source_len = strlen(source);
    size_t destination_len = 0;
    char *new_destination = NULL;

    if (destination != NULL) {
        destination_len = strlen(destination);
        size_t new_destination_len = source_len + 1 + destination_len + 1;
        new_destination = malloc(new_destination_len);
        snprintf(new_destination, new_destination_len, "%s %s", destination, source);
        free(destination);
    } else {
        size_t new_destination_len = source_len + 1;
        new_destination = malloc(new_destination_len);
        snprintf(new_destination, new_destination_len, "%s", source);
    }

    return new_destination;
}


/* PARSING ********************************************************************/
static uint8_t parse_config(struct config *this)
{
    char *extensions = ".c .h .cpp .py .S .pl .sh .php";    //TODO: get this from config file

    if (!this->only_user_extensions) {
        this->file_extensions = strcpycat(this->file_extensions, extensions);
    }

    return EXIT_SUCCESS;
}

static uint8_t parse_arguments(struct config *this, int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "ierfo:t:")) != -1) {
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
            this->only_user_extensions = 1;
            this->file_extensions = strcpycat(this->file_extensions, optarg);
            break;

        case 't':
            this->file_extensions = strcpycat(this->file_extensions, optarg);
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
