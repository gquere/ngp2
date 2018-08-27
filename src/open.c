#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entries.h"
#include "open.h"
#include "search.h"


extern struct search *current_search;


/* API ************************************************************************/
void open_entry(const struct entries *entries, const uint32_t index)
{
    char *file = entries_find_file(entries, index);
    if (file == NULL) {
        return;
    }
    uint32_t line = entries_get_line(entries, index);

    /* vim <file> -c /<pattern> -c <line_nr> */
    char *vim_cmdline = "vim %s -c /%s -c %d";
    char command[256] = {0};

    snprintf(command, sizeof(command), vim_cmdline,
             file, search_get_pattern(current_search), line);

    system(command);
}
