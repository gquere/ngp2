#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "file_utils.h"


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

uint8_t file_utils_is_dir_special(const char *directory_name)
{
    return !(strcmp(directory_name, ".") &&
             strcmp(directory_name, "..") &&
             strcmp(directory_name, ".git") &&
             strcmp(directory_name, ".svn"));
}
