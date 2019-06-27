#ifndef NGP_SEARCH_H
#define NGP_SEARCH_H

#include <regex.h>

#include "entries.h"
#include "config.h"
#include "tree.h"


struct search {
    uint8_t status:1;
    uint8_t stop:1;
    uint8_t case_insensitive:1;
    uint8_t raw_search:1;
    uint8_t regex_search:1;
    uint8_t follow_symlinks:1;
    uint8_t invert_search:1;    // used by subsearch to exclude patterns

    /* search parameters */
    char *directory;
    char *pattern;
    char * (*parser)(const struct search *, const char *, int);
    struct tree *file_extensions_tree;
    struct tree *dir_exclusion_tree;
    regex_t *regex;

    /* storage */
    struct entries *entries;

    /* subsearch */
    struct search *subsearch;
    struct search *parent;
    pthread_t subsearch_search_thread;
    uint32_t parent_previous_nb_entries;
    uint8_t first_line_of_file;
    uint32_t previous_file_index;
};


/* API ************************************************************************/
void search_stop(struct search *this);

/* GETTERS ********************************************************************/
char * search_get_pattern(const struct search *this);
uint8_t search_get_status(const struct search *this);
regex_t * search_get_regex(const struct search *this);
struct entries * search_get_entries(const struct search *this);
struct search * search_get_parent(const struct search *this);
uint8_t search_get_invert(const struct search *this);
uint8_t search_get_sensitive(const struct search *this);

/* SEARCH THREAD ENTRY POINT **************************************************/
void * search_thread_start(void *context);

/* CONSTRUCTOR ****************************************************************/
struct search * search_new(const char *directory, const char *pattern,
                           struct entries *entries, struct config *config);
void search_delete(struct search *this);

#endif /* NGP_SEARCH_H */
