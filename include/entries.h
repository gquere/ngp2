#ifndef NGP_ENTRIES_H
#define NGP_ENTRIES_H

#include <stdint.h>

#include <pthread.h>


struct entry {
    uint32_t line;          /* line = 0 is a file */
    char *data;             /* data to hold : file name or line contents */
    uint8_t visited;        /* if the entry was opened by the user during the session */
};

struct entries {
    struct entry *entries;
    uint32_t nb_entries;    /* number of entries filled */
    uint32_t nb_lines;      /* number of lines in the entries (rest are files) */
    uint32_t size;          /* actual number of entries to hold */
};


pthread_mutex_t entries_mutex;  /* mutex on entries because they are realloced */


/* GET ************************************************************************/
uint8_t entries_is_file(const struct entries *this, const uint32_t index);
char * entries_find_file(const struct entries *this, const uint32_t index);
uint32_t entries_get_line(const struct entries *this, const uint32_t index);
char * entries_get_data(const struct entries *this, const uint32_t index);
uint32_t entries_get_nb_lines(const struct entries *this);
uint32_t entries_get_nb_entries(const struct entries *this);
struct entry * entries_get_entry(const struct entries *this, const uint32_t index);
void entries_set_visited(const struct entries *this, const uint32_t index);
uint8_t entries_get_visited(const struct entries *this, const uint32_t index);
void entries_toggle_visited(const struct entries *this, const uint32_t index);

/* ADD ************************************************************************/
void entries_add(struct entries *this, const uint32_t line, const char *data);

/* CONSTRUCTOR ****************************************************************/
struct entries * entries_new(void);
void entries_delete(struct entries *this);

#endif /* NGP_ENTRIES_H */
