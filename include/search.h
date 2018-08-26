#ifndef NGP_SEARCH_H
#define NGP_SEARCH_H

#include <regex.h>

#include "entries.h"
#include "config.h"

struct search;


/* API ************************************************************************/
void search_stop(struct search *this);

/* GETTERS ********************************************************************/
char * search_get_pattern(const struct search *this);
uint8_t search_get_status(const struct search *this);
regex_t * search_get_regex(const struct search *this);

/* SEARCH THREAD ENTRY POINT **************************************************/
void * search_thread_start(void *context);

/* CONSTRUCTOR ****************************************************************/
struct search * search_new(const char *directory, const char *pattern,
                           struct entries *entries, struct config *config);
void search_delete(struct search *this);

#endif /* NGP_SEARCH_H */
