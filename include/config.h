#ifndef NGP_CONFIG_H
#define NGP_CONFIG_H

#include <stdint.h>
#include "tree.h"

struct config {

    /* search attributes */
    char *pattern;
    char *directory;

    /* search file types */
    struct tree *file_extensions_tree;
    uint8_t only_user_extensions:1;

    /* exclusions */
    struct tree *dir_exclusion_tree;

    /* search parser type */
    uint8_t insensitive_search:1;
    uint8_t regex_search:1;
    uint8_t raw_search:1;
    uint8_t follow_symlinks:1;
};


/* CONTRUCTOR *****************************************************************/
struct config * config_new(int argc, char *argv[]);
void config_delete(struct config *this);

#endif /* NGP_CONFIG_H */
