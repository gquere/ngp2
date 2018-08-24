#ifndef NGP_CONFIG_H
#define NGP_CONFIG_H

#include <stdint.h>

struct config {

    /* search attributes */
    char *pattern;
    char *directory;

    /* search file types */
    char *file_types;

    /* search parser type */
    uint8_t insensitive_search:1;
    uint8_t regex_search:1;
};


/* CONTRUCTOR *****************************************************************/
struct config * config_new(int argc, char *argv[]);
void config_delete(struct config *this);

#endif /* NGP_CONFIG_H */
