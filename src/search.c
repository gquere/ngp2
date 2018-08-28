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
#include "search_algorithm.h"


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
    char *file_extensions;
    regex_t *regex;

    /* storage */
    struct entries *entries;
};

extern struct search *current_search;


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
        if (p == orig_p + p_len) {
            return;
        }

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
    if (!file_utils_check_extension(file, this->file_extensions) &&
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

        if (dir_entry->d_type == DT_REG) {              // regular file
            lookup_file(this, dir_entry_path);
        } else if (dir_entry->d_type == DT_DIR) {       // folder
            if (!file_utils_is_dir_special(dir_entry->d_name)) {
                lookup_directory(this, dir_entry_path);
            }
        } else if (dir_entry->d_type&DT_LNK) {          // symlink
            /* default : ignore symlinks */
            if (this->follow_symlinks) {
                lookup_file(this, dir_entry_path);
            }
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


/* GETTERS ********************************************************************/
char * search_get_pattern(const struct search *this)
{
    return this->pattern;
}

uint8_t search_get_status(const struct search *this)
{
    return this->status;
}

regex_t * search_get_regex(const struct search *this)
{
    return this->regex;
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
    this->file_extensions = config->file_extensions;
    this->follow_symlinks = config->follow_symlinks;

    if (config->insensitive_search) {
        this->parser = search_algorithm_insensitive_search;
    } else if (config->regex_search) {
        this->regex = search_algorithm_compile_regex(this->pattern);
        if (this->regex) {
            this->parser = search_algorithm_regex_search;
        } else {
            printf("Failed validating regex\n");
            free(this->pattern);
            free(this->directory);
            free(this);
            return NULL;
        }
    } else {
        this->parser = search_algorithm_normal_search;
#ifdef _BMH
        search_algorithm_pre_bmh(this->pattern);
        this->parser = search_algorithm_bmh;
#endif /* _BMH */
#ifdef _RK
        search_algorithm_pre_rabin_karp(this->pattern);
        this->parser = search_algorithm_rabin_karp;
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
