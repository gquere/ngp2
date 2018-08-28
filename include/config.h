#ifndef NGP_CONFIG_H
#define NGP_CONFIG_H

#include <stdint.h>

struct config {

    /* search attributes */
    char *pattern;
    char *directory;

    /* search file types */
    char *file_extensions;
    uint8_t only_user_extensions:1;

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
