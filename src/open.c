#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entries.h"
#include "open.h"
#include "search.h"


extern struct search *current_search;

enum {
    slashes_only = 0,
    all_chars = 1,
};


/* UTILS **********************************************************************/
/**
 * Return a singlequote escaped expression valid as a shell argument.
 * Single quotes (') are escaped into ('\'') which basically glues an escaped
 * single quotes to the left and right of a closed and then reopened string.
 * Using single quotes is better because there's less escaping to do for the
 * shell.
 */
static char * shell_sanitize_pattern(const char *pattern)
{
    size_t escape_count = 0;
    size_t orig_count = 0;
    while (orig_count < strlen(pattern)) {
        if (pattern[orig_count] == '\'') {
            escape_count++;
        }
        orig_count++;
    }

    char *sanitized_pattern = calloc(strlen(pattern) + escape_count * 3 + 1, sizeof(char));

    orig_count = 0;
    size_t sanitized_count = 0;

    while (orig_count < strlen(pattern) + escape_count * 3) {
        if (pattern[orig_count] == '\'') {
            sanitized_pattern[sanitized_count++] = '\'';
            sanitized_pattern[sanitized_count++] = '\\';
            sanitized_pattern[sanitized_count++] = '\'';
        }

        sanitized_pattern[sanitized_count] = pattern[orig_count];
        sanitized_count++;
        orig_count++;
    }

    sanitized_pattern[sanitized_count] = 0;

    return sanitized_pattern;
}

/**
 * Check if a character has a litteral sense in vim's regex engine
 * and return 1 if so.
 */
static uint8_t is_char_regex_char(const char character, const uint8_t chars_to_escape)
{
    if (chars_to_escape == slashes_only) {
        if (character == '/') {
            return 1;
        } else {
            return 0;
        }
    }

    char regex_chars[] = {'[', ']', '^', '$', '.', '/',};
    uint8_t i;

    for (i = 0; i < sizeof(regex_chars); i++) {
        if (character == regex_chars[i]) {
            return 1;
        }
    }

    return 0;
}

/**
 * Escape all characters that have a meaning in vim regexes
 */
static char * regex_sanitize_pattern(const char *pattern, uint8_t chars_to_escape)
{
    size_t escape_count = 0;
    size_t orig_count = 0;

    while (orig_count < strlen(pattern)) {
        if (is_char_regex_char(pattern[orig_count], chars_to_escape)) {
            escape_count++;
        }
        orig_count++;
    }

    char *sanitized_pattern = calloc(strlen(pattern) + escape_count + 1, sizeof(char));

    orig_count = 0;
    size_t sanitized_count = 0;

    while (orig_count < strlen(pattern)) {
        if (is_char_regex_char(pattern[orig_count], chars_to_escape)) {
            sanitized_pattern[sanitized_count++] = '\\';
        }

        sanitized_pattern[sanitized_count] = pattern[orig_count];
        sanitized_count++;
        orig_count++;
    }

    sanitized_pattern[sanitized_count] = 0;

    return sanitized_pattern;
}


/* API ************************************************************************/
void open_entry(const struct entries *entries, const uint32_t index)
{
    char *file = entries_find_file(entries, index);
    if (file == NULL) {
        return;
    }

    /* pattern needs to be escaped first for vim's regex,
       then escaped for the shell */
    char *pattern = search_get_pattern(current_search);
    char *sanitized_pattern = NULL;

    if (search_get_regex(current_search)) {
        /* regexsearch, only escape slashes since vim's '/' is its internal search */
        char *regex_sanitized_pattern = regex_sanitize_pattern(pattern, slashes_only);
        sanitized_pattern = shell_sanitize_pattern(regex_sanitized_pattern);
        free(regex_sanitized_pattern);
    } else {
        /* stringsearch, need to escape all regex characters */
        char *regex_sanitized_pattern = regex_sanitize_pattern(pattern, all_chars);
        sanitized_pattern = shell_sanitize_pattern(regex_sanitized_pattern);
        free(regex_sanitized_pattern);
    }

    /* vim <file> -c /<pattern> -c <line_nr> */
    char *vim_cmdline = NULL;
    if (search_get_sensitive(current_search)) {
        vim_cmdline = "vim '%s' -c '/\\c%s' -c %d";
    } else {
        vim_cmdline = "vim '%s' -c '/%s' -c %d";
    }

    uint32_t line = entries_get_line(entries, index);

    size_t command_len = strlen(vim_cmdline) + strlen(file) + strlen(sanitized_pattern) + 10;
    char *command = calloc(1, command_len);
    snprintf(command, command_len, vim_cmdline, file, sanitized_pattern, line);
    system(command);
    free(command);
    entries_set_visited(entries, index);

    free(sanitized_pattern);
}
