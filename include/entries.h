#ifndef NGP_ENTRIES_H
#define NGP_ENTRIES_H

#include <stdint.h>

struct entry {
    uint32_t line;          /* line = 0 is a file */
    char *data;             /* data to hold : file name or line contents */
};

struct entries {
    struct entry *entries;
    uint32_t nb_entries;    /* number of entries filled */
    uint32_t nb_lines;      /* number of lines in the entries (rest are files) */
    uint32_t size;          /* actual number of entries to hold */
};


/* GET ************************************************************************/
uint8_t entries_is_file(const struct entries *this, const uint32_t index);
char * entries_find_file(const struct entries *this, const uint32_t index);
uint32_t entries_get_line(const struct entries *this, const uint32_t index);
char * entries_get_data(const struct entries *this, const uint32_t index);
uint32_t entries_get_nb_lines(const struct entries *this);
uint32_t entries_get_nb_entries(const struct entries *this);
struct entry * entries_get_entry(const struct entries *this, const uint32_t index);

/* ADD ************************************************************************/
void entries_add(struct entries *this, const uint32_t line, const char *data);

/* CONSTRUCTOR ****************************************************************/
struct entries * entries_new(void);
void entries_delete(struct entries *this);

#endif /* NGP_ENTRIES_H */
