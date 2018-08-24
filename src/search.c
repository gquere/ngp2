#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <sys/mman.h>

#include <regex.h>

#include "entries.h"
#include "config.h"
#include "file_utils.h"


struct search {
    uint8_t status:1;
    uint8_t stop:1;
    uint8_t case_insensitive:1;
    uint8_t raw_search:1;
    uint8_t regex_search:1;
    uint8_t follow_symlinks:1;

    /* search parameters */
    char *directory;
    char *pattern;
    char * (*parser)(const char *, const char *, int);
    char *file_types;
    regex_t *regex;

    /* storage */
    struct entries *entries;
};

extern struct search *current_search;


/* NORMAL SEARCH ALGORITHMS ***************************************************/
static char * normal_search(const char *line, const char *pattern, int size)
{
    (void) size;

    return strstr(line, pattern);
}

static char * insensitive_search(const char *line, const char *pattern, int size)
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
static void pre_rabin_karp(const char *pattern)
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
static inline char * rabin_karp(const char *text, const char *pattern, int text_size)
{
    int ht;
    int i;

    /* compute hash(text) at position 0 */
    for (ht = i = 0; i < psize; i++)
        ht = (ht << 1) + text[i];

    for (i = 0; i <= text_size - psize; i++) {
        if (ht == hp) /* got a hash match, but it could be a collision */
            if (!memcmp(pattern, text + i, psize))
                return (char *) text + i;
        /* compute rolling hash for next position */
        ht = REHASH(text[i], text[i + psize], ht);
    }

    return NULL;
}


/* BOYER-MOORE-HORSPOOL *******************************************************/
#define ASCII_ALPHABET  256
unsigned long int skipt[ASCII_ALPHABET];
int psize = 0;
static void pre_bmh(const char *pattern)
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
static inline char * bmh(const char *text, const char *pattern, int tsize)
{
    int i;

    i = 0;
    while (i <= tsize - psize) {
        if (text[i + psize - 1] == pattern[psize - 1] && text[i] == pattern[0])
            if (!memcmp(text + i + 1, pattern + 1, psize - 2))
                return (char *) text + i;
        if (text[i + psize - 1] > 0)
            i += skipt[(int) text[i + psize - 1]];
        else
            while (text[i + psize - 1] < 0) //unicode
                i += psize;
    }

    return NULL;
}


/* REGEX SEARCH ***************************************************************/
static uint8_t is_regex_valid(struct search *this)
{
    regex_t *reg = calloc(1, sizeof(regex_t));
    if (regcomp(reg, this->pattern, 0)) {
        free(reg);
        return 0;
    } else {
        this->regex = reg;
    }

    return 1;
}

static char * regex_search(const char *line, const char *pattern, int size)
{
    (void) size;
    (void) pattern;

    int ret = regexec(current_search->regex, line, 0, NULL, 0);

    if (ret != REG_NOMATCH) {
        return "1";
    } else {
        return NULL;
    }
}


/* FILE PARSING ***************************************************************/
static void parse_file_contents(struct search *this, const char *file, char *p,
                                const size_t p_len)
{
    char *endline;
    uint8_t first = 1;
    uint32_t line_number = 1;
    char *orig_p = p;

    while ((endline = strchr(p, '\n'))) {

        *endline = '\0';

        if (this->parser(p, this->pattern, endline - p) != NULL) {

            if (first) {
                /* add file */
                entries_add(this->entries, 0, file);
                first = 0;
            }

            entries_add(this->entries, line_number, p);
        }

        p = endline + 1;
        line_number++;
    }

    /* special case of not newline terminated file */
    if (endline == NULL && p < orig_p + p_len) {
        if (this->parser(p, this->pattern, orig_p + p_len - p) != NULL) {

            if (first) {
                /* add file */
                entries_add(this->entries, 0, file);
                first = 0;
            }

            entries_add(this->entries, line_number, p);
        }
    }
}

