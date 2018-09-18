#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entries.h"
#include "open.h"
#include "search.h"


extern struct search *current_search;


/* UTILS **********************************************************************/
char * sanitize_pattern(const char *pattern)
{
    size_t escape_count = 0;
    size_t orig_count = 0;
    while (orig_count < strlen(pattern)) {
        if (pattern[orig_count] == '"' || pattern[orig_count] == '\\') {
            escape_count++;
        }
        orig_count++;
    }

    char *sanitized_pattern = calloc(strlen(pattern) + escape_count, sizeof(char));

    orig_count = 0;
    size_t sanitized_count = 0;

    while (orig_count < strlen(pattern) + escape_count) {
        if (pattern[orig_count] == '"') {
            sanitized_pattern[sanitized_count++] = '\\';
        }

        if (pattern[orig_count] == '\\') {
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
    uint32_t line = entries_get_line(entries, index);
    char *sanitized_pattern = sanitize_pattern(search_get_pattern(current_search));

    /* vim <file> -c /<pattern> -c <line_nr> */
    char *vim_cmdline = NULL;
    if (search_get_sensitive(current_search)) {
        vim_cmdline = "vim \"%s\" -c \"/\\c%s\" -c %d";
    } else {
        vim_cmdline = "vim \"%s\" -c \"/%s\" -c %d";
    }
    char command[256] = {0};

    snprintf(command, sizeof(command), vim_cmdline,
             file, sanitized_pattern, line);

    system(command);

    free(sanitized_pattern);
}
