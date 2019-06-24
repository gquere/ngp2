#ifndef NGP_SUBSEARCH_H
#define NGP_SUBSEARCH_H

#include "search.h"


struct subsearch_user_params {
    char pattern[4096];
    uint8_t invert_search;
    uint8_t regex_search;
};


/* CONSTRUCTOR ****************************************************************/
struct search * subsearch_new(struct search *parent, const struct subsearch_user_params *user_params);
void subsearch_delete(struct search *this);

#endif /* NGP_SUBSEARCH_H */
