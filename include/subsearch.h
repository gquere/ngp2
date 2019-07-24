#ifndef NGP_SUBSEARCH_H
#define NGP_SUBSEARCH_H

#include "search.h"


enum search_type {
    search_type_string,
    search_type_nocase,
    search_type_regex
};

struct subsearch_user_params {
    char pattern[4096];
    uint8_t invert_search;
    uint8_t search_type;
};


/* CONSTRUCTOR ****************************************************************/
struct search * subsearch_new(struct search *parent, const struct subsearch_user_params *user_params);
void subsearch_delete(struct search *this);

#endif /* NGP_SUBSEARCH_H */
