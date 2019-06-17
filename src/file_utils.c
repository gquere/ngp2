#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "file_utils.h"
#include "tree.h"


/* FILE UTILS *****************************************************************/
uint8_t file_utils_exists(const char *path)
{
    struct stat buf;

    if (stat(path, &buf) == -1) {
        return 0;
    }

    return 1;
}

uint8_t file_utils_is_file(const char *path)
{
    struct stat buf;

    if (stat(path, &buf) == -1) {
        return 0;
    }

    return !S_ISDIR(buf.st_mode);
}

uint8_t file_utils_is_dir(const char *path)
{
    struct stat buf;

    if (stat(path, &buf) == -1) {
        return 0;
    }

    return S_ISDIR(buf.st_mode);
}

uint8_t file_utils_is_symlink(const char *path)
{
    struct stat filestat;

    if (lstat(path, &filestat) == -1) {
        return 0;
    }

    return S_ISLNK(filestat.st_mode);
}

/**
 * Checks if a file is in the list of extensions
 * Returns 1 if it is, 0 if it isn't.
 */
uint8_t file_utils_check_extension(const char *file_name, const struct tree *file_extensions_tree)
{
    char *file_extension;

    if ((file_extension = strrchr(file_name, '.')) == NULL) {
        return 0;
    }

    if (is_string_in_tree(file_extensions_tree, file_extension + 1)) {
        return 1;
    }

    return 0;
}


uint8_t file_utils_is_dir_special(const char *directory_name)
{
    return !(strcmp(directory_name, ".") &&
             strcmp(directory_name, "..") &&
             strcmp(directory_name, ".git") &&
             strcmp(directory_name, ".svn"));
}
