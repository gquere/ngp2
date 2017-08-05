#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdint.h>

struct config {

    /* search attributes */
    char *pattern;
    char *directory;

    /* search file types */

    /* search parser type */
    uint8_t insensitive_search:1;
    uint8_t regex_search:1;
};


/* CONTRUCTOR *****************************************************************/
struct config * config_new(int argc, char *argv[]);
void config_delete(struct config *this);

#endif /* _CONFIG_H */
