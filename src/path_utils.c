#include <errno.h>
#include <string.h>

#include <path_utils.h>

int UtilsParsePath(const char *path, filename_t entryList[], size_t entryListLen)
{
    int ret = 0;
    path_t pathCopy;
    char *p;

    if (entryList == NULL) {
        // we're just counting depth, use some arbitraty big number
        entryListLen = 999;
    }

    if (entryListLen == 0 && entryList != NULL) {
        return -EINVAL;
    }

    if (strnlen(path, MAX_PATH_LEN) == MAX_PATH_LEN) {
        return -ENAMETOOLONG;
    }

    strcpy(pathCopy, path);
    p = pathCopy;
    if (entryList != NULL) {
        entryList[0][0] = '\0';
    }

    // remove initial / and set first element to indicate current dir
    if (*p == '/') {
        if (entryList != NULL) {
            strcpy(entryList[0], ".");
        }
        ret = 1;
    }

    p = strtok(p, "/");
    while (ret < entryListLen) {
        if (p != NULL) {
            if (strnlen(p, MAX_NAME_LEN) == MAX_NAME_LEN) {
                ret = -ENAMETOOLONG;
                break;
            }
            if (entryList != NULL) {
                strcpy(entryList[ret], p);
            }
            ret++;
        } else break;
        p = strtok(NULL, "/");
    }

    return ret;
}
