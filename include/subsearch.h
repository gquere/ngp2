#ifndef NGP_SUBSEARCH_H
#define NGP_SUBSEARCH_H

#include "search.h"

/* API ************************************************************************/
void subsearch_search(struct search *this);

/* CONSTRUCTOR ****************************************************************/
struct search * subsearch_new(struct search *parent, char *pattern);
void subsearch_delete(struct search *this);

#endif /* NGP_SUBSEARCH_H */