static uint8_t lookup_file(struct search *this, const char *file)
{
    /* check file extension */
    if (!file_utils_check_extension(file, this->file_types) &&
        !this->raw_search) {
        return EXIT_FAILURE;
    }

    int f = open(file, O_RDONLY);
    if (f == -1) {
        //printf("Failed opening file %s\n", file);
        return EXIT_FAILURE;
    }

    struct stat sb;
    if (fstat(f, &sb) < 0) {
        //printf("Failed stat on file %s\n", file);
        close(f);
        return EXIT_FAILURE;
    }

    /* return if file is empty */
    if (sb.st_size == 0) {
        close(f);
        return EXIT_SUCCESS;
    }

    char *p = mmap(0, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, f, 0);
    if (p == MAP_FAILED) {
        //printf("Failed mapping file %s\n", file);
        close(f);
        return EXIT_FAILURE;
    }

    char *pp = p;
    parse_file_contents(this, file, p, sb.st_size);

    if (munmap(pp, sb.st_size) < 0) {
        //printf("Failed unmapping file %s\n", file);
        close(f);
        return EXIT_FAILURE;
    }

    close(f);
    return EXIT_SUCCESS;
}


/* DIRECTORY PARSING **********************************************************/
static uint32_t lookup_directory(struct search *this, const char *directory)
{
    //printf("Looking up directory %s\n", directory);

    DIR *dir_stream = opendir(directory);
    if (dir_stream == NULL) {
        //printf("Failed opening directory %s\n", directory);
        return EXIT_FAILURE;
    }

    while (!this->stop) {
        struct dirent *dir_entry = readdir(dir_stream);

        if (dir_entry == NULL) {
            break;
        }

        char dir_entry_path[PATH_MAX];
        snprintf(dir_entry_path, PATH_MAX, "%s/%s", directory, dir_entry->d_name);

        /* default : ignore symlinks */
        if (!this->follow_symlinks && file_utils_is_symlink(dir_entry_path)) {
            continue;
        }

        if (!(dir_entry->d_type&DT_DIR)) {
            lookup_file(this, dir_entry_path);
        } else if (!file_utils_is_dir_special(dir_entry->d_name)) {
            lookup_directory(this, dir_entry_path);
        }
    }

    closedir(dir_stream);

    return EXIT_SUCCESS;
}


/* API ************************************************************************/
void search_stop(struct search *this)
{
    this->stop = 1;
}


/* GET ************************************************************************/
char * search_get_pattern(const struct search *this)
{
    return this->pattern;
}

uint8_t search_get_status(const struct search *this)
{
    return this->status;
}


/* SEARCH THREAD ENTRY POINT **************************************************/
void * search_thread_start(void *context)
{
    struct search *this = context;

    /* signal we're running */
    this->status = 1;

    if (!this->follow_symlinks && file_utils_is_symlink(this->directory)) {
        return NULL;
    }

    if (file_utils_is_file(this->directory)) {
        lookup_file(this, this->directory);
    } else if (file_utils_is_dir(this->directory)) {
        lookup_directory(this, this->directory);
    }

    /* search is done */
    this->status = 0;

    return NULL;
}


/* CONSTRUCTOR ****************************************************************/
struct search * search_new(const char *directory, const char *pattern,
                           struct entries *entries, struct config *config)
{
    struct search *this = calloc(1, sizeof(struct search));
    this->directory = strdup(directory);
    this->pattern = strdup(pattern);
    this->entries = entries;
    this->case_insensitive = config->insensitive_search;
    this->raw_search = config->raw_search;
    this->regex_search = config->regex_search;
    this->file_types = config->file_types;
    this->follow_symlinks = config->follow_symlinks;

    if (config->insensitive_search) {
        this->parser = insensitive_search;
    } else if (config->regex_search) {
        if (is_regex_valid(this)) {
            this->parser = regex_search;
        } else {
            printf("Failed validating regex\n");
            free(this->regex);
            free(this->pattern);
            free(this->directory);
            free(this);
            return NULL;
        }
    } else {
        this->parser = normal_search;
#ifdef _BMH
        pre_bmh(this->pattern);
        this->parser = bmh;
#endif /* _BMH */
#ifdef _RK
        pre_rabin_karp(this->pattern);
        this->parser = rabin_karp;
#endif /* _RK */
    }

    return this;
}

void search_delete(struct search *this)
{
    free(this->regex);
    free(this->pattern);
    free(this->directory);
    free(this);
}
