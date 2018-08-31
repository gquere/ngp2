#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <regex.h>

#include "search.h"
#include "search_algorithm.h"


extern struct search *current_search;


/* NORMAL SEARCH ALGORITHMS ***************************************************/
char * search_algorithm_normal_search(const char *line, const char *pattern,
                                      int size)
{
    (void) size;

    return strstr(line, pattern);
}

char * search_algorithm_insensitive_search(const char *line, const char *pattern, int size)
{
    (void) size;

    return strcasestr(line, pattern);
}


/* RABIN-KARP STRING SEARCH ***************************************************/
/**
 * Rolling hash function used for this instance of Rabin-Karp
 */
#define REHASH(a,b,h) ((((h) - (a) * d) << 1) + b)

int d, hp; // Rabin-Karp parameters
int psize; // pattern size used by RK


/**
 * Compute Rabin-Karp parameters (shift d and hash(pattern))
 *
 * @param pattern   search pattern
 */
void search_algorithm_pre_rabin_karp(const char *pattern)
{
    int i;

    psize = strlen(pattern);

    /* compute shift */
    d = 1 << (psize - 1);

    /* compute hash(pattern) */
    for (hp = i = 0; i < psize; i++)
        hp = (hp << 1) + pattern[i];
}

/**
 * Rabin-Karp algorithm: use a rolling hash over the text to fasten
 * the comparison computation
 *
 * @param text      Haystack
 * @param pattern   Needle
 * @return          pointer to match or NULL
 */
char * search_algorithm_rabin_karp(const char *text,
                                   const char *pattern, int text_size)
{
    int ht;
    int i;

    if (text_size < psize) {
        return NULL;
    }

    /* compute hash(text) at position 0 */
    for (ht = i = 0; i < psize; i++)
        ht = (ht << 1) + (uint8_t) text[i];

    for (i = 0; i <= text_size - psize; i++) {
        if (ht == hp) /* got a hash match, but it could be a collision */
            if (!memcmp(pattern, text + i, psize))
                return (char *) text + i;
        /* compute rolling hash for next position */
        ht = REHASH((uint8_t) text[i], (uint8_t) text[i + psize], ht);
    }

    return NULL;
}


/* BOYER-MOORE-HORSPOOL *******************************************************/
#define ASCII_ALPHABET  256
unsigned long int skipt[ASCII_ALPHABET];
int psize = 0;

/**
 * Initialize the BMH's skip-table. This may fail because BMH doesn't work on a
 * single character.
 *
 * @return  EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
uint8_t search_algorithm_pre_bmh(const char *pattern,
                                 const uint8_t ignore_binary_files)
{

    psize = strlen(pattern);
    if (psize == 1) {
        return EXIT_FAILURE;
    }

    int i;
    for (i = 0; i < ASCII_ALPHABET; i++) {
        skipt[i] = psize;
    }

    if (ignore_binary_files) {
        for (i = 0; i < 0x20; i++) {
            skipt[i] = 0;
        }

        for (i = 0x7f; i < ASCII_ALPHABET; i++) {
            skipt[i] = 0;
        }
    }

    for (i = 0; i < psize - 1; i++) {
        skipt[(uint8_t) pattern[i]] = psize - i - 1;
    }

    return EXIT_SUCCESS;
}

/**
 * Tuned Boyer-Moore-Horspool algorithm:
 *   Checks last, first character of pattern
 */
char * search_algorithm_bmh(const char *text,
                            const char *pattern, int tsize)
{
    int i = 0;

    while (i <= tsize - psize) {

        if (text[i + psize - 1] == pattern[psize - 1] && text[i] == pattern[0]) {
            if (!memcmp(text + i + 1, pattern + 1, psize - 2)) {
                return (char *) text + i;
            }
        }

        i += skipt[(uint8_t) text[i + psize - 1]];
    }

    return NULL;
}

extern int is_binary;

/**
 * Tuned Boyer-Moore-Horspool algorithm:
 *   Checks last, first character of pattern
 */
char * search_algorithm_bmh_ignore_binary(const char *text,
                            const char *pattern, int tsize)
{
    int i = 0;

    while (i <= tsize - psize) {

        if (text[i + psize - 1] == pattern[psize - 1] && text[i] == pattern[0]) {
            if (!memcmp(text + i + 1, pattern + 1, psize - 2)) {
                return (char *) text + i;
            }
        }

        if (skipt[(uint8_t) text[i + psize - 1]] == 0) {
            is_binary = 1;
            return NULL;
        }

        i += skipt[(uint8_t) text[i + psize - 1]];
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

char * search_algorithm_regex_search(const char *line,
                                     const char *pattern, int size)
{
    (void) size;
    (void) pattern;

    int ret = regexec(search_get_regex(current_search), line, 0, NULL, 0);

    if (ret != REG_NOMATCH) {
        return "1";
    } else {
        return NULL;
    }
}
