#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "search_algorithm.h"
#include "subsearch.h"
#include "entries.h"
#include "search.h"


/* UTILS **********************************************************************/
uint8_t matches(const struct search *this, char *data)
{
    int res = (this->parser(this, data, 0) != NULL);

    return res ^ this->invert_search;
}


/* SUBSEARCH THREAD ***********************************************************/
static void subsearch_search(struct search *this)
{
    struct entries *parent_entries = search_get_entries(this->parent);

    uint32_t parent_nb_entries = entries_get_nb_entries(parent_entries);

    /* check if there's new data */
    if (parent_nb_entries == this->parent_previous_nb_entries) {
        return;
    }

    uint32_t i = 0;

    for (i = this->parent_previous_nb_entries; i < parent_nb_entries; i++) {
        uint32_t parent_entries_line = entries_get_line(parent_entries, i);

        /* if it's a file, store its index in case there's a line match later */
        if (parent_entries_line == 0) {
            this->first_line_of_file = 1;
            this->previous_file_index = i;
            continue;
        }

        char *parent_entry_data = entries_get_data(parent_entries, i);
        if (matches(this, parent_entry_data)) {
            /* check if file has been added yet */
            if (this->first_line_of_file) {
                entries_copy(this->entries, entries_get_entry(parent_entries, this->previous_file_index));
                this->first_line_of_file = 0;
            }

            /* add line */
            entries_copy(this->entries, entries_get_entry(parent_entries, i));
        }
    }
    this->parent_previous_nb_entries = parent_nb_entries;
}

void * subsearch_search_thread_start(void *context)
{
    struct search *this = context;

    while (!this->stop) {
        subsearch_search(this);
        usleep(50000);
    }

    return NULL;
}


/* CONSTRUCTOR ****************************************************************/
struct search * subsearch_new(struct search *parent, const struct subsearch_user_params *user_params)
{
    struct search *this = calloc(1, sizeof(struct search));
    this->parent = parent;
    this->pattern = strdup(user_params->pattern);
    this->invert_search = user_params->invert_search;
    this->parser = search_algorithm_normal_search;

    if (user_params->search_type == search_type_nocase) {
        this->case_insensitive = 1;
        this->parser = search_algorithm_insensitive_search;
    }

    if (user_params->search_type == search_type_regex) {
        this->regex_search = 1;
        this->regex = search_algorithm_compile_regex(this->pattern);
        if (this->regex == NULL) {
            return NULL;
        }
        this->parser = search_algorithm_regex_search;
    }

    this->entries = entries_new();

    pthread_create(&this->subsearch_search_thread, NULL, subsearch_search_thread_start, (void *) this);

    return this;
}

void subsearch_delete(struct search *this)
{
    this->stop = 1;
    pthread_join(this->subsearch_search_thread, NULL);
    entries_delete_copy(this->entries);

    free(this->regex);
    free(this->pattern);
    free(this);
}
