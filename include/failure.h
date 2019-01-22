#ifndef NGP_FAILURE_H
#define NGP_FAILURE_H

enum fail_modes {
    OPEN = 0,
    STAT,
    MMAP,
};

void failure_add(const char *filename, const uint32_t error);
void failure_display(void);

#endif /* NGP_FAILURE_H */
