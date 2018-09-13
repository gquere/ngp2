#include <stdlib.h>
#include <string.h>

#include "subsearch.h"
#include "entries.h"
#include "search.h"


/* API ************************************************************************/
void subsearch_search(struct search *this)
{
    struct entries *parent_entries = search_get_entries(this->parent);

    uint32_t parent_nb_entries = entries_get_nb_entries(parent_entries);

    uint32_t i = 0;
    uint8_t first = 1;
    uint32_t file_index = 0;

    for (i = 0; i < parent_nb_entries; i++) {
        uint32_t parent_entries_line = entries_get_line(parent_entries, i);

        /* if it's a file, store its index in case there's a line match later */
        if (parent_entries_line == 0) {
            first = 1;
            file_index = i;
            continue;
        }

        char *parent_entries_data = entries_get_data(parent_entries, i);
        if ( !!strstr(parent_entries_data, this->pattern) ^ this->invert_search) {
            /* check if file has been added yet */
            if (first) {
                entries_add(this->entries, 0, entries_get_data(parent_entries, file_index));
                first = 0;
            }

            /* add line */
            entries_add(this->entries, parent_entries_line, parent_entries_data);
        }
    }
}


/* CONSTRUCTOR ****************************************************************/
struct search * subsearch_new(struct search *parent, char *pattern,
                              const uint8_t invert_search)
{
    struct search *this = calloc(1, sizeof(struct search));
    this->regex_search = 1;
    this->pattern = strdup(pattern);
    this->parent = parent;
    this->invert_search = invert_search;

    this->entries = entries_new();

    return this;
}

void subsearch_delete(struct search *this)
{
    entries_delete(this->entries);

    free(this->pattern);
    free(this);
}
