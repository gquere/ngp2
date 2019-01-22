#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "failure.h"


struct failure {
    char *filename;
    uint32_t error;
    struct failure *next;
};

/**
 * Linked list of files that failed to open
 * Displayed at program termination to let the user know some files may not
 * have been scanned. Usual reason is that they're bigger than mmap would
 * allow without the pontentially system-breaking MAP_NORESERVE flag
 */
struct failure_control {
    struct failure *first;
    struct failure *last;
};

struct failure_control failure_control = {0};

void failure_add(const char *filename, const uint32_t error)
{
    /* allocate new node for linked list */
    struct failure *new_failure = calloc(1, sizeof(struct failure));
    if (!new_failure) {
        return;
    }

    new_failure->filename = strdup(filename);
    new_failure->error = error;

    /* make a linked list */
    if (!failure_control.first) {
        failure_control.first = new_failure;
    } else {
        failure_control.last->next = new_failure;
    }

    failure_control.last = new_failure;
}

void failure_display(void)
{
    const char *fail_strings[] = {"OPEN", "STAT", "MMAP"};

    if (!failure_control.first) {
        return;
    }

    struct failure *current_failure = failure_control.first;
    while (current_failure) {
        printf("Warning: ngp failed to open file %s during %s"
               "; it might be too big\n",
               current_failure->filename,
               fail_strings[current_failure->error]);

        /* free the linked list and go to next */
        free(current_failure->filename);
        struct failure *tmp_failure = current_failure;
        current_failure = current_failure->next;
        free(tmp_failure);
    }
}
