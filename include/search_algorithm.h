#ifndef NGP_SEARCH_ALGORITHM_H
#define NGP_SEARCH_ALGORITHM_H

#include <regex.h>


/* NORMAL SEARCH ALGORITHMS ***************************************************/
char * search_algorithm_normal_search(const char *line, const char *pattern,
                                      int size);
char * search_algorithm_insensitive_search(const char *line,
                                           const char *pattern, int size);


/* RABIN-KARP STRING SEARCH ***************************************************/
void search_algorithm_pre_rabin_karp(const char *pattern);
char * search_algorithm_rabin_karp(const char *text,
                                   const char *pattern, int text_size);

/* BOYER-MOORE-HORSPOOL *******************************************************/
void search_agorithm_pre_bmh(const char *pattern);
char * search_algorithm_bmh(const char *text,
                            const char *pattern, int tsize);

/* REGEX SEARCH ***************************************************************/
regex_t * search_algorithm_compile_regex(const char *pattern);
char * search_algorithm_regex_search(const char *line,
                                     const char *pattern, int size);

#endif /* NGP_SEARCH_ALGORITHM_H */
