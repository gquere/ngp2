#ifndef NGP_FILE_UTILS_H
#define NGP_FILE_UTILS_H

#include <stdint.h>
#include "tree.h"

/* UTILS **********************************************************************/
uint8_t file_utils_exists(const char *path);
uint8_t file_utils_is_file(const char *path);
uint8_t file_utils_is_dir(const char *path);
uint8_t file_utils_is_symlink(const char *path);

uint8_t file_utils_check_extension(const char *file_name, const struct tree *file_extensions_tree);
uint8_t file_utils_is_dir_special(const char *directory_name);

#endif /* NGP_FILE_UTILS_H */
