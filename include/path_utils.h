/* Small header for mini-lib used for path parsing */

#pragma once

#include <stddef.h>

#ifndef MAX_NAME_LEN
#define MAX_NAME_LEN     32
#endif

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN     256
#endif

typedef char path_t[MAX_PATH_LEN];

char *UtilsParsePathGetNext(const char *path, path_t buf, char **state);
int UtilsCheckPath(const char *path);
