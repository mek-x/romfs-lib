#include <errno.h>
#include <string.h>

#include <path_utils.h>

char *UtilsParsePathGetNext(const char *path, path_t buf, char **state) {
    char *p;

    if ((NULL == path) && (NULL == *state) || NULL == buf) {
        return NULL;
    }

    /* initial condition */
    if (NULL != path) {
        if (strnlen(path, MAX_PATH_LEN) == MAX_PATH_LEN) {
            return NULL;
        }
        strncpy(buf, path, MAX_PATH_LEN);

        p = buf;

        // remove initial slashes from path
        if (*p == '/') {
            while (*p == '/') { p++; }
            *state = p;
            return ".";
        }

        *state = buf;
    } else {
        p = NULL;
    }

    p = __strtok_r(p, "/", state);
    return p;
}

int UtilsCheckPath(const char *path)
{
    int ret = 0;
    path_t buf;
    char *state;
    char *p;

    if (path == NULL) {
        return -EINVAL;
    }

    if (strnlen(path, MAX_PATH_LEN) == MAX_PATH_LEN) {
        return -ENAMETOOLONG;
    }

    p = UtilsParsePathGetNext(path, buf, &state);
    while (p != NULL) {
        if (strnlen(p, MAX_NAME_LEN) == MAX_NAME_LEN) {
            ret = -ENAMETOOLONG;
            break;
        }

        ret++;
        p = UtilsParsePathGetNext(NULL, buf, &state);
    }

    return ret;
}
