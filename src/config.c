#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "config.h"
#include "tree.h"


/* UTILS **********************************************************************/
static char * remove_dot(const char *string)
{
    int i = 0;

    while (string[i] == '.') {
        i++;
    }

    return (char *)&string[i];
}


/* PARSING ********************************************************************/
static uint8_t parse_config(struct config *this)
{
    /* Directories ignored */
    tree_add_string(this->dir_exclusion_tree, ".");
    tree_add_string(this->dir_exclusion_tree, "..");
    tree_add_string(this->dir_exclusion_tree, ".git");
    tree_add_string(this->dir_exclusion_tree, ".svn");

    /* don't add custom extensions if user requested -o */
    if (this->only_user_extensions) {
        return EXIT_SUCCESS;
    }

    // TODO: get this from config file
    /* Source file extensions to check */
    tree_add_string(this->file_extensions_tree, "c");
    tree_add_string(this->file_extensions_tree, "h");
    tree_add_string(this->file_extensions_tree, "cpp");
    tree_add_string(this->file_extensions_tree, "py");
    tree_add_string(this->file_extensions_tree, "pl");
    tree_add_string(this->file_extensions_tree, "pm");
    tree_add_string(this->file_extensions_tree, "ksh");
    tree_add_string(this->file_extensions_tree, "sh");
    tree_add_string(this->file_extensions_tree, "php");
    tree_add_string(this->file_extensions_tree, "java");
    tree_add_string(this->file_extensions_tree, "jsp");
    tree_add_string(this->file_extensions_tree, "kt");
    tree_add_string(this->file_extensions_tree, "R");

    return EXIT_SUCCESS;
}

static uint8_t parse_arguments(struct config *this, int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "ierfo:t:x:")) != -1) {
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
            tree_add_string(this->file_extensions_tree, remove_dot(optarg));
            break;

        case 't':
            tree_add_string(this->file_extensions_tree, remove_dot(optarg));
            break;

        case 'x':
            tree_add_string(this->dir_exclusion_tree, optarg);
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

    if (strlen(this->pattern) == 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


/* CONTRUCTOR *****************************************************************/
struct config * config_new(int argc, char *argv[])
{
    struct config *this = calloc(1, sizeof(struct config));
    this->file_extensions_tree = tree_new();
    this->dir_exclusion_tree = tree_new();

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
    tree_delete(this->dir_exclusion_tree);
    tree_delete(this->file_extensions_tree);
    free(this->pattern);
    free(this->directory);
    free(this);
}
