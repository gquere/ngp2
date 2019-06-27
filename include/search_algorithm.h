#ifndef NGP_SEARCH_ALGORITHM_H
#define NGP_SEARCH_ALGORITHM_H

#include <regex.h>
#include "search.h"

/* NORMAL SEARCH ALGORITHMS ***************************************************/
char * search_algorithm_normal_search(const struct search *this,
                                      const char *line, const int size);
char * search_algorithm_insensitive_search(const struct search *this,
                                           const char *line, const int size);

/* RABIN-KARP STRING SEARCH ***************************************************/
void search_algorithm_pre_rabin_karp(const char *pattern);
char * search_algorithm_rabin_karp(const struct search *this,
                                   const char *line, const int size);

/* BOYER-MOORE-HORSPOOL *******************************************************/
void search_algorithm_pre_bmh(const char *pattern);
char * search_algorithm_bmh(const struct search *this,
                            const char *line, const int size);

/* REGEX SEARCH ***************************************************************/
regex_t * search_algorithm_compile_regex(const char *pattern);
char * search_algorithm_regex_search(const struct search *this,
                                     const char *line, const int size);

#endif /* NGP_SEARCH_ALGORITHM_H */
