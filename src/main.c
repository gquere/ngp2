#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "config.h"
#include "search.h"
#include "entries.h"
#include "display.h"


/* USAGE **********************************************************************/
static void usage(void)
{
    printf("usage: ngp [options]... pattern [directory/file]\n\n");
    printf("options:\n");
    printf(" -i : case insensitive search\n");
    printf(" -e : regex search\n");
    printf(" -r : raw search, ignores extensions restrictions\n");
}


/* MAIN ***********************************************************************/
int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage();
        return EXIT_FAILURE;
    }

    struct config *config = config_new(argc, argv);
    if (config == NULL) {
        usage();
        return EXIT_FAILURE;
    }

    struct entries *entries = entries_new();
    struct search *search = search_new(config->directory, config->pattern, entries, config);

    pthread_t search_thread;
    pthread_create(&search_thread, NULL, search_thread_start, (void *) search);

#ifndef _PERFORMANCE_TEST
    struct display *display = display_new();
    display_loop(display, search, entries);
    display_delete(display);
    search_stop(search);
#endif

    pthread_join(search_thread, NULL);

#ifdef _PERFORMANCE_TEST
    uint32_t nb_lines = entries_get_nb_lines(entries);
    uint32_t nb_files = entries_get_nb_entries(entries) - nb_lines;
    printf("Found %d files, %d lines\n", nb_files, nb_lines);
#endif

    entries_delete(entries);
    search_delete(search);
    config_delete(config);

    return EXIT_SUCCESS;
}
