#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "entries.h"


/* GET ************************************************************************/
uint8_t entries_is_file(const struct entries *this, const uint32_t index)
{
    return !this->entries[index].line;
}

char * entries_find_file(const struct entries *this, const uint32_t index)
{
    uint32_t i = index;

    while (this->entries[--i].line != 0) {
    }

    return this->entries[i].data;
}

uint32_t entries_get_line(const struct entries *this, const uint32_t index)
{
    return this->entries[index].line;
}

char * entries_get_data(const struct entries *this, const uint32_t index)
{
    return this->entries[index].data;
}

uint32_t entries_get_nb_lines(const struct entries *this)
{
    return this->nb_lines;
}

uint32_t entries_get_nb_entries(const struct entries *this)
{
    return this->nb_entries;
}

struct entry * entries_get_entry(const struct entries *this, const uint32_t index)
{
    return &this->entries[index];
}


/* ADD ************************************************************************/
static void check_alloc(struct entries *this)
{
    if (this->nb_entries < this->size) {
        return;
    }

    void *tmp = realloc(this->entries, (this->size + 500) * sizeof(struct entry));
    if (tmp != NULL) {
        this->entries = tmp;
    }

    this->size += 500;
}

void entries_add(struct entries *this, const uint32_t line, const char *data)
{
    /* check size of entries */
    check_alloc(this);

    /* copy input string */
    size_t data_size = strlen(data);
    char *data_copy = malloc(data_size + 1);
    memcpy(data_copy, data, data_size);
    data_copy[data_size] = 0;

    this->entries[this->nb_entries].line = line;
    this->entries[this->nb_entries].data = data_copy;
    this->nb_entries++;
    if (line != 0) {
        this->nb_lines++;
    }
}


/* CONSTRUCTOR ****************************************************************/
struct entries * entries_new(void)
{
    struct entries *this = calloc(1, sizeof(struct entries));

    this->entries = malloc(sizeof(struct entry));
    this->size = 1;

    return this;
}

void entries_delete(struct entries *this)
{
    uint32_t i = 0;
    for (i = 0; i < this->nb_entries; i++) {
        free(this->entries[i].data);
    }

    free(this->entries);
    free(this);
}
