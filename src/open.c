#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entries.h"
#include "open.h"
#include "search.h"


extern struct search *current_search;


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

    char *sanitized_pattern = calloc(strlen(pattern) + escape_count * 3, sizeof(char));

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

    return sanitized_pattern;
}

/**
 * Check if a character has a litteral sense in vim's regex engine
 * and return 1 if so.
 */
static uint8_t is_char_regex_char(char character)
{
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
static char * regex_sanitize_pattern(const char *pattern)
{
    size_t escape_count = 0;
    size_t orig_count = 0;

    while (orig_count < strlen(pattern)) {
        if (is_char_regex_char(pattern[orig_count])) {
            escape_count++;
        }
        orig_count++;
    }

    char *sanitized_pattern = calloc(strlen(pattern) + escape_count, sizeof(char));

    orig_count = 0;
    size_t sanitized_count = 0;

    while (orig_count < strlen(pattern) + escape_count) {
        if (is_char_regex_char(pattern[orig_count])) {
            sanitized_pattern[sanitized_count++] = '\\';
        }

        sanitized_pattern[sanitized_count] = pattern[orig_count];
        sanitized_count++;
        orig_count++;
    }

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

    /* if the pattern is a regex, no need to regex escape it */
    if (search_get_regex(current_search)) {
        sanitized_pattern = shell_sanitize_pattern(pattern);
    } else {
        char *regex_sanitized_pattern = regex_sanitize_pattern(pattern);
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

    char command[256] = {0};
    uint32_t line = entries_get_line(entries, index);

    snprintf(command, sizeof(command), vim_cmdline,
             file, sanitized_pattern, line);
    system(command);
    entries_set_visited(entries, index);

    free(sanitized_pattern);
}
