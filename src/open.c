#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entries.h"
#include "open.h"


/* API ************************************************************************/
void open_entry(const struct entries *entries, const uint32_t index)
{
    char *file = entries_find_file(entries, index);
    if (file == NULL) {
        return;
    }

    char *vim_cmdline = "vim -c '%1$d' '%2$s'";

    char command[256] = {0};

    uint32_t line = entries_get_line(entries, index);

    snprintf(command, sizeof(command), vim_cmdline, line, file);

    system(command);
}
