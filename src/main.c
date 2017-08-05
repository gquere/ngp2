#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "config.h"
#include "search.h"
#include "entries.h"
#include "display.h"


static void usage(const char *name)
{
    printf("%s\n", name);
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    struct config *config = config_new(argc, argv);
    if (config == NULL) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    struct entries *entries = entries_new();
    struct search *search = search_new(config->directory, config->pattern, entries, config);

    pthread_t search_thread;
    pthread_create(&search_thread, NULL, search_thread_start, (void *) search);

#if 1
    display_loop(search, entries);
#endif

    pthread_join(search_thread, NULL);
    entries_delete(entries);
    search_delete(search);
    config_delete(config);

    return EXIT_SUCCESS;
}
