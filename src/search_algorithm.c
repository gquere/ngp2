#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <regex.h>

#include "search.h"
#include "search_algorithm.h"


extern struct search *current_search;


/* NORMAL SEARCH ALGORITHMS ***************************************************/
char * search_algorithm_normal_search(const struct search *this,
                                      const char *line, const int size)
{
    (void) size;

    return strstr(line, this->pattern);
}

char * search_algorithm_insensitive_search(const struct search *this,
                                           const char *line, const int size)
{
    (void) size;

    return strcasestr(line, this->pattern);
}


/* BOYER-MOORE-HORSPOOL *******************************************************/
#define ASCII_ALPHABET  256
unsigned long int skipt[ASCII_ALPHABET];
int psize = 0;
void search_algorithm_pre_bmh(const char *pattern)
{

    psize = strlen(pattern);
//  if (psize == 1) {
//      mainsearch->parser = strstr_wrapper;
//  }

    int i;
    for (i = 0; i < ASCII_ALPHABET; i++)
        skipt[i] = psize;

    for (i = 0; i < psize - 1; i++) {
        /*
        if ((int) pattern[i] < 0) {
            pre_rabin_karp(pattern);
            mainsearch->parser = rabin_karp;
            return;
        }
        */

        skipt[(int) pattern[i]] = psize - i - 1;
    }
}

/**
 * Tuned Boyer-Moore-Horspool algorithm:
 * - Checks last, first character of pattern
 * - skips if unicode text
 */
char * search_algorithm_bmh(const struct search *this,
                            const char *text, const int tsize)
{
    int i = 0;

    while (i <= tsize - psize) {

        if (text[i + psize - 1] == this->pattern[psize - 1] && text[i] == this->pattern[0]) {
            if (!memcmp(text + i + 1, this->pattern + 1, psize - 2)) {
                return (char *) text + i;
            }
        }

        if ((uint8_t) text[i + psize - 1] < 0x21 || (uint8_t) text[i + psize - 1] > 0x7f) {
            i += psize;
        } else {
            i += skipt[(int) text[i + psize - 1]];
        }
    }

    return NULL;
}


/* REGEX SEARCH ***************************************************************/
regex_t * search_algorithm_compile_regex(const char *pattern)
{
    regex_t *reg = calloc(1, sizeof(regex_t));
    if (regcomp(reg, pattern, 0)) {
        free(reg);
        return NULL;
    }

    return reg;
}

char * search_algorithm_regex_search(const struct search *this,
                                     const char *line, const int size)
{
    (void) size;

    int ret = regexec(this->regex, line, 0, NULL, 0);

    if (ret != REG_NOMATCH) {
        return "1";
    } else {
        return NULL;
    }
}
